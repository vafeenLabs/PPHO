#include "ThreadPool.hpp"

void ThreadPool::run()
{
    while (true)
    {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]
                           { return stop || !tasks.empty(); });
            if (stop && tasks.empty())
                return;
            if (!tasks.empty())
            {
                task = std::move(tasks.front());
                tasks.pop();
            }
        }

        if (task)
        {
            task();
        }
    }
}


// Метод для добавления задачи в пул и получения результата через future
std::future<void> ThreadPool::enqueue(std::function<void()> task)
{
    auto taskPtr = std::make_shared<std::packaged_task<void()>>(std::move(task));
    std::future<void> res = taskPtr->get_future();

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([taskPtr]()
                      { (*taskPtr)(); });
    }
    condition.notify_one();
    return res;
}

// Конструктор для инициализации пула потоков с заданным количеством потоков
ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back([this]
                             { run(); });
}

// Деструктор завершает работу потоков
ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
        if (worker.joinable())
            worker.join();
}