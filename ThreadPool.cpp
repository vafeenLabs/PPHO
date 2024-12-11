#include "ThreadPool.hpp"

// Метод для выполнения задач в пуле потоков
void ThreadPool::run()
{
    while (true)
    {
        std::function<void()> task;

        // Блокировка очереди задач для безопасного доступа к ней
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            // Ожидаем, пока не появится задача или пул не будет остановлен
            condition.wait(lock, [this]
                           { return stop || !tasks.empty(); });

            // Если пул остановлен или задач в очереди нет, выходим
            if (stop || tasks.empty())
                return;

            // Извлекаем задачу из очереди
            if (!tasks.empty())
            {
                task = std::move(tasks.front());
                tasks.pop();
            }
        }

        // Выполняем задачу
        if (task)
        {
            task();
        }
    }
}

// Метод для добавления задачи в пул и получения результата через future
std::future<void> ThreadPool::enqueue(std::function<void()> task)
{
    // Создаём packaged_task для упаковки задачи, которая будет выполнена
    auto taskPtr = std::make_shared<std::packaged_task<void()>>(std::move(task));
    // Получаем future, чтобы можно было получить результат выполнения задачи
    std::future<void> res = taskPtr->get_future();

    // Блокировка очереди задач для безопасного добавления задачи
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        // Если пул остановлен, выбрасываем исключение
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        // Добавляем задачу в очередь
        tasks.emplace([taskPtr]()
                      { (*taskPtr)(); });
    }
    // Уведомляем один из потоков, что появилась новая задача
    condition.notify_one();

    // Возвращаем future, чтобы можно было получить результат задачи
    return res;
}

// Конструктор для инициализации пула потоков с заданным количеством потоков
ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    // Создаём заданное количество потоков, каждый из которых выполняет метод run()
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back([this]
                             { run(); });
}

// Деструктор, который завершает работу всех потоков
ThreadPool::~ThreadPool()
{
    {
        // Блокировка очереди задач перед остановкой пула
        std::lock_guard<std::mutex> lock(queueMutex);
        stop = true;
    }

    // Уведомляем все потоки, что пул должен быть остановлен
    condition.notify_all();

    // Ожидаем завершения всех рабочих потоков
    for (std::thread &worker : workers)
        if (worker.joinable())
            worker.join();
}
