#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <boost/thread/mutex.hpp>
#include <boost/asio/thread_pool.hpp>
#include "Database.h"
#include <future>

// Класс для обработки запросов
class RequestHandler {
public:
    RequestHandler(Database& db, boost::asio::thread_pool& pool);
    // void handle_request(double user_waist, double user_hip, double user_length);
    std::future<void> handle_request(double user_waist, double user_hip, double user_length);
    static boost::mutex cin_mtx; // Мьютекс для синхронизации ввода
    static boost::mutex cout_mutex; // Мьютекс для синхронизации вывода

private:
    Database& database; // Ссылка на базу данных
    boost::asio::thread_pool& pool; // Ссылка на пул потоков
};

#endif //REQUESTHANDLER_H
