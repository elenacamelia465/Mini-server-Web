#include "ThreadPool.h"

// Helper pentru timestamp-uri
std::string getCurrentTimestamp() 
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
ThreadPool::ThreadPool(const unsigned short &numWorkers) : stop(false)
{
    for(unsigned short i=0; i<numWorkers; i++)
    {
        std::thread thread(ThreadPool::work, this);
        thread.detach();
    }
}

ThreadPool::~ThreadPool()  = default;

void ThreadPool::addTask(HttpConnection *connection)
{
     {
        std::unique_lock<std::mutex> locker(mutex);
        tasks.push(connection);
        std::cout << "[Main Thread] Task added for FD: " << connection->getFd() << "\n";
    }
    condition.notify_one();
}

void ThreadPool::quitLoop()
{
    {
        std::unique_lock<std::mutex> locker(mutex);
        stop=true;
    }
    condition.notify_all();
}

#include <chrono>  // pentru calcularea timpului

void ThreadPool::run() 
{
    std::unique_lock<std::mutex> locker(mutex);
    std::thread::id threadId = std::this_thread::get_id();
    while (!stop) 
    {
        if (tasks.empty()) 
        {
            std::cout << "[" << getCurrentTimestamp() << "] [Thread " << threadId << "] Waiting for tasks...\n";
            condition.wait(locker);
        } 
        else 
        {
            HttpConnection *task = tasks.front();
            tasks.pop();
            std::cout << "[Thread " << threadId << "] Starting task for FD: " << task->getFd() << "\n";
            locker.unlock();

            auto start = std::chrono::high_resolution_clock::now();
            task->handleRequest();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;

            std::cout << "[Thread " << threadId << "] Task completed for FD: " << task->getFd()
                      << " in " << elapsed.count() << " seconds.\n";

            locker.lock();
        }
    }
    std::cout << "[Thread " << threadId << "] Exiting...\n";
}

void *ThreadPool::work(void *pool)
{
    auto *threadPool = (ThreadPool*) pool;
    threadPool->run();
    return threadPool;
}