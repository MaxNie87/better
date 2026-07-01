#pragma once

#include <asio.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "core/media_hub.h"
#include "gb28181/gb28181_server.h"
#include "net/tcp_server.h"
#include "webrtc/whep_server.h"

namespace csk {

struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;

    std::string path_param(const std::string &prefix) const;
};

struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::map<std::string, std::string> headers;
    std::string body;

    void set_json(const std::string &json);
    void set_status(int code, const std::string &text = "");
    std::string serialize() const;
};

class HttpSession : public Session {
public:
    using RequestHandler = std::function<void(const HttpRequest &, HttpResponse &)>;

    HttpSession(asio::ip::tcp::socket socket, RequestHandler handler);

    void start() override;
    void on_data(SpanU8 data) override;

private:
    void process_request();

    RequestHandler handler_;
    std::string buffer_;
};

class ApiServer {
public:
    ApiServer(asio::io_context &io, uint16_t port, MediaHub &hub,
              std::shared_ptr<WhepServer> whep = nullptr,
              std::shared_ptr<Gb28181Server> gb28181 = nullptr);

    void start();
    void stop();

private:
    void handle_request(const HttpRequest &req, HttpResponse &resp);

    void handle_list_streams(const HttpRequest &req, HttpResponse &resp);
    void handle_add_stream(const HttpRequest &req, HttpResponse &resp);
    void handle_get_stream(const HttpRequest &req, HttpResponse &resp);
    void handle_delete_stream(const HttpRequest &req, HttpResponse &resp);
    void handle_metrics(const HttpRequest &req, HttpResponse &resp);
    void handle_whep_offer(const HttpRequest &req, HttpResponse &resp);
    void handle_whep_delete(const HttpRequest &req, HttpResponse &resp);

    void handle_gb28181_devices(const HttpRequest &req, HttpResponse &resp);
    void handle_gb28181_invite(const HttpRequest &req, HttpResponse &resp);
    void handle_gb28181_bye(const HttpRequest &req, HttpResponse &resp);

    std::unique_ptr<TcpServer> tcp_server_;
    MediaHub &hub_;
    asio::io_context &io_;
    std::shared_ptr<WhepServer> whep_;
    std::shared_ptr<Gb28181Server> gb28181_;
};

}  // namespace csk
