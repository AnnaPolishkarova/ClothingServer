#include "../../include/network/NetworkManager.h"
#include "../../include/network/HttpSession.h"
#include <boost/asio.hpp>
#include <iostream>
#include "../../include/RequestHandler.h"
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

namespace beast = boost::beast;
using tcp = boost::asio::ip::tcp;

NetworkManager::NetworkManager(boost::asio::io_context& ioc,
                             unsigned short http_port,
                             Database& db)
    : ioc_(ioc),
      http_port_(http_port),
      db_(db),
      http_acceptor_(ioc_) {}

void NetworkManager::start() {
    start_http();
    {
        boost::lock_guard<boost::mutex> lock(RequestHandler::cout_mutex); // Инфо о запуске
        std::cout << "Network manager started (HTTP: " << http_port_ << ")\n";
        std::cout.flush();
    }
}

void NetworkManager::start_http() {
    try {
        tcp::endpoint endpoint{tcp::v4(), http_port_};
        http_acceptor_.open(endpoint.protocol());
        http_acceptor_.set_option(tcp::acceptor::reuse_address(true));
        http_acceptor_.bind(endpoint);
        http_acceptor_.listen();

        do_http_accept(); // Прием соединений
    } catch (const std::exception& e) {
        std::cerr << "HTTP server error: " << e.what() << "\n";
    }
}

void NetworkManager::do_http_accept() {
    http_acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<HttpSession>( // сессия для нового подключения
                    beast::tcp_stream(std::move(socket)), db_
                )->run();
            }
            do_http_accept(); // следующее соединение (рекурсивный вызов)
        });
}
