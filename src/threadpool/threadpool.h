#ifndef THREAD_POOL_THREADPOOL_THREADPOOL_H
#define THREAD_POOL_THREADPOOL_THREADPOOL_H
#include<atomic>
#include<future>
#include<glog/logging.h>
#include<boost/thread/thread.hpp>
#include"queue/thread_safe_queue.h"
namespace threadpool {
class ThreadPool {

public:
    // 无返回值函数加入线程池所调用函数，
    // Func:可调用对象，
    // Args:调用时传入的参数
    // 此函数返回值：bool类型的future，可判断任务是否执行完毕。
    template<typename Func, typename...Args, typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<Func, Args...>>>>
    std::future<bool> Submit(const Func& func, const Args&&...);
    // 有返回值函数加入线程池所调用函数，
    // Func:可调用对象，
    // Args:调用时传入的参数
    // 此函数返回值：包含可调用对象的返回值的future。
    template<typename Func, typename...Args, typename ReturnType = std::invoke_result_t<Func, Args...>, typename = std::enable_if_t<!std::is_void_v<std::invoke_result_t<Func, Args...>>>>
    std::future<ReturnType> Submit(const Func& func, const Args&&...);
    // 构造函数，传入初始线程池中县城的数量，缺省值为机器的实际最大并发数。
    explicit ThreadPool(int thread_count = std::thread::hardware_concurrency()) : thread_count_(thread_count), working_thread_(thread_count_) {
        thread_list_.push_back(std::make_shared<std::thread>(&ThreadPool::Dispatcher, this));
        for (int i = 0; i < thread_count_ - 1; ++i) {
            thread_list_.push_back(std::make_shared<std::thread>(&ThreadPool::Worker, this));
        }
    }
    // 析构函数，
    // 立即停止所有新任务的执行，等待所有正在执行的任务执行完毕后析构。
    ~ThreadPool() {
        running = false;
        for (auto thread : thread_list_) {
            if (thread->joinable()) {
                thread->join();
            }

        }
    }
private:
    void Worker();
    void Dispatcher();
    template<typename Func, typename... Args, typename RetType>
    void PushTask(const Func& func, const Args&&... args, std::promise<RetType> promise) {

    }
    template<typename Task> 
    void PushTask(const Task& task);
    unsigned int thread_count_;
    std::atomic_bool running = true;
    std::atomic_bool pause = false;
    std::vector<std::shared_ptr<std::thread>> thread_list_;
    queue::ThreadSafeQueue<std::function<void(void)>> task_queue_;
    unsigned int working_thread_;
    std::condition_variable worker_cond_;
    std::mutex worker_mutex_;
};

template<typename Func, typename...Args, typename>
std::future<bool> ThreadPool::Submit(const Func& func, const Args&&... args) {
    std::shared_ptr<std::promise<bool>> promise(new std::promise<bool>);
    PushTask([=] {
        try {
            func(args...);
            promise->set_value(true);
        } catch (...) {
            try {
                promise->set_exception(std::current_exception());
            } catch (...) {

            }
        }
    });
    return promise->get_future();
}

template<typename Func, typename...Args, typename ReturnType, typename>
std::future<ReturnType> ThreadPool::Submit(const Func& func, const Args&&... args) {
    std::shared_ptr<std::promise<ReturnType>> promise(new std::promise<ReturnType>);
        PushTask([=] {
        try {
            promise->set_value(func(args...));
        } catch (...) {
            try {
                promise->set_exception(std::current_exception());
            } catch (...) {
                
            }
        }
    });
    return promise->get_future();
}

template<typename Task> 
void ThreadPool::PushTask(const Task& task) {
    task_queue_.Push(std::function<void(void)>(task));
}
}

#endif