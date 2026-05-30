#pragma once
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class ThreadPool {
    std::vector<std::thread> threads;
    std::deque<std::function<void()>> tasks;
    std::mutex tasksMutex;
    std::counting_semaphore<999999> tasksSem;
    unsigned int threadCount;
    volatile bool shutdown = false;

    void ThreadRun();

public:
    ThreadPool(unsigned int threadCount);
    void SubmitTask(std::function<void()> task);

    ~ThreadPool();
};
