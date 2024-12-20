#include <iostream>
#include <thread> // Ліба для використання потоків
#include <chrono>

// Функція для обчислення суми двох чисел
int addNumbers(int a, int b) 
{
    return a + b;
}

// Функція для обчислення різниці двох чисел
int subtractNumbers(int a, int b) 
{
    return a - b;
}

// Функція для паралельного обчислення суми чисел у проміжку
void calculateSumInRange(int start, int end) 
{
    int sum = 0;
    for (int i = start; i <= end; ++i) 
    {
        sum += i;
        std::cout << "Step " << i << ": " << sum << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "The sum of numbers from " << start << " to " << end << " is " << sum << "." << std::endl;
}

// Функція для паралельного обчислення різниці чисел у проміжку
void calculateDifferenceInRange(int start, int end) 
{
    int diff = 0;
    for (int i = start; i <= end; ++i)
    {
        diff -= i;
        std::cout << "Step " << i << ": " << diff << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }    
    std::cout << "The difference of numbers from " << start << " to " << end << " is " << diff << "." << std::endl;
}

int main() 
{
    std::cout << "Hello, World!" << std::endl;

    std::cout << "Now you can see the parallel search for the sum and difference of range from 1 to 10." << std::endl;

    int start = 1;
    int end = 10;

    // Два потоки для розрахунків
    std::thread sumThread(calculateSumInRange, start, end);
    std::thread diffThread(calculateDifferenceInRange, start, end);

    // Очікуємо завершення обох потоків
    sumThread.join();
    diffThread.join();

    return 0;
}