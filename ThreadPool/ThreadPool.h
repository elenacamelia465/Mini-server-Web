#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <condition_variable>
#include <queue>
#include <thread>
#include <chrono> 
#include <iomanip>
#include <sstream>
#include "../HttpConnection/HttpConnection.h"

class ThreadPool
{
    private:
        struct PriorityTask
        {
            int priority; //imiplementam un MPM APACHE SI mai adaugam threaduri de prioritati
            HttpConnection *connection;
            bool operator<(const PriorityTask &other)const{
                return priority < other.priority; //reverse pentru prioritatea mai importanta prima
            }
        };
        std::condition_variable condition;
        std::mutex mutex;
       // std::queue<HttpConnection *> tasks;
        std::priority_queue<PriorityTask> tasks;
        std::vector<std::thread> workers;
        bool stop;

    public:
        explicit ThreadPool(const unsigned short &numWorkers);
        virtual ~ThreadPool();
        void addTask(HttpConnection *connection, int priority = 0);
        void resizePool(short unsigned int newWorkerCount);
        void quitLoop();

    private:
        void workerThread();
};

#endif