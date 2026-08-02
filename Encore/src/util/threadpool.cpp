#include "threadpool.h"

#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"
void ThreadPool::ThreadRun() {
    TracyCSetThreadName("Thread Pool Member")
    while (true) {
        tasksSem.acquire();
        {
            ZoneScopedN("Lock Tasks")
            tasksMutex.lock();
        }
        if (shutdown && tasks.empty()) {
            tasksMutex.unlock();
            return;
        }
        auto func = tasks.front();
        tasks.pop_front();
        tasksMutex.unlock();
        func();
    }
}
ThreadPool::ThreadPool(unsigned int threadCount) : tasksSem(0), threadCount(threadCount) {
    if (this->threadCount < 1) {
        this->threadCount = 1;
    }
    for (size_t i = 0; i < threadCount; ++i) {
        threads.push_back(std::thread(&ThreadPool::ThreadRun, this));
    }
}
void ThreadPool::SubmitTask(std::function<void()> task) {
    ZoneScoped
    {
        ZoneScopedN("Lock Tasks")
        tasksMutex.lock();
    }
    tasks.push_back(task);
    tasksMutex.unlock();
    tasksSem.release();
}
void ThreadPool::Detach() {
    waitsForShutdown = false;
    for (auto &thread : threads) {
        thread.detach();
    }
}
ThreadPool::~ThreadPool() {
    ZoneScopedN("Wait for Task Finish")
    shutdown = true;
    tasksSem.release(threadCount);
    if (!waitsForShutdown) {
        return;
    }
    for (size_t i = 0; i < threadCount; ++i) {
        threads[i].join();
    }
}