#include "WebServer.h"

SSL_CTX *sslCtx = nullptr;

WebServer::WebServer(const uint16_t &port, const unsigned short &numWorkers, std::string webRoot)
    : port(port), webRoot(std::move(webRoot)), httpConnections(Config::maxConnectFd),
      threadPool(numWorkers), epollFd(epoll_create(2000)), listenFd(0), events{}, pipeFd{} {

    if (Config::isHttpsEnabled) {
        //initializare OpenSSL
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        sslCtx = SSL_CTX_new(SSLv23_server_method());
        if (!sslCtx) {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }

        //incarcare cheia privata si certificat
       if (SSL_CTX_use_certificate_file(sslCtx, "./certs/cert.pem", SSL_FILETYPE_PEM) <= 0 || SSL_CTX_use_PrivateKey_file(sslCtx, "./certs/key.pem", SSL_FILETYPE_PEM) <= 0) 
        {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
        }

    }

    listenFd = Utility::getListenFd(port);
    Utility::addFdToEpoll(epollFd, listenFd, false);
    pipe2(pipeFd, O_NONBLOCK);
    Utility::addFdToEpoll(epollFd, pipeFd[0], false);
    Utility::pipeFd = pipeFd;
    Utility::setSignal(SIGPIPE, SIG_IGN);
    Utility::setSignal(SIGINT, Utility::signalSigintHandler);
    Utility::setSignal(SIGTERM, Utility::signalSigintHandler);
    Utility::setSignal(SIGALRM, Utility::signalAlrmHandler);
    alarm(Config::timeoutSecond);
}

//WebServer::~WebServer() = default;

void WebServer::eventLoop() 
{
    std::cout << "Server start at 127.0.0.1 port: " << port << std::endl;
    while (!Utility::stop) 
    {
        int num = epoll_wait(epollFd, events, Config::maxEpollWaitEvent, -1);
        if (num < 0)
         if (errno == EINTR) 
         {
                // Întrerupere de la un semnal, continuăm bucla
                std::cerr <<  " [EPOLL] Interrupted by signal, resuming...\n";
                continue;
            } else {
                // Eroare serioasă
                std::cerr << " [EPOLL] Wait failure: " << strerror(errno) << "\n";
                break;
            }
        std::cout << num << " evenimente epoll detectate.\n";

        for (int i = 0; i < num; ++i) 
        {
            int socketFd = events[i].data.fd;
            if (socketFd == listenFd) 
            {
                std::cout << "[Epoll Event] Noua conexiune detectată.\n";
                handleComingConnection();
            } 
            else if (socketFd == pipeFd[0]) 
            {
                std::cout << "[Epoll Event] Semnal primit din pipe.\n";
                handleSignal();
            } 
            else if (events[i].events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP)) 
            {
                std::cout << "[Epoll Event] Erorare sau închidere pe FD: " << socketFd << "\n";
                handleCloseOrError(socketFd);
            } 
            else if (events[i].events & EPOLLIN) 
            {
                std::cout << "[Epoll Event] Eveniment READ pe FD: " << socketFd << "\n";
                handleInput(socketFd);
            } 
            else if (events[i].events & EPOLLOUT) 
            {
                std::cout << "[Epoll Event] Eveniment WRITE pe FD: " << socketFd << "\n";
                handleOutput(socketFd);
            }
        }
    }
    threadPool.quitLoop();
    std::cout << "Server Stopped.\n";
}


void WebServer::handleCloseOrError(const int &fd) 
{
    std::cout << "[FD " << fd << "] Inchidere sau eroare detectata. Eliminare conexiune.\n";
    Utility::timeList.removeTimer(fd);
    httpConnections[fd].closeConnection();
     std::cout << "[FD " << fd << "] Conexiune inchisa cu succes.\n";
}

void WebServer::handleComingConnection() 
{
    sockaddr_in clientAddr{0};
    socklen_t addrLen = sizeof(clientAddr);
    while (true) 
    {
        auto start = std::chrono::high_resolution_clock::now();  //start cronometrul
        std::unique_lock<std::mutex> locker(Utility::mutex);
        int clientFd = accept(listenFd, reinterpret_cast<sockaddr *>(&clientAddr), &addrLen);
        locker.unlock();
         auto end = std::chrono::high_resolution_clock::now();  //stop cronometrul
        if (clientFd == -1) {
            break;
        }
        //preluare ip folosim biblioteca inet.h
        std::cout<<"Noua conexiune de la: "<<inet_ntoa(clientAddr.sin_addr)<<":"<<ntohs(clientAddr.sin_port)<<" (FD: "<<clientFd<<")\n";
        std::chrono::duration<double> elapsed = end - start;
        std::cout<<"Timp acceptare conexiune: "<<elapsed.count()<<" secunde.\n";

        if (Config::isHttpsEnabled) 
        {
            SSL *ssl = SSL_new(sslCtx);
            SSL_set_fd(ssl, clientFd);
            if (SSL_accept(ssl) <= 0) 
            {
                ERR_print_errors_fp(stderr);
                SSL_free(ssl);
                close(clientFd);
                std::cerr<<"Eșec la stabilirea conexiunii HTTPS pentru FD: "<<clientFd<<"\n";
                continue;
            }
            httpConnections[clientFd].init(epollFd, clientFd, ssl);
        } 
        else 
        {
            SSL *ssl = nullptr;
            httpConnections[clientFd].init(epollFd, clientFd, ssl);
        }

        Utility::timeList.attachTimer(&httpConnections[clientFd]);
        Utility::addFdToEpoll(epollFd, clientFd);
        std::cout<< "FD "<<clientFd<<" adaugat la epoll pentru evenimente.\n";
    }
}

void WebServer::handleInput(const int &fd) 
{
    std::cout << "[FD " << fd << "] Input event detected.\n";

    //det task priority based on the request type
    int priority = 0;
    if (httpConnections[fd].isPostRequest()) {
        std::cout << "[FD " << fd << "] Detected POST request. Assigning high priority.\n";
        priority = 10;  //mareee prioritate pt POST requests
    } else if (httpConnections[fd].isDynamicRequest()) {
        std::cout << "[FD " << fd << "] Detected dynamic request (.php). Assigning priority.\n";
        priority = 8;   //prioritate pt dynamic .php requests
    } else {
        std::cout << "[FD " << fd << "] Regular GET request. Assigning default priority.\n";
        priority = 0;   //basic priority pt static GET requests
    }

    //update the connection's state and timer
    httpConnections[fd].setEvent(true);
    Utility::timeList.updateTimer(fd);

    //add the task to the thread pool with the calculated priority
    threadPool.addTask(&httpConnections[fd], priority);
    std::cout << "[FD " << fd << "] Task added to ThreadPool with priority: " << priority << "\n";
}

void WebServer::handleOutput(const int &fd) 
{
    std::cout << "[FD " << fd << "] Output event detected.\n";
    httpConnections[fd].setEvent(false);
    Utility::timeList.updateTimer(fd);
    threadPool.addTask(&httpConnections[fd]);
    std::cout << "[FD " << fd << "] Task adăugat în ThreadPool pentru procesare output.\n";
}

void WebServer::handleSignal() 
{
    int buf;
    ssize_t ret = read(pipeFd[0], &buf, sizeof(buf));
    if (ret == -1) 
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
        {
            return;
        }
    }
    switch (buf) 
    {
        case SIGALRM:
            std::cout << "[Signal] Timeout SIGALRM primit.\n";
            Utility::timeList.tick();
            break;
        case SIGINT:
            std::cout << "[Signal] SIGINT primit. Oprirea serverului...\n";
            Utility::stop = true;
            break;
        case SIGTERM:
            std::cout << "[Signal] SIGTERM primit. Oprirea serverului...\n";
            Utility::stop = true;
            break;
        default:
            std::cout << "[Signal] Semnal necunoscut primit: " << buf << "\n";
    }
}

WebServer::~WebServer() 
{
    if (sslCtx) 
    {
        SSL_CTX_free(sslCtx);
        std::cout << "Contextul SSL a fost eliberat.\n";
    }
    std::cout << "WebServer destructurat cu succes.\n";
}