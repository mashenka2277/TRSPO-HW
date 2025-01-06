#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <chrono>

class ThreadPool {
private:
    std::vector<std::thread> work_threads; // Вектор з потоками
    std::queue<std::function<void()>> tasks; // Черга завдань
    std::mutex q_mutex; 
    std::condition_variable condition; // Умовна змінна для управління потоками
    bool stop; // Флаг завершення роботи

public:
    // Конструктор (створюємо потоки)
    ThreadPool(size_t threads): stop(false) {
        for (size_t i = 0; i < threads; i++)
        {
            work_threads.emplace_back([this] (){
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->q_mutex);
                        this->condition.wait(lock, [this] (){
                            // Потік чекає, поки АБО завершиться робота АБО у черзі з'явиться нове завдання
                            return this->stop || !this->tasks.empty();
                        });

                        // Перевірка на завершення роботи
                        if (this->stop && this->tasks.empty())
                            return;

                        // Забираємо завдання з черги
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task(); // Виконання завдання
                }
            });
        }
    }

    // Метод, що додає завдання в чергу
    template <class F>
    void enqueue(F&& f)
    {
        {
            std::lock_guard<std::mutex> lock(q_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one(); // Сигналізуємо про те, що в черзі з'явилось нове завдання
    }

    // Деструктор (завершення роботи потоків)
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(q_mutex);
            stop = true;
        }
        condition.notify_all(); // Сигналізуємо про те, що пул завершує роботу

        // Чекаємо завершення роботи всіх потоків
        for (std::thread& work_thread : work_threads)
            work_thread.join();
    }

};

std::atomic<long long> total_steps(0); // Загальна к-ть кроків
std::atomic<int> completed_tasks{0}; // К-ть виконаних завдань (для перевірки виконання всіх)

// Функція для обчислення "градин" (к-ті кроків до 1)
int collatzSteps(int n) 
{
    int steps = 0;
    while (n != 1)
    {
        if (n % 2 == 0)
        {
            // Якщо число парне - ділимо на 2
            n /= 2;
        }
        else 
        {
            // Якщо непарне - множимо на 3 і додаємо 1
            n = 3 * n + 1;
        }
        steps++;
    }
    return steps;
}

void processCollatz(int number) 
{
    int steps = collatzSteps(number);
    total_steps.fetch_add(steps, std::memory_order_relaxed); // Атомарне збільшення
    completed_tasks.fetch_add(1, std::memory_order_seq_cst); // Збільшуємо лічильник
}

int main()
{
    const int MAX_NUMBER = 100000; // Верхня межа діапазону, що розглядається (Зменшено до 100 000)
    const int THREAD_COUNT = 6; // К-ть потоків

    std::cout << "Starting Collatz computation with ThreadPool..." << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    ThreadPool pool(THREAD_COUNT); // Створюємо пул потоків

    // Додаємо завдання у чергу
    for (int i = 1; i <= MAX_NUMBER; i++)
    { 
        pool.enqueue([i] (){
            processCollatz(i);
        });
    }

    // Очікуємо завершення всіх завдань через атомарну змінну
    while (completed_tasks < MAX_NUMBER) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    double average_steps = static_cast<double>(total_steps.load()) / MAX_NUMBER;

    auto end_time = std::chrono::high_resolution_clock::now();

    std::cout << "\nAverage number of steps: " << average_steps << std::endl;
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms" << std::endl;

    return 0;
}