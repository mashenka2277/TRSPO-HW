#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include <winsock2.h> // Ліба для використання WinSock2
#include <cstdint>

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

    // Обмін повідомленнями
    char buffer[1024];
    while (true)
    {
        // Прийом повідомлення від клієнта (за бінарним протоколом)

        // Прийом довжини повідомлення
        uint32_t msg_length;
        int read_len = recv(newConnection, (char*)&msg_length, sizeof(msg_length), 0);
        if (read_len <= 0)
        {
            std::cout << "The client has disconnected.\n";
            break;
        }

        // Прийом самого повідомлення
        int read_msg = recv(newConnection, buffer, msg_length, 0);

        if (read_msg <= 0)
        {
            std::cout << "The client has disconnected.\n";
            break;
        }

        buffer[read_msg] = '\0'; // Завершення рядка
        std::cout << "Client: " << buffer << std::endl;

        // Відправка відповіді клієнту
        std::string ans;
        std::cout << "Your message to Client: ";
        std::getline(std::cin, ans);

        if (ans.empty())  
        {
            std::cout << "Exiting...\n";
            closesocket(newConnection);  // Сервер закриває сокет з клієнтом
        }

        // Відправка довжини повідомлення
        uint32_t ans_len = ans.size();
        send(newConnection, (char*)&ans_len, sizeof(ans_len), 0);

        send(newConnection, ans.c_str(), ans.size(), 0); // Відправка самого повідомлення
    }

    // Закриття сокетів
    closesocket(sListen);
    WSACleanup();

    return 0;
}