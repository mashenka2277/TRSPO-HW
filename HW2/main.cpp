#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>

std::queue<int> task_queue; // Черга завдань
std::mutex q_mutex;

long long total_steps = 0; // Загальна к-ть кроків для всіх чисел
std::mutex ts_mutex;

std::mutex cout_mutex;

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

void processTasks(int thread_id)
{
    while (true)
    {
        int number = -1;

        {
            std::lock_guard<std::mutex> lock(q_mutex);
            if (task_queue.empty())
                break; // Якщо черга порожня - цикл завершується

            number = task_queue.front();
            task_queue.pop();
        }

        int steps = collatzSteps(number);

        {
            std::lock_guard<std::mutex> lock(ts_mutex);
            total_steps += steps;
        }

        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Thread " << thread_id << "] Processed number: " << number << ", Steps: " << steps << std::endl;
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Затримка 5 мс
    }
}

int main()
{
    const int MAX_NUMBER = 100000; // Верхня межа діапазону, що розглядається (Зменшено до 100 000)
    const int THREAD_COUNT = 4; // К-ть потоків

    auto start_time = std::chrono::high_resolution_clock::now();

    // Додаємо завдання до черги
    for (int i = 1; i <= MAX_NUMBER; i++)
        task_queue.push(i);

    std::cout << "Starting Collatz computation with " << THREAD_COUNT << " threads...\n";

    std::vector<std::thread> threads; // Вектор з потоками

    // Запускаємо всі потоки, передаючи їх індекс як параметр
    for (int i = 0; i < THREAD_COUNT; i++)
        threads.emplace_back(processTasks, i + 1);

    // Очікуємо завершення всіх потоків
    for (auto& t : threads)
        t.join();

    // Розраховуємо середню к-ть кроків
    double average_steps = static_cast<double>(total_steps) / MAX_NUMBER;

    auto end_time = std::chrono::high_resolution_clock::now();

    std::cout << "\nAverage number of steps: " << average_steps << std::endl;
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms" << std::endl;
    
    return 0;
}