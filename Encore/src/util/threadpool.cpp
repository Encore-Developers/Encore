#include "threadpool.h"

#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"
void ThreadPool::ThreadRun() {
    TracyCSetThreadName("Thread Pool Member")
    while (true) {
        {
            ZoneScopedN("Wait for Task")
            tasksSem.acquire();
        }
        tasksMutex.lock();
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
    for (size_t i = 0; i < threadCount; ++i) {
        threads.push_back(std::thread(&ThreadPool::ThreadRun, this));
    }
}
void ThreadPool::SubmitTask(std::function<void()> task) {
    tasksMutex.lock();
    tasks.push_front(task);
    tasksMutex.unlock();
    tasksSem.release();
}
ThreadPool::~ThreadPool() {
    ZoneScopedN("Wait for Task Finish")
    shutdown = true;
    tasksSem.release(threadCount);
    for (size_t i = 0; i < threadCount; ++i) {
        threads[i].join();
    }
}