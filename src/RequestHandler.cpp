#include "../include/RequestHandler.h"
#include <iostream>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/asio/post.hpp>

// Инициализация статического мьютекса
boost::mutex RequestHandler::cin_mtx;
boost::mutex RequestHandler::cout_mutex;

// Конструктор
RequestHandler::RequestHandler(Database& db, boost::asio::thread_pool& pool): database(db), pool(pool) {}

// Обработка запроса
std::future<void> RequestHandler::handle_request(double user_waist, double user_hip, double user_length) {
    auto promise = std::make_shared<std::promise<void>>();
    std::future<void> future = promise->get_future();
    // Лямбда-функция для выполнения в пуле потоков
    auto task = [this, user_waist, user_hip, user_length, promise]()  {
        // Поиск подходящих предметов одежды
        auto results = database.find_matches(user_waist, user_hip, user_length);

        // Синхронизация вывода
        {
            boost::lock_guard<boost::mutex> lock(cout_mutex);
            std::cout << "Found " << results.size() << " matches for user (" << user_waist << ", " << user_hip << ", " << user_length << ")\n";
            std::cout << "Results:\n";
            if (results.empty()) {
                std::cout << "No matches found.\n";
            } else {
                for (const auto& result : results) {
                    std::cout << " - " << result.first << ": " << result.second * 100 << "%\n";
                }
            }
            std::cout.flush();
        }
        promise->set_value(); // Уведомление о завершении задачи
    };
    boost::asio::post(pool, task); // Отправка задачи в пул потоков
    return future;
}