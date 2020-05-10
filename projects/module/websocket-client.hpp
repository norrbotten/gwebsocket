#pragma once

#include <ixwebsocket/IXWebSocket.h>

#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

enum class websocket_client_event_type {
    MESSAGE_TEXT,
    MESSAGE_BINARY,
    OPENED,
    CLOSED,
    ERROR,
};

auto client_event_name = [](websocket_client_event_type type) -> std::string {
    switch (type) {
    case websocket_client_event_type::MESSAGE_TEXT:
        return "message_text";

    case websocket_client_event_type::MESSAGE_BINARY:
        return "message_bin";

    case websocket_client_event_type::OPENED:
        return "opened";

    case websocket_client_event_type::CLOSED:
        return "closed";

    case websocket_client_event_type::ERROR:
        return "error";
    }

    return "";
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

    std::string m_status;

    ix::WebSocketHttpHeaders m_headers;

    std::mutex m_mtx;

    std::queue<event> m_events;

  public:
    websocket_client(const std::string& uri)
        : m_uri(uri)
        , m_socket(ix::WebSocket()) {

        this->setup(uri);
    }

    websocket_client(const std::string& uri, const ix::WebSocketHttpHeaders& headers)
        : m_uri(uri)
        , m_socket(ix::WebSocket()) {

        m_headers = headers;
        this->setup(uri);
    }

    ~websocket_client() {
        m_socket.stop();
    }

  private:
    void setup(const std::string& uri) {
        m_socket.setUrl(uri);
        m_socket.setPingInterval(30);
        m_socket.setExtraHeaders(m_headers);

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
                m_status = "open";
                this->push_event(msg->wireSize, event_type::ERROR, fmt_open(msg->openInfo));
                break;

            case ix::WebSocketMessageType::Close:
                m_status = "closed";
                this->push_event(msg->wireSize, event_type::CLOSED, fmt_close(msg->closeInfo));
                break;

            case ix::WebSocketMessageType::Error:
                m_status = "errored";
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

        this->connect();
    }

    void push_event(size_t size, event_type type, const std::string& payload) {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_events.push(event{size, type, std::string(payload)});
    }

  public:
    void connect() {
        m_status = "connecting";
        m_socket.start();
    }

    void close() {
        m_status = "closed";
        m_socket.stop();
    }

    void send(const std::string& payload) {
        m_socket.sendText(payload);
    }

    void send_binary(const std::string& payload) {
        m_socket.sendBinary(payload);
    }

    void set_ping_interval(int interval) {
        m_socket.setPingInterval(interval);
    }

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

typedef std::shared_ptr<websocket_client> websocket_client_ptr;

class websocket_client_manager {
    size_t m_next_index;

    std::unordered_map<size_t, websocket_client_ptr> m_sockets;

  public:
    websocket_client_manager()
        : m_next_index(0) {
    }

    ~websocket_client_manager() {
        for (auto& socket : m_sockets) {
            socket.second->close();
        }
    }

    size_t new_websocket(const std::string& uri) {
        m_sockets[m_next_index++] = websocket_client_ptr(new websocket_client(uri));
        return m_next_index - 1;
    }

    websocket_client_ptr get_websocket(const size_t id) {
        auto found_it = m_sockets.find(id);

        if (found_it == m_sockets.end())
            throw "Internal GWebSocket Error: Invalid websocket ID";

        return (*found_it).second;
    }
};
