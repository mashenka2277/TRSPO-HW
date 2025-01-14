#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include <winsock2.h> // Ліба для використання WinSock2
#include <cstdint>
#include <vector>
#include <cstdlib>

// Функція для генерації матриці
void generateMatrix(int rows, int cols, std::vector<std::vector<int>>& matrix)
{
    matrix.resize(rows, std::vector<int>(cols));
    for (int i = 0; i < rows; ++i) 
    {
        for (int j = 0; j < cols; ++j) 
        {
            matrix[i][j] = rand() % 100; // Випадкове число від 0 до 99
        }
    }
}

// Функція для передачі матриці серверу
void sendMatrix(SOCKET& socket, const std::vector<std::vector<int>>& matrix)
{
    int rows = matrix.size();
    int cols = matrix[0].size();

    // Відправляємо розміри матриці
    send(socket, (char*)&rows, sizeof(rows), 0);
    send(socket, (char*)&cols, sizeof(cols), 0);

    // Відправляємо елементи матриці
    for (int i = 0; i < rows; ++i) 
    {
        for (int j = 0; j < cols; ++j) 
        {
            send(socket, (char*)&matrix[i][j], sizeof(matrix[i][j]), 0);
        }
    }
}

int main()
{
    // Завантажуємо необхідну версію бібліотеки
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1); 
    // Перевірка: чи завантажилась бібліотека 
    if(WSAStartup(DLLVersion, &wsaData) != 0)
    {
        std::cout << "Error: failed to initialize WinSock." << std::endl;
        return 1;
    }

    // Заповнюємо інформацію про адресу сокета
    SOCKADDR_IN addr; // Структура для зберігання адреси 
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // "127.0.0.1" - localhost
    addr.sin_port = htons(1111); 
    addr.sin_family = AF_INET; 

    SOCKET Connection = socket(AF_INET, SOCK_STREAM, 0); // Створюємо новий сокет для з'єднання з сервером

    // Спроба під'єднання до серверу
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
    {
        std::cout << "Error: failed connect to server.\n";
        return 1;
    } 
    std::cout << "Connected to server." << std::endl;

    // Генерація двох матриць
    int N = rand() % 10 + 1000; // Випадкові розміри
    int M = rand() % 10 + 1000;
    int L = rand() % 10 + 1000;

    std::vector<std::vector<int>> matrix1, matrix2;

    generateMatrix(N, M, matrix1);
    generateMatrix(M, L, matrix2);

    // Відправка матриць на сервер
    sendMatrix(Connection, matrix1);
    sendMatrix(Connection, matrix2);

    // Отримуємо результат
    char buffer[1024];
    int read_len = recv(Connection, buffer, sizeof(buffer), 0);
    if (read_len > 0) 
    {
        std::cout << "Result received from server." << std::endl;
    }

    WSACleanup();

    return 0;
}