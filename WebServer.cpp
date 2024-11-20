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
    while (!Utility::stop) {
        int num = epoll_wait(epollFd, events, Config::maxEpollWaitEvent, -1);
        if (num < 0 && errno != EINTR) {
            std::cerr << "Epoll wait failure.\n";
            break;
        }
        for (int i = 0; i < num; ++i) {
            int socketFd = events[i].data.fd;
            if (socketFd == listenFd)
            {
                std::cout << "Connection coming." << std::endl;
                handleComingConnection();
            } else if (socketFd == pipeFd[0]) 
            {
                std::cout << "Signal from pipe." << std::endl;
                handleSignal();
            } else if (events[i].events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP))
            {
                std::cout << "Closing " << socketFd << std::endl;
                handleCloseOrError(socketFd);
            } else if (events[i].events & EPOLLIN) 
            {
                std::cout << "Epoll in event from fd: " << socketFd << std::endl;
                handleInput(socketFd);
            } else if (events[i].events & EPOLLOUT) 
            {
                std::cout << "Epoll out event from fd: " << socketFd << std::endl;
                handleOutput(socketFd);
            }
        }
    }
    threadPool.quitLoop();
    std::cout << "Server Stopped." << std::endl;
}

void WebServer::handleCloseOrError(const int &fd) 
{
    Utility::timeList.removeTimer(fd);
    httpConnections[fd].closeConnection();
}

void WebServer::handleComingConnection() {
    sockaddr_in clientAddr{0};
    socklen_t addrLen = sizeof(clientAddr);
    while (true) {
        std::unique_lock<std::mutex> locker(Utility::mutex);
        int clientFd = accept(listenFd, reinterpret_cast<sockaddr *>(&clientAddr), &addrLen);
        locker.unlock();
        if (clientFd == -1) {
            break;
        }
        //preluare ip folosim biblioteca inet.h
        std::cout << "Noua conexiune de la: "<< inet_ntoa(clientAddr.sin_addr)<< ":"<<ntohs(clientAddr.sin_port)<<std::endl;
        if (Config::isHttpsEnabled) 
        {
            SSL *ssl = SSL_new(sslCtx);
            SSL_set_fd(ssl, clientFd);
            if (SSL_accept(ssl) <= 0) 
            {
                ERR_print_errors_fp(stderr);
                SSL_free(ssl);
                close(clientFd);
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
    }
}

void WebServer::handleInput(const int &fd) 
{
    httpConnections[fd].setEvent(true);
    Utility::timeList.updateTimer(fd);
    threadPool.addTask(&httpConnections[fd]);
}

void WebServer::handleOutput(const int &fd) 
{
    httpConnections[fd].setEvent(false);
    Utility::timeList.updateTimer(fd);
    threadPool.addTask(&httpConnections[fd]);
}

void WebServer::handleSignal() 
{
    int buf;
    ssize_t ret = read(pipeFd[0], &buf, sizeof(buf));
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
    }
    if (buf == SIGALRM) {
        std::cout << "SIGALRM" << std::endl;
        Utility::timeList.tick();
    } else if (buf == SIGINT) {
        std::cout << "SIGINT" << std::endl;
        Utility::stop = true;
    }
}
WebServer::~WebServer() 
{
    if (sslCtx) 
    {
        SSL_CTX_free(sslCtx);
    }
}