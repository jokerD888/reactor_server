#include "thread_pool.h"

ThreadPool::ThreadPool(size_t thread_num) : stop_(false) {
    for (size_t i = 0; i < thread_num; ++i) {
        workers_.emplace_back([this]() {
            printf("create thread(%d).\n", syscall(SYS_gettid));
            while (!stop_) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->mutex_);
                    this->condition_.wait(lock, [this]() { return this->stop_ || !this->tasks_.empty(); });

                    if (!this->stop_ && this->tasks_.empty()) return;
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                printf("thread is %d.\n", syscall(SYS_gettid));
                task();
            }
        });
    }
}

void ThreadPool::AddTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.emplace(task);
    }
    condition_.notify_one();
}

ThreadPool::~ThreadPool() {
    stop_ = true;
    condition_.notify_all();
    for (auto& worker : workers_) {
        worker.join();
    }
}