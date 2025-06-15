#ifndef SERVER_H
#define SERVER_H

// #include <boost/thread.hpp>
 #include <boost/asio.hpp>
//#include "C:/vcpkg/installed/x64-windows/include/boost/asio.hpp"
#include "Database.h"
#include "RequestHandler.h"
#include "network/NetworkManager.h"
#include <thread>

// Основной класс сервера
class Server {
public:
    Server();
    ~Server();
    void start();
    void stop();

private:
    void run_console();
    void handle_add_command();
    void handle_edit_command();
    void handle_delete_command();
    void handle_user_command();
    boost::asio::io_context ioc_; // Контекст ввода-вывода
    Database database_; // База данных
    RequestHandler handler_; // Обработчик запросов
    NetworkManager network_manager_; // Управление сетевыми соединениями
    boost::asio::thread_pool pool_; // Пул потоков для обработки запросов
    std::thread network_thread_; // Поток для сетевого интерфейса
    bool running_ = false; // Флаг работы сервера
};

#endif //SERVER_H
