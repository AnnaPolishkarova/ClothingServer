#include "../include/Server.h"
#include <iostream>
//#include <boost/thread.hpp>
#include "C:/vcpkg/installed/x64-windows/include/boost/thread.hpp"

// Конструктор
Server::Server()
    : pool_(4),
    handler_(database_, pool_),
    network_manager_(ioc_, 8080, database_)
{
    // Небольшая задержка для инициализации
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

Server::~Server()
{
    stop();
}

void Server::start()
{
    running_ = true;
    network_thread_ = std::thread([this]() { // Запуск сетевого интерфейса (в отдельном потоке)
        try
        {
            {
                boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
                std::cout << "Starting network services...\n";
                std::cout.flush();
            }
            network_manager_.start(); // Старт HTTP/WebSocket серверов
            ioc_.run(); // Запуск обработки сетевых событий
        } catch (const std::exception& e)
        {
            boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
            std::cerr << "Network error: " << e.what() << "\n";
            std::cout.flush();
        }
    });
    // Дать время для запуска сетевых сервисов
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    run_console(); // Запуск консольного интерфейса (в главном потоке)
}

void Server::stop()
{
    if (!running_) return;
    running_ = false;
    ioc_.stop(); // Остановка сетевого интерфейса
    if (network_thread_.joinable()) network_thread_.join();
    pool_.stop(); // Остановка пула потоков
}

void Server::run_console()
{
    {
        boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
        std::cout << "\nEnter a command (add/edit/delete/user/exit): ";
        std::cout.flush();
    }
    while (running_)
    {
        try
        {
            std::string command;
            std::cin >> command;
            if (command == "exit") {
                stop();
                break;
            }
            if (command == "add") {
                handle_add_command();
            } else if (command == "edit") {
                handle_edit_command();
            } else if (command == "delete") {
                handle_delete_command();
            } else if (command == "user") {
                handle_user_command();
            } else {
                boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
                std::cout << "Unknown command.\n";
                std::cout.flush();
            }
            if (running_) {
                boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
                std::cout << "\nEnter a command (add/edit/delete/user/exit): ";
                std::cout.flush();
            }
        } catch (const std::exception& e){
            boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
            std::cerr << "Console error: " << e.what() << "\n";
        }
    }
}

void Server::handle_add_command() {
    double waist, hip, length;
    std::string name;

    {
        boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
        std::cout << "Enter item details (waist, hips, length, name): ";
        std::cout.flush();
    }
    std::cin >> waist >> hip >> length >> name;

    database_.add_item(ClothingItem(waist, hip, length, name));

    boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
    std::cout << "Item added successfully\n";
}

void Server::handle_edit_command() {
    std::string name;
    double waist, hip, length;
    std::string new_name;

    {
        boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
        std::cout << "Enter item name to edit: ";
        std::cout.flush();
    }
    std::cin >> name;

    {
        boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
        std::cout << "Enter new waist hip length name: ";
        std::cout.flush();
    }
    std::cin >> waist >> hip >> length >> new_name;

    ClothingItem new_item(waist, hip, length, new_name);
    database_.edit_item(name, new_item);

    boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
    std::cout << "Item edited successfully\n";
}

void Server::handle_delete_command() {
    std::string name;

    {
        boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
        std::cout << "Enter item name to delete: ";
        std::cout.flush();
    }
    std::cin >> name;

    database_.delete_item(name);

    boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
    std::cout << "Item deleted successfully\n";
}

void Server::handle_user_command() {
    double waist, hip, length;

    {
        boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex);
        std::cout << "Enter user measurements (waist hip length): ";
        std::cout.flush();
    }
    std::cin >> waist >> hip >> length;

    auto future = handler_.handle_request(waist, hip, length);
    future.wait();  // Ожидаем завершения обработки
}
