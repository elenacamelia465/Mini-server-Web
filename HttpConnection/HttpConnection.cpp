#include <mutex>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "HttpConnection.h"
#include "../Utility/Utility.h"



HttpConnection::HttpConnection()
    : epollFd(0), fd(0), isReadEvent(true), sendBuf{}, recvBuf{}, recvIndex{}, sendIndex{}, webRoot("web/")
{
    //Bufferul de raspuns va fi completat din fisiere HTML in functie de cerere
}

HttpConnection::~HttpConnection() = default;

void HttpConnection::closeConnection() const
{
    close(fd);
}

//fucntion pentru incarcarea unui fisier HTML si crearea unui raspuns HTTP
void HttpConnection::loadHtmlFile(const std::string &filename)
{
    std::string fullPath = webRoot + filename;
    std::cout << "Încerc să deschid fișierul: " << fullPath << std::endl; // Linia de debug

    std::ifstream file(fullPath);
    if (file)
    {
        std::stringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/html; charset=UTF-8\r\n"
                 << "Connection: close\r\n"
                 << "\r\n";

        response << file.rdbuf();
        strncpy(sendBuf, response.str().c_str(), sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf) - 1] = '\0';
    }
    else
    {
        std::cerr << "Eroare: Nu s-a putut deschide fișierul " << fullPath << std::endl;
        std::string errorResponse = "HTTP/1.1 404 Not Found\r\n"
                                    "Content-Type: text/html; charset=UTF-8\r\n"
                                    "Connection: close\r\n"
                                    "\r\n"
                                    "<html><body><h1>404 - Page Not Found</h1></body></html>";
        strncpy(sendBuf, errorResponse.c_str(), sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf) - 1] = '\0';
    }
}


// //Functie pentru a identifica pagina HTML solicitata pe baza cererii
// std::string HttpConnection::getRequestedPage()
// {
//     std::string request(recvBuf);
//     std::string page = "index.html"; //pagina implicita

//     size_t pos = request.find("GET ");
//     if (pos != std::string::npos)
//     {
//         size_t endPos = request.find(" ", pos + 4);
//         if (endPos != std::string::npos)
//         {
//             std::string requestedFile = request.substr(pos + 4, endPos - pos - 4);
//             if (requestedFile == "/")
//             {
//                 page = "index.html"; //Se returneaza index.html pentru radacina
//             }
//             else
//             {
//                 page = requestedFile.substr(1); //elimina '/' de la inceput
//             }
//         }
//     }
//     return page;
// }


//MODIFICARE FUNCTIE PENTRU A SUPORTA GESTIONAREA TIPURILOR DE CERERI
std::string HttpConnection::getRequestedPage() {
    std::string request(recvBuf);
    std::string page = "index_test.html";  

    size_t methodEnd = request.find(' ');
    if (methodEnd == std::string::npos) {
        return page;  
    }

    size_t urlStart = methodEnd + 1;
    size_t urlEnd = request.find(' ', urlStart);
    if (urlEnd == std::string::npos) {
        return page;  
    }

    std::string url = request.substr(urlStart, urlEnd - urlStart);

    size_t queryStart = url.find('?');
    if (queryStart != std::string::npos) {
        std::string queryString = url.substr(queryStart + 1);
        url = url.substr(0, queryStart); 
        std::cout << "Query string: " << queryString << std::endl;
       
    }

    if (!url.empty() && url[0] == '/') {
        url = url.substr(1);
    }

    if (url.empty()) {
        return page;
    }

    if (url.find("..") != std::string::npos) {
        std::cerr << "Tentativă de acces neautorizat detectată!" << std::endl;
        return "index_test.html";
    }

    return url;
}


//MODIFICARE FUNCTIE PENTRU A SUPORTA GESTIONAREA TIPURILOR DE CERERI
void HttpConnection::handleRead()
{
     ssize_t ret;
    if (ssl) //verificare pt htts
    {
        ret = SSL_read(ssl, recvBuf + recvIndex, Config::sendRecvBufSize - recvIndex);
    } 
    else 
    {
        ret = recv(fd, recvBuf + recvIndex, Config::sendRecvBufSize - recvIndex, 0);
    }
    if (ret <= 0) {
        return; 
    }
    recvIndex += ret;

    if (strstr(recvBuf, "\r\n\r\n") != nullptr) {
        recvBuf[recvIndex] = '\0';
        std::string request(recvBuf);

        if (request.find("GET") == 0) {
            handleGet();
        } else if (request.find("POST") == 0) {
            handlePost();
        } else if (request.find("PUT") == 0) {
            handlePut();
        } else if (request.find("DELETE") == 0) {
            handleDelete();
        } else if (request.find("HEAD") == 0) {
            handleHead();
        } else {
            strncpy(sendBuf, ERROR, sizeof(sendBuf) - 1);
            sendBuf[sizeof(sendBuf) - 1] = '\0';
        }

        epoll_event event{};
        event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
        event.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
    }
}

void HttpConnection::handleRequest()
{
    if (isReadEvent)
    {
        handleRead();
    }
    else
    {
        handleWrite();
    }
}

void HttpConnection::handleWrite()
{
    size_t len = strlen(sendBuf);
    sendBuf[len] = '\0';
    std::cout << "Răspuns trimis la socketFd: " << fd << std::endl;
    std::cout << sendBuf;
    ssize_t ret = 0;
    while (sendIndex < len)
    {
        if (ssl) 
        {
            ret = SSL_write(ssl, sendBuf + sendIndex, len - sendIndex);
        } 
        else 
        {
            ret = send(fd, sendBuf + sendIndex, len - sendIndex, 0);
        }
        if (ret == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
        }
        else if (ret > 0)
        {
            sendIndex += ret;
        }
    }
    if (sendIndex >= len)
    {
        recvIndex = 0;
        sendIndex = 0;
        if (isKeepAlive)
        {
            epoll_event event{};
            event.events = EPOLLIN | EPOLLONESHOT | EPOLLRDHUP;
            event.data.fd = fd;
            epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
        }
        else
        {
            std::unique_lock<std::mutex> locker(Utility::mutex);
            close(fd);
            if (ssl) 
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
                ssl = nullptr;
            }
            locker.unlock();
        }
    }
    else
    {
        epoll_event event{};
        event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
        event.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
    }
}

void HttpConnection::init(const int &epollFd, const int &fd, SSL *ssl) //!!!
{
    this->epollFd = epollFd;
    this->fd = fd;
    this->ssl = ssl;
}

void HttpConnection::setEvent(const bool &isReadEvent)
{
    this->isReadEvent = isReadEvent;
}

int HttpConnection::getFd() const
{
    return fd;
}


//ADAUGARE PENTRU GESTIONAREA TIPURILOR DE CERERI
void HttpConnection::handleHead()
{
    std::string page = getRequestedPage();
    std::string fullPath = webRoot + page;

    struct stat fileStat;
    if(stat(fullPath.c_str(), &fileStat) == -1)
    {
        strncpy(sendBuf, ERROR, sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf)-1]='\0';
    }
    else{
         snprintf(sendBuf, sizeof(sendBuf),
                 HEAD_SUCCESS,
                 fileStat.st_size);
    }
    epoll_event event{};
    event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
    event.data.fd = fd;
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
}




void HttpConnection::handlePost()
{
    
     std::string request(recvBuf);
    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart == std::string::npos) {
        strncpy(sendBuf, ERROR, sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf) - 1] = '\0';
        return;
    }

    std::string body = request.substr(bodyStart + 4);

    std::string username, password;
    size_t usernamePos = body.find("username=");
    size_t passwordPos = body.find("password=");
    if (usernamePos != std::string::npos && passwordPos != std::string::npos) {
        username = body.substr(usernamePos + 9, body.find("&", usernamePos) - (usernamePos + 9));
        password = body.substr(passwordPos + 9);
    }

    std::string response;
    if (username == "admin" && password == "admin") {
        response = LOGIN_SUCCESS + username + "!</p></body></html>";
    } else {
        response =  LOGIN_FAILED;
    }

    strncpy(sendBuf, response.c_str(), sizeof(sendBuf) - 1);
    sendBuf[sizeof(sendBuf) - 1] = '\0';

}


void HttpConnection::handlePut() {
   

    std::string request(recvBuf);

    size_t pathStart = request.find(" ") + 1;
    size_t pathEnd = request.find(" ", pathStart);
    std::string relativePath = request.substr(pathStart, pathEnd - pathStart);

    if (!relativePath.empty() && relativePath[0] == '/') {
        relativePath = relativePath.substr(1);
    }

    std::string filePath = webRoot + relativePath;

    std::cout << "Uploading file to: " << filePath << std::endl;

    if (!std::filesystem::exists(webRoot)) {
        std::filesystem::create_directories(webRoot);
    }

    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart == std::string::npos) {
        std::cerr << "Error: Missing body in PUT request" << std::endl;
        strncpy(sendBuf, ERROR, sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf) - 1] = '\0';
        return;
    }
    std::string fileData = request.substr(bodyStart + 4);

    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Could not open file for writing: " << filePath << std::endl;
        strncpy(sendBuf, UPLOAD_FAILED, sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf) - 1] = '\0';
        return;
    }
    outFile.write(fileData.c_str(), fileData.size());
    outFile.close();

    std::string response = UPLOAD_SUCCESS;
    strncpy(sendBuf, response.c_str(), sizeof(sendBuf) - 1);
    sendBuf[sizeof(sendBuf) - 1] = '\0';
}

void HttpConnection::handleDelete() {

 std::cout << "Handling DELETE request" << std::endl;

    std::string page = getRequestedPage();
    std::string fullPath = webRoot + page;

    if (remove(fullPath.c_str()) == 0) {
        std::string response = DELETE_SUCCESS;
        strncpy(sendBuf, response.c_str(), sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf) - 1] = '\0';
    } else {
        strncpy(sendBuf, ERROR, sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf) - 1] = '\0';
    }
}

void HttpConnection::handleGet() {  //aici nu merge sa descarcam documentele prin https TO DO! 

   std::string request(recvBuf);

    size_t pathStart = request.find(" ") + 1;
    size_t pathEnd = request.find(" ", pathStart);
    std::string filePath = request.substr(pathStart, pathEnd - pathStart);

    if (!filePath.empty() && filePath[0] == '/') {
        filePath = filePath.substr(1);
    }
    if (filePath.empty()) {
        filePath = "index_test.html"; 
    }

    if (filePath.find("files/") == 0) {
        std::string fileName = filePath.substr(6);
        std::string fullPath = webRoot + "files/" + fileName;

        int fileFd = open(fullPath.c_str(), O_RDONLY);
        if (fileFd == -1) {
            strncpy(sendBuf, ERROR, sizeof(sendBuf) - 1);
            sendBuf[sizeof(sendBuf) - 1] = '\0';
            handleWrite();
            return;
        }

        struct stat fileStat;
        fstat(fileFd, &fileStat);

        std::string fileType = get_file_type((char *)fileName.c_str());

        std::string headers = DOWNLOAD_HEADER;

        send(fd, headers.c_str(), headers.size(), 0);

        char buffer[DATA_LENGTH];
        ssize_t bytesRead;
        while ((bytesRead = read(fileFd, buffer, sizeof(buffer))) > 0) {
            send(fd, buffer, bytesRead, 0);
        }

        close(fileFd); 
        return;
    }

    if (filePath.find(".php") != std::string::npos) {
        std::string fullPath = webRoot + filePath;

        
        std::string command = "php-cgi " + fullPath;
        //folosim pipe pentru ca acesta preia iesirea generata de comanda php-cgi. Pipe-ul permite server-ului sa captureze continutul dinamic produs de scriptul php si sa il includa in raspunsul http trimis catre client
        //cand serverul executa un script php folosind php-cgi, iesirea este trimisa la stdout. de aceea folosim un pipe pentru a citi aceasta iesire si pentru a o include in raspunsul http catre client
        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe) {
            strncpy(sendBuf, ERROR, sizeof(sendBuf) - 1);
            sendBuf[sizeof(sendBuf) - 1] = '\0';
            handleWrite();
            return;
        }

        std::stringstream output;
        char buffer[DATA_LENGTH];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output << buffer;
        }
        pclose(pipe);

        std::string phpOutput = output.str();

        size_t doctypePos = phpOutput.find("<!DOCTYPE html>");
        if (doctypePos != std::string::npos) {
            phpOutput = phpOutput.substr(doctypePos); 
        }

        std::string response = TEXT_HTML + phpOutput;

        strncpy(sendBuf, response.c_str(), sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf) - 1] = '\0';
        handleWrite();
        return;
    }

    std::string fullPath = webRoot + filePath;
    int fileFd = open(fullPath.c_str(), O_RDONLY);
    if (fileFd == -1) {
        strncpy(sendBuf, ERROR, sizeof(sendBuf) - 1);
        sendBuf[sizeof(sendBuf) - 1] = '\0';
        handleWrite();
    } else {
        const char *fileType = get_file_type((char *)filePath.c_str());
        snprintf(sendBuf, sizeof(sendBuf), "%s", fileType);
        ssize_t headerLength = strlen(fileType);
        ssize_t bytesRead = read(fileFd, sendBuf + headerLength, sizeof(sendBuf) - headerLength - 1);
        sendBuf[headerLength + bytesRead] = '\0';
        handleWrite();
        close(fileFd);
    }
}

void HttpConnection::handleWriteLargeResponse(int fileFd) {
    char buffer[DATA_LENGTH];  
    ssize_t bytesRead, bytesSent;

    while ((bytesRead = read(fileFd, buffer, sizeof(buffer))) > 0) {
        bytesSent = send(fd, buffer, bytesRead, 0);
        if (bytesSent == -1) {
            perror("Eroare la trimiterea datelor către client");
            break;
        }
    }
    close(fileFd);
}


const char* HttpConnection::get_file_type(char* filename)
{
    if ( strstr(filename, ".html") || strstr(filename, ".php")) return TEXT_HTML;
    else if ( strstr(filename, ".jar")) return APP_JS_ARCHIVE;
    else if ( strstr(filename, ".js")) return TEXT_JS;
    else if ( strstr(filename, ".ogg")) return APP_OGG;
    else if ( strstr(filename, ".pdf")) return APP_PDF;
    else if ( strstr(filename, ".json")) return APP_JSON;
    else if ( strstr(filename, ".xml")) return APP_XML;
    else if ( strstr(filename, ".zip")) return APP_ZIP;
    else if ( strstr(filename, ".mp3")) return AUDIO_MPEG;
    else if ( strstr(filename, ".wav")) return AUDIO_WAV;
    else if ( strstr(filename, ".gif")) return IMAGE_GIF;
    else if ( strstr(filename, ".jpeg") ||  strstr(filename, ".jpg")) return IMAGE_JPEG;
    else if ( strstr(filename, ".png")) return IMAGE_PNG;
    else if ( strstr(filename, ".tiff")) return IMAGE_TIFF;
    else if ( strstr(filename, ".ico")) return IMAGE_VND_MSICON;
    else if ( strstr(filename, ".json")) return APP_JSON;
    else if ( strstr(filename, ".xml")) return APP_XML;
    else if ( strstr(filename, ".mpeg")) return VIDEO_MPEG;
    else if ( strstr(filename, ".mp4")) return VIDEO_MP4;
    else if ( strstr(filename, ".webm")) return VIDEO_WEBM;
    return TEXT_PLAIN;
}