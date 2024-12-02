#ifndef WEBSERVER_HTTPCONNECTION_H
#define WEBSERVER_HTTPCONNECTION_H

#include <sys/socket.h>
#include <iostream>
#include <sys/epoll.h>
#include <unordered_map>
#include <filesystem>
#include <cstring>
#include "../Config/Config.h"
#include "HttpRequest.h"
#include <filesystem>


//------ADAUGARE GESTIONARE TIPURI DE CERERI
#include "../file_types.h"
#include <sys/stat.h> 
#include <openssl/ssl.h>

class HttpConnection
{
    private:
        //epoll instance is used for managing events related to this connection
        
        SSL *ssl = nullptr;
        int epollFd;
        int fd;
        bool isReadEvent;
        char recvBuf[Config::sendRecvBufSize];
        unsigned long recvIndex;
        char sendBuf[Config::sendRecvBufSize];

        std::string responseHeader;
        std::string responseBody; // Pentru datele binare
        size_t headerIndex = 0;
        size_t bodyIndex = 0;

        unsigned long sendIndex;
        std::string webRoot;
        void handleRead();
        void handleWrite();
        bool isKeepAlive = false;
        void loadHtmlFile(const std::string &filename);
        


        //AGAUDARE POSIBILITATE DE GESTIONARE TIPURI DE CERERI
        void handleHead(HttpRequest request);
        void handlePost(HttpRequest request);
        void handlePut(HttpRequest request);
        void handleDelete(HttpRequest request);
        std::string getRequestedPage();
        void handleGet(HttpRequest request);
        void finalizeConnection(); 

        void handleWriteLargeResponse(int fileFd);
        //const char* get_file_type(char* filename);

    public:
        HttpConnection();
        virtual ~HttpConnection();
        void closeConnection() const;
        void handleRequest();
        void init(const int &epollFd, const int &fd,SSL *ssl ); //initializare cu ssl
        void setEvent(const bool &isReadEvent);
        int getFd() const;
        std::string getContentType(const std::string& filePath);
        std::string decompressGzip(const std::string& str);
        bool isValidUsername(const std::string& username);
        bool isValidPassword(const std::string& password);
        bool authenticateUser(const std::string& username, const std::string& password);
        std::string urlDecode(const std::string& str);
        std::unordered_map<std::string, std::string> parseFormData(const std::string& body);
        std::string compressStringGzip(const std::string& str);

};

#endif