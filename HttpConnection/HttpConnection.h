#ifndef WEBSERVER_HTTPCONNECTION_H
#define WEBSERVER_HTTPCONNECTION_H

#include <sys/socket.h>
#include <iostream>
#include <sys/epoll.h>
#include <cstring>
#include "../Config/Config.h"

class HttpConnection
{
    private:
        //epoll instance is used for managing events related to this connection
        int epollFd;
        int fd;
        bool isReadEvent;
        char recvBuf[Config::sendRecvBufSize];
        unsigned long recvIndex;
        char sendBuf[Config::sendRecvBufSize];
        unsigned long sendIndex;
        int webRoot;
        void handleRead();
        void handleWrite();
        bool isKeepAlive = false;

    public:
        HttpConnection();
        virtual ~HttpConnection();
        void closeConnection() const;
        void handleRequest();
        void init(const int &epollFd, const int &fd);
        void setEvent(const bool &isReadEvent);
        int getFd() const;
};

#endif