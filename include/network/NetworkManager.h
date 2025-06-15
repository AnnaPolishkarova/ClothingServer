#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <boost/asio.hpp>
#include <memory>
#include "network/HttpSession.h"

class NetworkManager {
public:
    NetworkManager(boost::asio::io_context& ioc,
        unsigned short http_port,
        Database& db);
    void start();

private:
    void start_http();  // Запуск HTTP сервера
    void do_http_accept();  // Прием HTTP соединений

    boost::asio::io_context& ioc_;
    unsigned short http_port_;
    Database& db_;
    boost::asio::ip::tcp::acceptor http_acceptor_;
};

#endif //NETWORKMANAGER_H
