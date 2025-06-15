 // #include <gtest/gtest.h>
 // #include <thread>
 // #include <boost/asio.hpp>
 // #include "../include/Server.h"
 //
 // class ServerTest : public ::testing::Test {
 // protected:
 //     Server server;
 //     std::thread server_thread;
 //
 //     void SetUp() override {
 //         server_thread = std::thread([this]() { server.start(); });
 //         std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Ждем инициализации
 //     }
 //
 //     void TearDown() override {
 //         server.stop();
 //         if (server_thread.joinable()) server_thread.join();
 //     }
 // };
 //
 // // Проверка старта/остановки сервера
 // TEST_F(ServerTest, StartsAndStops) {
 //     EXPECT_TRUE(true); // Если SetUp/TearDown не упали - тест пройден
 // }
 //
 // // Тест HTTP API
 // TEST_F(ServerTest, HandlesHttpRequests) {
 //     boost::asio::io_context io;
 //     boost::asio::ip::tcp::socket socket(io);
 //     socket.connect(boost::asio::ip::tcp::endpoint(
 //         boost::asio::ip::make_address("127.0.0.1"), 8080));
 //
 //     std::string request =
 //         "GET /api/match?waist=70&hip=95&length=60 HTTP/1.1\r\n"
 //         "Host: localhost\r\n"
 //         "Accept: application/json\r\n"
 //         "Connection: close\r\n\r\n";
 //
 //     boost::asio::write(socket, boost::asio::buffer(request));
 //
 //     std::string response;
 //     boost::asio::read(socket, boost::asio::dynamic_buffer(response));
 //
 //     EXPECT_NE(response.find("Skirt A"), std::string::npos);
 //     EXPECT_NE(response.find("200 OK"), std::string::npos);
 // }