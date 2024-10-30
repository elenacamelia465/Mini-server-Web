#ifndef WEBSERVER_UTILITY_H
#define WEBSERVER_UTILITY_H

#include <mutex>
#include <netinet/in.h>
#include <fcntl.h>
#include <csignal>
#include "../TimeList/TimeList.h"

class Utility
{
    public:
        Utility();
        virtual ~Utility();
        static bool stop;
        static TimeList timeList;
        static std::mutex mutex;
        static int *pipeFd;
        static void addFdToEpoll(const int &epollFD, const int &fd, const bool &oneShot = true);
        static int getListenFd(const uint16_t &port);
        static void setFdNonBlock(const int &fd);
        static void setSignal(const int &sig, void(*signalHandle)(int));
        static void signalAlrmHandler(int sig);
        static void signalSigintHandler(int sig);
};

#endif