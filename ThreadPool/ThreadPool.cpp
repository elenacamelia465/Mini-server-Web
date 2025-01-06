#include "ThreadPool.h"
#include <chrono> //pentru a calc timpul
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
    resizePool(numWorkers); //INITIALIZARE slavi dinamic
}

ThreadPool::~ThreadPool()
{
    quitLoop();
}

void ThreadPool::addTask(HttpConnection *connection, int priority)
{
     {
        std::unique_lock<std::mutex> locker(mutex);
        tasks.push({priority, connection});  //push in priority queue
        std::cout << "[Main Thread] Task added for FD: " 
                  << connection->getFd() << " with priority: " << priority << "\n";
    }
    condition.notify_one();
}
//resize sclavi thread pool dynamically
void ThreadPool::resizePool(unsigned short newWorkerCount) 
{
    std::unique_lock<std::mutex> locker(mutex);
    unsigned short currentWorkers = workers.size();

    if (newWorkerCount > currentWorkers) {
        for (unsigned short i = currentWorkers; i < newWorkerCount; i++) 
        {
            workers.emplace_back(&ThreadPool::workerThread, this);
            std::cout << "[ThreadPool] Added new worker thread.\n";
        }
    } else if (newWorkerCount < currentWorkers) 
    {
        stop = true;  // Stop current workers to resize
        condition.notify_all();
        for (std::thread &worker : workers) 
        {
            if (worker.joinable()) worker.join();
        }
        workers.clear();
        stop = false;

        // Restart with fewer threads
        for (unsigned short i = 0; i < newWorkerCount; i++) 
        {
            workers.emplace_back(&ThreadPool::workerThread, this);
        }
        std::cout << "[ThreadPool] Resized to " << newWorkerCount << " workers.\n";
    }
}

//Sclav thread pool in functie de coada de prioritati asa actioneaza , nu cum au ei chef
void ThreadPool::workerThread() 
{
    std::thread::id threadId = std::this_thread::get_id();
    while (true) {
        PriorityTask task;
        {
            std::unique_lock<std::mutex> locker(mutex);
            condition.wait(locker, [this]() { return !tasks.empty() || stop; });

            if (stop && tasks.empty()) 
            {
                std::cout << "[Thread " << threadId << "] Exiting...\n";
                return;
            }

            task = tasks.top();
            tasks.pop();
        }

        std::cout << "[Thread " << threadId << "] Starting task for FD: "
                  << task.connection->getFd() << "\n";
        auto start = std::chrono::high_resolution_clock::now();

        try 
        {
            task.connection->handleRequest();
        } catch (const std::exception &e) 
        {
            std::cerr << "[Thread " << threadId << "] Exception: " << e.what() << "\n";
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "[Thread " << threadId << "] Task completed for FD: "
                  << task.connection->getFd() << " in " << elapsed.count()
                  << " seconds.\n";
    }
}
//Oprire thread pool
void ThreadPool::quitLoop() 
{
    {
        std::unique_lock<std::mutex> locker(mutex);
        stop = true;
    }
    condition.notify_all();

    for (std::thread &worker : workers) 
    {
        if (worker.joinable()) worker.join();
    }
    workers.clear();
    std::cout << "[ThreadPool] All threads stopped.\n";
}
