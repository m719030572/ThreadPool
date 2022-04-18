#ifndef THREAD_POOL_TASK_TASK_H
#define THREAD_POOL_TASK_TASK_H
class Task {
public:
    virtual bool Run() = 0;
};
#endif