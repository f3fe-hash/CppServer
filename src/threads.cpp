#include "threads.hpp"

ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    for(size_t i = 0; i < threads; ++i)
    {
        workers.emplace_back([this]
        {
            for(;;)
            {
                Task task;

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this]
                    {
                        return this->stop || !this->tasks.empty();
                    });

                    if(this->stop && this->tasks.empty())
                        return;

                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                if (task.task)
                    task.task(task.args);
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }

    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

void ThreadPool::enqueue(Task task)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace(std::move(task));
    }
    condition.notify_one();
}

// C++ style enqueue: converts function + args to raw Task
template<class F, class... Args>
auto ThreadPool::enqueue_cpp(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto bound_task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = bound_task->get_future();

    // Allocate a struct to wrap the task pointer
    auto task_wrapper = new std::packaged_task<return_type()>(std::move(*bound_task));

    Task task;
    task.task = [](void* arg)
    {
        auto t = static_cast<std::packaged_task<return_type()>*>(arg);
        (*t)();
        delete t; // clean up after execution
    };
    task.args = task_wrapper;

    enqueue(task);
    return res;
}