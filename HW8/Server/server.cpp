#include <iostream>
#include <cstring>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <sys/socket.h> // Для сокетів
#include <netinet/in.h> // Для sockaddr_in
#include <arpa/inet.h> // Для inet_addr
#include <unistd.h> // Для close

// Інтегруємо клас ThreadPool з ДЗ3
class ThreadPool {
private:
    std::vector<std::thread> work_threads;
    std::queue<std::function<void()>> tasks;
    std::mutex q_mutex;
    std::condition_variable condition;
    bool stop;

public:
    ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; i++) {
            work_threads.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->q_mutex);
                        this->condition.wait(lock, [this]() {
                            return this->stop || !this->tasks.empty();
                        });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template <class F>
    void enqueue(F&& f) {
        {
            std::lock_guard<std::mutex> lock(q_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(q_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& work_thread : work_threads) work_thread.join();
    }
};

// Функція для множення рядка матриці
void multiplyRow(int i, int N, int M, int L, const std::vector<std::vector<int>>& matrix1, const std::vector<std::vector<int>>& matrix2, std::vector<std::vector<int>>& result) 
{
    for (int j = 0; j < L; ++j) 
    {
        result[i][j] = 0;
        for (int k = 0; k < M; ++k) 
        {
            result[i][j] += matrix1[i][k] * matrix2[k][j];
        }
    }
}

// Функція для обробки запиту клієнта
void handleClient(int clientSocket) 
{
    int N, M, L;

    // Виведення повідомлення про підключення клієнта
    struct sockaddr_in clientAddr;
    socklen_t addr_len = sizeof(clientAddr);
    getpeername(clientSocket, (struct sockaddr*)&clientAddr, &addr_len);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), ip, INET_ADDRSTRLEN);
    std::cout << "Connection from: " << ip << std::endl;

    // Приймаємо розміри та елементи першої матриці
    recv(clientSocket, (char*)&N, sizeof(N), 0);
    recv(clientSocket, (char*)&M, sizeof(M), 0);

    std::vector<std::vector<int>> matrix1(N, std::vector<int>(M));
    for (int i = 0; i < N; ++i) 
    {
        for (int j = 0; j < M; ++j) 
        {
            recv(clientSocket, (char*)&matrix1[i][j], sizeof(matrix1[i][j]), 0);
        }
    }

    // Приймаємо розміри та елементи другої матриці
    recv(clientSocket, (char*)&M, sizeof(M), 0);
    recv(clientSocket, (char*)&L, sizeof(L), 0);

    std::vector<std::vector<int>> matrix2(M, std::vector<int>(L));
    for (int i = 0; i < M; ++i) 
    {
        for (int j = 0; j < L; ++j) 
        {
            recv(clientSocket, (char*)&matrix2[i][j], sizeof(matrix2[i][j]), 0);
        }
    }

    // Перевірка на відповідність розмірів для множення
    if (matrix1[0].size() != matrix2.size()) 
    {
        const char* errorMessage = "Matrix dimensions do not match for multiplication!";
        send(clientSocket, errorMessage, strlen(errorMessage), 0);
        close(clientSocket);
        return;
    }

    // Множення матриць
    std::vector<std::vector<int>> result(N, std::vector<int>(L));

    // Створення ThreadPool для обробки запитів
    ThreadPool pool(4);  // 4 потоки для обробки завдань

    // Кожен рядок обчислюється окремо
    for (int i = 0; i < N; ++i) 
    {
        pool.enqueue([i, N, M, L, &matrix1, &matrix2, &result]() 
        {
            multiplyRow(i, N, M, L, matrix1, matrix2, result);
        });
    }

    // Відправлення результату клієнту
    for (int i = 0; i < N; ++i) 
    {
        for (int j = 0; j < L; ++j) 
        {
            send(clientSocket, (char*)&result[i][j], sizeof(result[i][j]), 0);
        }
    }

    close(clientSocket); // Закриваємо з'єднання
}

int main()
{
    // Заповнюємо інформацію про адресу сокета
    struct sockaddr_in addr; // Структура для зберігання адреси 
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // "127.0.0.1" - localhost
    addr.sin_port = htons(1111); 
    addr.sin_family = AF_INET; 

    int sListen = socket(AF_INET, SOCK_STREAM, 0); // Створюємо сокет
    bind(sListen, (struct sockaddr*)&addr, sizeof(addr)); // Прив'язуємо адресу сокета
    listen(sListen, SOMAXCONN); // Прослуховуємо порт в очікуванні з'єднання зі сторони клієнту

    std::cout << "The server is ready to receive clients...\n";

    // Прийом клієнтських з'єднань
    int clientSocket;

    while (true) {
        clientSocket = accept(sListen, (struct sockaddr*)&addr, (socklen_t*)&sizeofaddr); 

        if (clientSocket < 0) 
        {
            std::cerr << "Error: accept failed." << std::endl;
            continue;
        }

        // Обробка клієнта в окремому потоці
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach(); // Запускаємо потік, щоб обробляти клієнтів одночасно
    }

    close(sListen); // Закриваємо серверний сокет

    return 0;
}