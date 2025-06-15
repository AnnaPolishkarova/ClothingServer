#include <gtest/gtest.h>
#include <sstream>
#include <boost/asio/thread_pool.hpp>
#include "../include/RequestHandler.h"

class RequestHandlerTest : public ::testing::Test
{
protected:
    Database db;
    boost::asio::thread_pool pool{4};
    RequestHandler handler{db, pool};
};

// Проверка вывода при отсутствии совпадений
TEST_F(RequestHandlerTest, HandlesNoMatches)
{
    std::stringstream buffer;
    auto old_buf = std::cout.rdbuf(buffer.rdbuf());

    handler.handle_request(700000, 68, 90).wait();
    std::cout.rdbuf(old_buf);

    EXPECT_NE(buffer.str().find("No matches found"), std::string::npos);
}

// Проверка вывода при успешном поиске
TEST_F(RequestHandlerTest, HandlesSuccessfulMatch)
{
    std::stringstream buffer;
    auto old_buf = std::cout.rdbuf(buffer.rdbuf());

    handler.handle_request(70, 95, 60).wait();
    std::cout.rdbuf(old_buf);

    EXPECT_NE(buffer.str().find("Skirt A"), std::string::npos);
    EXPECT_NE(buffer.str().find("100%"), std::string::npos);
}
