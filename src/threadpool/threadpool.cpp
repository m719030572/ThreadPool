#include "threadpool.h"

namespace threadpool {
void ThreadPool::Worker() {
    std::thread::id id = std::this_thread::get_id();
    LOG(INFO) << " Worker " << id << " created.";
    std::shared_ptr<std::function<void ()>> task;
    while (running) {
        if (!pause && (task = task_queue_.TryPop()) != nullptr) {
            LOG(INFO) << " Worker " << id << " Working.";
            (*task)();
            LOG(INFO) << " Worker " << id << " Working done.";
        } else {
            LOG(INFO) << " Worker " << id << " Waiting.";
            std::unique_lock<std::mutex> lock(worker_mutex_);
            --working_thread_;
            worker_cond_.wait(lock, [&]{return !running || !task_queue_.Empty();});
        }

    }
    LOG(INFO) << " Worker " << id << " exit.";
}
void ThreadPool::Dispatcher() {
    while (running) {
        int now_task_size = task_queue_.Size();
        std::unique_lock<std::mutex> lock(worker_mutex_);
        int now_worker_count = working_thread_;
        lock.unlock();
        int need_created_size = (now_task_size - now_worker_count)/3;
        if (need_created_size < 0) {
            need_created_size = 0;
        }
        if (need_created_size + now_worker_count > thread_count_) {
            need_created_size = thread_count_ - now_worker_count;
        }
        while (need_created_size--) {
            worker_cond_.notify_one();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    worker_cond_.notify_all();
}
}