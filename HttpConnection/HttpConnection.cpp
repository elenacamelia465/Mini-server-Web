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


//Functie pentru a identifica pagina HTML solicitata pe baza cererii
std::string HttpConnection::getRequestedPage()
{
    std::string request(recvBuf);
    std::string page = "index.html"; //pagina implicita

    size_t pos = request.find("GET ");
    if (pos != std::string::npos)
    {
        size_t endPos = request.find(" ", pos + 4);
        if (endPos != std::string::npos)
        {
            std::string requestedFile = request.substr(pos + 4, endPos - pos - 4);
            if (requestedFile == "/")
            {
                page = "index.html"; //Se returneaza index.html pentru radacina
            }
            else
            {
                page = requestedFile.substr(1); //elimina '/' de la inceput
            }
        }
    }
    return page;
}


void HttpConnection::handleRead()
{
    ssize_t ret;
    while (true)
    {
        ret = recv(fd, recvBuf + recvIndex, Config::sendRecvBufSize - recvIndex, 0);
        if (ret == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
        }
        else if (ret > 0)
        {
            recvIndex += ret;
        }
    }
    if (recvIndex >= 5 && recvBuf[recvIndex - 1] == '\n' && recvBuf[recvIndex - 2] == '\r' &&
        recvBuf[recvIndex - 3] == '\n' && recvBuf[recvIndex - 4] == '\r')
    {
        recvBuf[recvIndex] = '\0';
        std::cout << "Cerere primită de la socketFd: " << fd << std::endl;
        std::cout << recvBuf;
        recvIndex = 0;
        sendIndex = 0;

        //Det pq solicitata si incarca fisierul HTML corespunzator
        std::string page = getRequestedPage();
        loadHtmlFile(page);

        epoll_event event{};
        event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
        event.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
    }
    else
    {
        epoll_event event{};
        event.events = EPOLLIN | EPOLLONESHOT | EPOLLRDHUP;
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
        ret = send(fd, sendBuf + sendIndex, len - sendIndex, 0);
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

void HttpConnection::init(const int &epollFd, const int &fd)
{
    this->epollFd = epollFd;
    this->fd = fd;
}

void HttpConnection::setEvent(const bool &isReadEvent)
{
    this->isReadEvent = isReadEvent;
}

int HttpConnection::getFd() const
{
    return fd;
}
