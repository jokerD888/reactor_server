#pragma once
#include <sys/syscall.h>
#include <unistd.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic_bool stop_;
    std::string thread_type_;

public:
    ThreadPool(size_t thread_num, const std::string& thread_type);
    void AddTask(std::function<void()> task);
    ~ThreadPool();
};