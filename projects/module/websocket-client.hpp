#pragma once

#include <ixwebsocket/IXWebSocket.h>

#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

enum class websocket_client_event_type {
    MESSAGE_TEXT,
    MESSAGE_BINARY,
    OPENED,
    CLOSED,
    ERROR,
};

struct websocket_client_event {
    size_t                      size;
    websocket_client_event_type type;
    std::string                 payload;
};

class websocket_client {
    using event      = websocket_client_event;
    using event_type = websocket_client_event_type;

    std::string   m_uri;
    ix::WebSocket m_socket;

    std::mutex m_mtx;

    std::queue<event> m_events;

  public:
    websocket_client(const std::string& uri)
        : m_uri(uri)
        , m_socket(ix::WebSocket()) {

        m_socket.setUrl(uri);
        m_socket.setPingInterval(30);

        auto fmt_error = [](ix::WebSocketErrorInfo& errinfo) -> std::string {
            std::stringstream ss;
            ss << errinfo.http_status << " (" << errinfo.reason << ")";
            return ss.str();
        };

        auto fmt_close = [](ix::WebSocketCloseInfo& closeinfo) -> std::string {
            std::stringstream ss;
            ss << closeinfo.code << " (" << closeinfo.reason << ")";
            return ss.str();
        };

        auto fmt_open = [](ix::WebSocketOpenInfo& openinfo) -> std::string {
            std::stringstream ss;
            ss << openinfo.uri;
            return ss.str();
        };

        m_socket.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
            switch (msg->type) {
            case ix::WebSocketMessageType::Open:
                this->push_event(msg->wireSize, event_type::ERROR, fmt_open(msg->openInfo));
                break;

            case ix::WebSocketMessageType::Close:
                this->push_event(msg->wireSize, event_type::CLOSED, fmt_close(msg->closeInfo));
                break;

            case ix::WebSocketMessageType::Error:
                this->push_event(msg->wireSize, event_type::ERROR, fmt_error(msg->errorInfo));
                break;

            case ix::WebSocketMessageType::Message:
                this->push_event(
                    msg->wireSize,
                    msg->binary ? event_type::MESSAGE_BINARY : event_type::MESSAGE_TEXT, msg->str);
                break;

            default:
                break;
            }
        });

        m_socket.start();
    }

    ~websocket_client() {
        m_socket.stop();
    }

  private:
    void push_event(size_t size, event_type type, const std::string& payload) {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_events.push(event{size, type, std::string(payload)});
    }

  public:
    std::optional<event> next_event() {
        std::lock_guard<std::mutex> lock(m_mtx);

        if (m_events.empty())
            return {};

        auto evnt = m_events.front();
        m_events.pop();

        return evnt;
    }

    std::vector<event> all_events() {
        std::lock_guard<std::mutex> lock(m_mtx);

        std::vector<event> res;
        res.reserve(m_events.size());

        while (!m_events.empty()) {
            res.push_back(m_events.front());
            m_events.pop();
        }

        return res;
    }
};

class websocket_client_manager {};