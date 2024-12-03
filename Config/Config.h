#ifndef WEBSERVER_CONFIG_H
#define WEBSERVER_CONFIG_H

#include <unistd.h>
#include <getopt.h>
#include <cstdint>
#include <string>
#define DATA_LENGTH 1024 


class Config
{
    public:
        static constexpr unsigned int maxConnectFd = 65536;
        static constexpr unsigned int maxEpollWaitEvent=10000;
        static constexpr unsigned int sendRecvBufSize=10240;
        static unsigned short threadPoolNumWorkers;

        static constexpr unsigned short defaultTimeoutSecond = 20;
        static unsigned short timeoutSecond; //pentru -t option 
        static const std::string webRoot;
        static uint16_t webServerPort;
        static bool isHttpsEnabled; //pentru https

        Config();
        virtual ~Config();
        static void parseArgs(int argc, char **argv);
        static void displayBanner();
};

#endif