#include <mutex>
#include "HttpConnection.h"
#include "../Utility/Utility.h"

HttpConnection::HttpConnection()
    :epollFd(0), fd(0), isReadEvent(true), sendBuf{}, recvBuf{}, recvIndex{}, sendIndex{}, webRoot{}
    {
        //here is the initial message that is send to the client as a simple response
         std::string s = "HTTP/1.1 200 OK\r\n"
                    "Date: Sun, 4 Oct 2020 22:59:21 GMT\r\n"
                    "Content-Type: text/html; charset=UTF-8\r\n"
                    "Content-Length: 87\r\n"
                    "\r\n"
                    "<html>\n"
                    "      <head></head>\n"
                    "      <body>\n"
                    "            Hello World!\n"
                    "      </body>\n"
                    "</html>\n";
        strcpy(sendBuf, s.c_str());
    }

HttpConnection::~HttpConnection() = default;

void HttpConnection::closeConnection() const
{
    close(fd);
}

void HttpConnection::handleRead()
{
    ssize_t ret;
    while (true)
    {
        ret = recv(fd, recvBuf + recvIndex, Config::sendRecvBufSize - recvIndex, 0);
        if(ret == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
        }
        else if (ret >0)
        {
            recvIndex +=ret;
        }
    }
     if (recvIndex >= 5 && recvBuf[recvIndex - 1] == '\n' && recvBuf[recvIndex - 2] == '\r' &&
        recvBuf[recvIndex - 3] == '\n' && recvBuf[recvIndex - 4] == '\r') {
        recvBuf[recvIndex] = '\0';
        std::cout << "Receive from socketFd: " << fd << std::endl;
        std::cout << recvBuf;
        recvIndex = 0;
        sendIndex = 0;
        epoll_event event{};
        event.events = EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP;
        event.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
    } else {
        epoll_event event{};
        event.events = EPOLLIN | EPOLLONESHOT | EPOLLRDHUP;
        event.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
    }
}

void HttpConnection::handleRequest()
{
    if(isReadEvent)
    {
        handleRead();
    }
    else{
        handleWrite();
    }
}

void HttpConnection::handleWrite()
{
    size_t len = strlen (sendBuf);
    sendBuf[len] = '\0';
    std::cout<<"Sent to socketFd: "<< fd << std::endl;
    std::cout<<sendBuf;
    ssize_t ret= 0;
    while(sendIndex < len)
    {
        ret=send(fd, sendBuf + sendIndex, len - ret, 0);
        if (ret == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
        }
        else if(ret >0)
        {
            sendIndex +=ret;
        }
    }
    if(sendIndex >=len)
    {
        recvIndex = 0;
        sendIndex = 0;
        if(isKeepAlive)
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
        event.events = EPOLLOUT | EPOLLONESHOT|EPOLLRDHUP;
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

int HttpConnection::getFd() const{
    return fd;
}