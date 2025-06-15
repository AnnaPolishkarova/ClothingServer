#ifndef HTTPSESSION_H
#define HTTPSESSION_H

#include <memory>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include "Database.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(beast::tcp_stream&& stream, Database& db);
    void run();

private:
    void do_read();
    void on_read(beast::error_code ec, size_t bytes);
    void handle_request(http::request<http::string_body>&& req);
    void handle_add_item(const nlohmann::json& data, http::response<http::string_body>& res);
    void handle_edit_item(const nlohmann::json& data, http::response<http::string_body>& res);
    void handle_delete_item(const std::string& name, http::response<http::string_body>& res);

    beast::tcp_stream stream_;
    Database& db_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
};

#endif //HTTPSESSION_H
