#ifndef WEBSERVER_HTTPCONNECTION_H
#define WEBSERVER_HTTPCONNECTION_H

#include <sys/socket.h>
#include <iostream>
#include <sys/epoll.h>
#include <cstring>
#include "../Config/Config.h"
#include <filesystem>


//------ADAUGARE GESTIONARE TIPURI DE CERERI
#include "../file_types.h"
#include <sys/stat.h> 

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
        std::string webRoot;
        void handleRead();
        void handleWrite();
        bool isKeepAlive = false;
        void loadHtmlFile(const std::string &filename);
        


        //AGAUDARE POSIBILITATE DE GESTIONARE TIPURI DE CERERI
        void handleHead();
        void handlePost();
        void handlePut();
        void handleDelete();
        std::string getRequestedPage();
        void handleGet();
        void handleWriteLargeResponse(int fileFd);
        const char* get_file_type(char* filename);

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