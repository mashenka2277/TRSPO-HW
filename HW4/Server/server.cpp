#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <winsock2.h> // Ліба для використання WinSock2

int main()
{
    // Завантажуємо необхідну версію бібліотеки
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1); 
    // Перевірка: чи завантажилась бібліотека 
    if(WSAStartup(DLLVersion, &wsaData) != 0)
    {
        std::cout << "Error: failed to initialize WinSock." << std::endl;
    }

    // Заповнюємо інформацію про адресу сокета
    SOCKADDR_IN addr; // Структура для зберігання адреси 
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // "127.0.0.1" - localhost
    addr.sin_port = htons(1111); 
    addr.sin_family = AF_INET; 

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, 0); // Створюємо сокет
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); // Прив'язуємо адресу сокета
    listen(sListen, SOMAXCONN); // Прослуховуємо порт в очікуванні з'єднання зі сторони клієнту

    std::cout << "The server is ready to receive clients...\n";

    // Новий сокет для утримання зв'язку з клієнтом
    SOCKET newConnection;
    newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr); 

    // Перевірка на встановлення з'єднання з користувачем 
    if (newConnection == 0) 
    {
        std::cout << "Error: failed to accept client." << std::endl;
    }
    else
    {
        std::cout << "Client connected." << std::endl;
    }

    system("pause");

    return 0;
}