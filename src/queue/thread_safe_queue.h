#ifndef THREAD_POOL_QUEUE_THREAD_SAFE_QUEUE_H
#define THREAD_POOL_QUEUE_THREAD_SAFE_QUEUE_H
#include<condition_variable>
#include<memory>
#include<mutex>
#include<queue>
namespace queue{
template<typename T>
class ThreadSafeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;
    };
public:
    ThreadSafeQueue();
    // 在队列尾加入数据
    void Push(const T& data);
    // 取出队列顶的数据，如果队列无数据，则阻塞
    std::shared_ptr<T> WaitPop();
    // 取出队列顶的数据，如果队列无数据，则返回空指针
    std::shared_ptr<T> TryPop();
    // 若队列为空，则返回True。
    bool Empty();
    uint64_t Size();
private:
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue operator=(const ThreadSafeQueue) = delete;
    Node* GetTail();
    std::unique_lock<std::mutex> WaitForData();
    std::unique_ptr<Node> PopHead();
    std::unique_ptr<Node> WaitPopHead();
    std::unique_ptr<Node> TryPopHead();
    void PushBack(std::unique_ptr<T>&& node_ptr);
    std::mutex head_mutex;
    std::mutex tail_mutex;
    std::unique_ptr<Node> head;
    Node* tail;
    std::condition_variable data_cond;
    uint64_t upper_size;
    uint64_t lower_size;
};

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue() : head(new Node()), tail(head.get()), upper_size(0), lower_size(0) {
}
template<typename T>
std::unique_lock<std::mutex> ThreadSafeQueue<T>::WaitForData() {
    std::unique_lock<std::mutex> head_lock(head_mutex);
    data_cond.wait(head_lock, [&]{return head.get() != GetTail();});
    return std::move(head_lock);
}
template<typename T>
typename ThreadSafeQueue<T>::Node* ThreadSafeQueue<T>::GetTail() {
    std::lock_guard<std::mutex> tail_lock(tail_mutex);
    return tail;
}
template<typename T>
std::unique_ptr<typename ThreadSafeQueue<T>::Node> ThreadSafeQueue<T>::PopHead() {
    ++lower_size;
    std::unique_ptr<Node> old_head = std::move(head);
    head = std::move(old_head->next);
    return old_head;
}
template<typename T>
std::unique_ptr<typename ThreadSafeQueue<T>::Node> ThreadSafeQueue<T>::WaitPopHead() {
    std::unique_lock<std::mutex> head_lock(WaitForData());
    return PopHead();
}
template<typename T>
std::unique_ptr<typename ThreadSafeQueue<T>::Node> ThreadSafeQueue<T>::TryPopHead() {
    std::lock_guard<std::mutex> head_lock(head_mutex);
    if (head.get() == GetTail()) {
        return std::unique_ptr<Node>();
    }
    return PopHead();
}
template<typename T>
void ThreadSafeQueue<T>::PushBack(std::unique_ptr<T>&& node_ptr) {
    std::unique_ptr<Node> new_node(new Node());
    std::unique_lock<std::mutex> tail_lock(tail_mutex);
    ++upper_size;
    tail->data = std::move(node_ptr);
    tail->next = std::move(new_node);
    tail = tail->next.get();
    tail_lock.unlock();
    data_cond.notify_one();
}

template<typename T>
bool ThreadSafeQueue<T>::Empty() {
    std::lock_guard<std::mutex> head_lock(head_mutex);
    return head.get() == GetTail();
}
template<typename T>
void ThreadSafeQueue<T>::Push(const T& data) {
    std::unique_ptr<T> node_ptr(new T(data));
    PushBack(std::move(node_ptr));
}
template<typename T>
std::shared_ptr<T> ThreadSafeQueue<T>::WaitPop() {
    std::unique_ptr<Node> node = WaitPopHead();
    return std::shared_ptr<T>(node->data);
}
template<typename T>
std::shared_ptr<T> ThreadSafeQueue<T>::TryPop() {
    std::unique_ptr<Node> node = TryPopHead();
    return node == nullptr ? std::shared_ptr<T>() : node->data;
}
template<typename T>
uint64_t ThreadSafeQueue<T>::Size() {
    head_mutex.lock();
    uint64_t size = lower_size;
    lower_size = 0;
    head_mutex.unlock();
    tail_mutex.lock();
    upper_size -= size;
    size = upper_size;
    tail_mutex.unlock();
    return size;
}
}
#endif
