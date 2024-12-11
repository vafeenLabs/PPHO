#include "ThreadPool.hpp"

// Функция для расчета числа Фибоначчи
long long fibonacci(int n)
{
    if (n <= 2)
        return 1;
    else
        return fibonacci(n - 1) + fibonacci(n - 2);
}

// Функция записи текста в файл
void writeToFile(const std::string &filename, const std::string &content)
{
    std::ofstream file(filename);
    if (file.is_open())
    {
        file << content;
        file.close();
        std::cout << "Текст записан в файл " << filename << std::endl;
    }
    else
    {
        std::cerr << "Не удалось открыть файл для записи!" << std::endl;
    }
}

int main()
{
    ThreadPool pool;
    int choice;
    bool program = true;
    while (program)
    {
        std::cout << "Выберите команду (1: Фибоначчи, 2: Запись в файл, 3: Выход, 4: Описание работы программы): ";
        std::cin >> choice;

        switch (choice)
        {
        case FIBONACHI_CHOICE:
        {
            int n;
            std::cout << "Введите число для расчета Фибоначчи: ";
            std::cin >> n;
            std::cout << "Начали расчет числа Фибоначчи под номером " << n << ", ожидайте. А пока можете воспользоваться файловым вводом или посчитать еще одно число\n";
            pool.enqueue([n]()
                         {
                long long fibResult = fibonacci(n);
                std::cout << "Число Фибоначчи для " << n << ": " << fibResult << std::endl; });
            break;
        }
        case FILE_WRITING_CHOICE:
        {
            std::string filename, content;
            std::cout << "Введите имя файла: ";
            std::cin >> filename;
            std::cin.ignore();
            std::cout << "Введите текст для записи в файл: ";
            std::getline(std::cin, content);

            pool.enqueue([filename, content]()
                         { writeToFile(filename, content); });
            break;
        }
        case DESCRIPTION_CHOICE:
        {
            std::cout << "Описание работы программы:\n";
            std::cout << "Данная программа демонстрирует работу многопоточности.\n";
            std::cout << "Вы можете одновременно рассчитывать число Фибоначчи и записывать данные в файл.\n";
            std::cout << "Во время вычисления числа Фибоначчи вы можете продолжать взаимодействовать с программой.\n";
            std::cout << "Когда расчет числа Фибоначчи завершится, результат будет выведен на экран.\n";
            break;
        }
        case EXIT_CHOICE:
        {
            program = false;
            break;
        }
        default:
        {
            std::cout << "Неверная команда!" << std::endl;
            break;
        }
        }
    }

    return 0;
}