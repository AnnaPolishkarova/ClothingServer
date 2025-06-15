#include "../../include/network/HttpSession.h"

#include <Global.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

using json = nlohmann::json;

HttpSession::HttpSession(beast::tcp_stream&& stream, Database& db)
    : stream_(std::move(stream)), db_(db) {}

void HttpSession::run() { do_read(); }

void HttpSession::do_read() {
    http::async_read(stream_, buffer_, req_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred) {
                     self->on_read(ec, bytes_transferred);
        });
}

void HttpSession::on_read(beast::error_code ec, std::size_t) {
    if (ec) return;
    handle_request(std::move(req_));
}

void HttpSession::handle_request(http::request<http::string_body>&& req) {
    auto res = std::make_shared<http::response<http::string_body>>();
    res->set(http::field::content_type, "application/json");
    res->set(http::field::access_control_allow_origin, "*");

    try {
        // Логирование входящего запроса
        std::cout << "Received request: " << req.method_string() << " " << req.target() << std::endl;

        // Обработка GET /api/items
        if (req.method() == http::verb::get && req.target() == "/api/items") {
            auto items = db_.get_all_items();
            json j;
            j["status"] = "success";
            j["items"] = json::array();

            for (const auto& item : items) {
                j["items"].push_back({
                    {"name", item.name},
                    {"waist", item.waist_size},
                    {"hip", item.hip_size},
                    {"length", item.length}
                });
            }
            res->result(http::status::ok);
            res->body() = j.dump();
        }
        // Обработка GET /api/match
        else if (req.method() == http::verb::get && req.target().starts_with("/api/match")) {
            std::string target(req.target());
            size_t query_start = target.find('?');

            if (query_start == std::string::npos) {
                throw std::runtime_error("Missing query parameters");
            }

            std::string query = target.substr(query_start + 1);
            std::vector<std::string> params;
            boost::split(params, query, boost::is_any_of("&"));

            double waist = 0.0, hip = 0.0, length = 0.0;

            for (const auto& param : params) {
                size_t eq_pos = param.find('=');
                if (eq_pos == std::string::npos) continue;

                std::string key = param.substr(0, eq_pos);
                std::string value = param.substr(eq_pos + 1);

                try {
                    if (key == "waist") waist = std::stod(value);
                    else if (key == "hip") hip = std::stod(value);
                    else if (key == "length") length = std::stod(value);
                } catch (const std::exception&) {
                    throw std::runtime_error("Invalid parameter value for " + key);
                }
            }

            auto results = db_.find_matches(waist, hip, length);
            json j;

            if (results.empty()) {
                j["status"] = "success";
                j["matches"] = json::array();
            } else {
                j["status"] = "success";
                for (const auto& [name, prob] : results) {
                    j["matches"].push_back({
                        {"name", name},
                        {"probability", prob}
                    });
                }
            }

            res->result(http::status::ok);
            res->body() = j.dump();
        }
        // Обработка POST /api/items
        else if (req.method() == http::verb::post && req.target() == "/api/items") {
            json data = json::parse(req.body());

            if (!data.contains("name") || !data.contains("waist") ||
                !data.contains("hip") || !data.contains("length")) {
                throw std::runtime_error("Missing required fields");
            }

            ClothingItem item{
                data["waist"].get<double>(),
                data["hip"].get<double>(),
                data["length"].get<double>(),
                data["name"].get<std::string>()
            };

            db_.add_item(item);

            res->result(http::status::created);
            res->body() = json{
                {"status", "success"},
                {"message", "Item added successfully"},
                {"item", {
                    {"name", item.name},
                    {"waist", item.waist_size},
                    {"hip", item.hip_size},
                    {"length", item.length}
                }}
            }.dump();
        }
        // Обработка PUT /api/items
        else if (req.method() == http::verb::put && req.target() == "/api/items") {
            json data = json::parse(req.body());

            if (!data.contains("old_name") || !data.contains("name") ||
                !data.contains("waist") || !data.contains("hip") || !data.contains("length")) {
                throw std::runtime_error("Missing required fields");
            }

            ClothingItem new_item{
                data["waist"].get<double>(),
                data["hip"].get<double>(),
                data["length"].get<double>(),
                data["name"].get<std::string>()
            };

            db_.edit_item(data["old_name"].get<std::string>(), new_item);

            res->result(http::status::ok);
            res->body() = json{
                {"status", "success"},
                {"message", "Item updated successfully"},
                {"item", {
                    {"name", new_item.name},
                    {"waist", new_item.waist_size},
                    {"hip", new_item.hip_size},
                    {"length", new_item.length}
                }}
            }.dump();
        }
        // Обработка DELETE /api/items/{name}
        else if (req.method() == http::verb::delete_ && req.target().starts_with("/api/items/")) {
            std::string name = req.target().substr(strlen("/api/items/"));
            db_.delete_item(name);

            res->result(http::status::ok);
            res->body() = json{
                {"status", "success"},
                {"message", "Item deleted successfully"},
                {"deleted_item", name}
            }.dump();
        }
        // Неподдерживаемый эндпоинт
        else {
            res->result(http::status::not_found);
            res->body() = json{
                {"status", "error"},
                {"message", "Endpoint not found"}
            }.dump();
        }
    }
    catch (const json::parse_error& e) {
        res->result(http::status::bad_request);
        res->body() = json{
            {"status", "error"},
            {"message", "Invalid JSON format"},
            {"details", e.what()}
        }.dump();
    }
    catch (const std::exception& e) {
        res->result(http::status::internal_server_error);
        res->body() = json{
            {"status", "error"},
            {"message", e.what()}
        }.dump();
    }

    // Отправка ответа с гарантией времени жизни объектов
    auto self = shared_from_this();
    http::async_write(
        stream_,
        *res,
        [self, res](beast::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                std::cerr << "Write error: " << ec.message() << std::endl;
            }
            // После отправки ответа закрываем соединение
            self->stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
        });
}


void HttpSession::handle_add_item(const json& data, http::response<http::string_body>& res) {
    try {
        if (!data.contains("name") || !data.contains("waist") ||
            !data.contains("hip") || !data.contains("length")) {
            throw std::runtime_error("Missing required fields");
            }

        // Логирование получаемых данных
        std::cout << "Adding item: " << data.dump() << std::endl;

        ClothingItem item(
            data["waist"].get<double>(),
            data["hip"].get<double>(),
            data["length"].get<double>(),
            data["name"].get<std::string>()
        );

        db_.add_item(item);

        // Убедимся, что потокобезопасный вывод работает
        {
            boost::lock_guard<boost::mutex> lock(db_cout_mutex);
            std::cout << "Item " << item.name << " added to database" << std::endl;
        }

        res.result(http::status::created);
        res.body() = json{
                {"status", "success"},
                {"message", "Item added successfully"},
                {"item", {
                    {"name", item.name},
                    {"waist", item.waist_size},
                    {"hip", item.hip_size},
                    {"length", item.length}
                }}
        }.dump();
    }
    catch (const std::exception& e) {
        res.result(http::status::bad_request);
        res.body() = json{
                {"status", "error"},
                {"message", e.what()}
        }.dump();

        boost::lock_guard<boost::mutex> lock(db_cout_mutex);
        std::cerr << "Error adding item: " << e.what() << std::endl;
    }
}


void HttpSession::handle_edit_item(const json& data, http::response<http::string_body>& res) {
    try {
        // Проверяем наличие всех полей более безопасным способом
        if (!data.is_object() ||
            data.find("old_name") == data.end() ||
            data.find("waist") == data.end() ||
            data.find("hip") == data.end() ||
            data.find("length") == data.end() ||
            data.find("new_name") == data.end()) {
            throw std::runtime_error("Missing required fields");
            }

        ClothingItem new_item(
            data["waist"].get<double>(),
            data["hip"].get<double>(),
            data["length"].get<double>(),
            data["new_name"].get<std::string>()
        );

        db_.edit_item(data["old_name"].get<std::string>(), new_item);
        res.result(http::status::ok);
        res.body() = json{{"status", "success"}, {"message", "Item edited successfully"}}.dump();
    } catch (const std::exception& e) {
        res.result(http::status::bad_request);
        res.body() = json{{"status", "error"}, {"message", e.what()}}.dump();
    }
    http::async_write(stream_, res, [res](auto ec, auto) {});
}

void HttpSession::handle_delete_item(const std::string& name, http::response<http::string_body>& res) {
    db_.delete_item(name);
    res.result(http::status::ok);
    res.body() = json{{"status", "success"}, {"message", "Item deleted"}}.dump();
}
