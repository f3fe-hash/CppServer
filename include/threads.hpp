#ifndef __THREADS_HPP__
#define __THREADS_HPP__

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <stdexcept>
#include <functional>

#include "memory.hpp"

// A task that consists of a function and arguments
typedef struct
{
    void(*task)(void *);
    void* args;
} Task;

extern Allocator* _alloc;

class ThreadPool
{
    std::vector<std::thread> workers;
    std::queue<Task> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

public:
    ThreadPool(size_t max_threads) __THROW;
    ~ThreadPool() __THROW;

    // Enqueue a raw Task (C-style)
    void enqueue(Task task);

    // Enqueue using C++ function and arguments
    template<class F, class... Args>
    auto enqueue_cpp(F&& f, Args&&... args) __THROW
        -> std::future<typename std::result_of<F(Args...)>::type>;
};

Task* new_task(void (*func)(void*), void* args) __THROW __nonnull((1));

#endif
