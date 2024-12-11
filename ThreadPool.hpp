#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>

enum Point
{
    FIBONACHI_CHOICE = 1,
    FILE_WRITING_CHOICE,
    EXIT_CHOICE,
    DESCRIPTION_CHOICE,

};

// Класс ThreadPool для управления пулом потоков
class ThreadPool
{
public:
    // Конструктор для инициализации пула потоков с заданным количеством потоков
    ThreadPool(size_t threads = std::thread::hardware_concurrency());

    // Деструктор завершает работу потоков
    ~ThreadPool();

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    // Метод для добавления задачи в пул и получения результата через future
    std::future<void> enqueue(std::function<void()> task);

private:
    // Метод, выполняющий задачи в потоках
    void run();

    std::vector<std::thread> workers;        // Вектор рабочих потоков
    std::queue<std::function<void()>> tasks; // Очередь задач
    std::mutex queueMutex;                   // Мьютекс для защиты очереди задач
    std::condition_variable condition;       // Условная переменная для синхронизации
    bool stop;                               // Флаг остановки пула потоков
};
