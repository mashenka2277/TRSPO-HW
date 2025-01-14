#include <iostream>
#include <string>
#include <sys/socket.h> // Для сокетів
#include <netinet/in.h> // Для sockaddr_in
#include <arpa/inet.h> // Для inet_addr
#include <unistd.h> // Для close

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

    // Новий сокет для утримання зв'язку з клієнтом
    int newConnection;
    newConnection = accept(sListen, (struct sockaddr*)&addr, (socklen_t*)&sizeofaddr); 

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
    int message_counter = 0;  // Лічильник отриманих повідомлень
    for (int i = 0; i < 100; ++i) 
    { // Прийом 100 повідомлень
        
        // Прийом текстового повідомлення від клієнта
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

        // Відправка лічильника отриманих повідомлень у відповідь
        message_counter++;

        // Відправка довжини повідомлення
        uint32_t ans_len = sizeof(message_counter);
        send(newConnection, (char*)&ans_len, sizeof(ans_len), 0);

        send(newConnection, (char*)&message_counter, sizeof(message_counter), 0); // Відправка самого повідомлення
    
    }

    // Закриття сокетів
    close(sListen);
    close(newConnection);

    return 0;
}