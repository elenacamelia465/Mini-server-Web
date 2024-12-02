#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <netinet/in.h>
#include <csignal>
#include "Utility/Utility.h"
#include "ThreadPool/ThreadPool.h"
#include "TimeList/TimeList.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>


class WebServer
{
    private:
        int epollFd;
        epoll_event events[Config::maxEpollWaitEvent];
        std::vector<HttpConnection> httpConnections;
        int listenFd;
        uint16_t port;
        ThreadPool threadPool;
        std::string webRoot;
        int pipeFd[2];

        void handleSignal();

    public:
        WebServer(const uint16_t &port, const unsigned short &numWorkers, std::string webRoot);
        virtual ~WebServer();
        void eventLoop();
        void handleCloseOrError(const int &fd);
        void handleComingConnection();
        void handleInput(const int &fd);
        void handleOutput(const int &fd);
};

#endif