#include "ThreadPool.hpp"

// Константы для вычислений
const int N = 4;          // Количество потоков
const int STEPS = 1000;    // Количество шагов для интегрирования
const double A = -25;      // Левая граница интегрирования
const double B = 5;        // Правая граница интегрирования
std::mutex mut;            // Мьютекс для синхронизации доступа к глобальным переменным
std::mutex cout_mut;       // Мьютекс для синхронизации вывода в консоль

// Функция для вычисления значения функции в точке x
double f(double x);

// Функция для обработки сегмента, вычисления минимума, максимума и интеграла для поддиапазона
void counting_segment(double left, double right, int steps, double &global_min,
                      double &global_max, double &global_integral, int id_thread, bool check_main_thread);

int main()
{
    setlocale(LC_ALL, "Russian");  // Устанавливаем локаль для корректного отображения текста на русском
    auto total_start_time = std::chrono::high_resolution_clock::now();  // Засекаем время начала выполнения программы

    double segment_length = (B - A) / N;  // Длина одного сегмента для каждого потока
    double global_min = f(A);             // Инициализация глобального минимума
    double global_max = f(A);             // Инициализация глобального максимума
    double global_integral = 0;           // Инициализация глобального значения интеграла

    ThreadPool pool; // Используем пул потоков для параллельного вычисления

    std::cout << "Создание потоков\n";

    // Распределение задач между потоками
    for (int i = 0; i < N - 1; ++i)
    {
        double left = A + i * segment_length;           // Левая граница текущего сегмента
        double right = left + segment_length;          // Правая граница текущего сегмента

        // Добавляем задачу в пул потоков
        pool.enqueue([=, &global_min, &global_max, &global_integral]()
                     { counting_segment(left, right, STEPS / N, global_min, global_max, global_integral, i + 1, false); });
    }

    // Последний сегмент обрабатывается основным потоком
    double left = A + (N - 1) * segment_length;   // Левая граница последнего сегмента
    double right = B;                             // Правая граница последнего сегмента
    counting_segment(left, right, STEPS / N, global_min, global_max, global_integral, N, true);  // Обработка последнего сегмента основным потоком

    // Завершаем выполнение всех задач в пуле
    // Мы не нуждаемся в явном ожидании завершения, так как потоки автоматически синхронизируются с помощью std::future
    auto total_end_time = std::chrono::high_resolution_clock::now();   // Засекаем время окончания выполнения программы
    auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(total_end_time - total_start_time);  // Время выполнения программы

    // Выводим результаты
    std::cout << "-------------------------------------------\n";
    std::cout << "\nИтоговый результат:\n";
    std::cout << "минимум = " << std::fixed << std::setprecision(6) << global_min << "\n";  // Минимальное значение функции
    std::cout << "максимум = " << std::fixed << std::setprecision(6) << global_max << "\n";  // Максимальное значение функции
    std::cout << "интеграл = " << std::fixed << std::setprecision(6) << global_integral << "\n";  // Интеграл
    std::cout << "общее затраченное время: " << total_duration.count() << " микросекунд\n";  // Время выполнения программы

    return 0;
}

// Функция для вычисления значения функции в точке x
double f(double x)
{
    double f;
    if (x < -20)
    {
        f = 0.1 * x;  // Линейная функция для значений x < -20
    }
    else if (x < -5)
    {
        f = 0.5 * sin(0.25 * x) + 2.2 * cos(0.01 * x);  // Сложная функция для значений -20 <= x < -5
    }
    else
    {
        f = pow(x, 5) - pow(x, 4) + pow(x, 2) - x + 1;  // Полиномиальная функция для значений x >= -5
    }
    return f;
}

// Функция для обработки сегмента, вычисления минимума, максимума и интеграла для поддиапазона
void counting_segment(double left, double right, int steps, double &global_min,
                      double &global_max, double &global_integral, int id_thread, bool check_main_thread)
{
    auto start_time = std::chrono::high_resolution_clock::now();  // Засекаем время начала обработки сегмента
    double dx = (right - left) / steps;  // Шаг по x
    double local_min = f(left);          // Минимальное значение на сегменте
    double local_max = f(left);          // Максимальное значение на сегменте
    double local_integral = 0;           // Интеграл на сегменте

    // Проходим по всем точкам сегмента и вычисляем минимум, максимум и интеграл
    for (int i = 0; i < steps; ++i)
    {
        double x = left + i * dx;  // Текущая точка
        double y = f(x);           // Значение функции в точке x
        local_min = std::min(local_min, y);  // Находим минимум
        local_max = std::max(local_max, y);  // Находим максимум
        local_integral += y * dx;  // Вычисляем интеграл
    }

    // Блокируем доступ к глобальным переменным для безопасного их обновления
    std::lock_guard<std::mutex> guard(mut);
    global_min = std::min(global_min, local_min);  // Обновляем глобальный минимум
    global_max = std::max(global_max, local_max);  // Обновляем глобальный максимум
    global_integral += local_integral;  // Обновляем глобальный интеграл

    auto end_time = std::chrono::high_resolution_clock::now();  // Засекаем время окончания обработки сегмента
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);  // Время обработки сегмента

    // Блокируем вывод в консоль для синхронизации
    std::lock_guard<std::mutex> cout_guard(cout_mut);
    std::cout << "---------------------------------------------\n";
    std::cout << (check_main_thread ? "Родительский поток" : "Дочерний поток")
              << " ID: " << std::this_thread::get_id() << "\n";  // Выводим ID потока
    std::cout << "Processed Range: [" << left << ", " << right << "]\n";  // Выводим обрабатываемый диапазон
    std::cout << "минимум = " << std::fixed << std::setprecision(6) << local_min << "\n";  // Минимум для текущего сегмента
    std::cout << "максимум = " << std::fixed << std::setprecision(6) << local_max << "\n";  // Максимум для текущего сегмента
    std::cout << "интеграл = " << std::fixed << std::setprecision(6) << local_integral << "\n";  // Интеграл для текущего сегмента
    std::cout << "время: " << duration.count() << " микросекунд\n";  // Время обработки текущего сегмента
}
