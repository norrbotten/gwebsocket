#pragma once

#include <ixwebsocket/IXWebSocket.h>

#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace GWebSocket {

    class Client {
    public:
        Client(const std::string& url)
            : m_socket() {

            m_socket.setUrl(url);
            m_socket.setPingInterval(45);

            m_socket.enableAutomaticReconnection();
            m_socket.setMaxWaitBetweenReconnectionRetries(5);

            m_socket.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg_ptr) {
                std::lock_guard g(m_messages_mtx);
                printf("[c++] message callback\n");

                if (msg_ptr->type == ix::WebSocketMessageType::Message ||
                    msg_ptr->type == ix::WebSocketMessageType::Fragment) {
                    m_messages.push_back(
                        {std::string(msg_ptr->str), ix::WebSocketMessage(*msg_ptr)});
                }
                else {
                    m_messages.push_back({"", ix::WebSocketMessage(*msg_ptr)});
                }
            });
        }

        bool set_header(const std::string& key, const std::string& value) {
            if (m_started)
                return true;

            m_headers[key] = value;

            return false;
        }

        bool connect() {
            if (m_started)
                return true;

            m_socket.setExtraHeaders(m_headers);

            m_started = true;
            m_socket.start();

            return false;
        }

        bool close() {
            if (!m_started)
                return true;

            m_closed = true;
            m_socket.stop();

            return false;
        }

        bool started() { return m_started; }
        bool closed() { return m_closed; }

        auto& socket() { return m_socket; }

        std::optional<std::pair<std::string, ix::WebSocketMessage>> next_message() {
            std::lock_guard g(m_messages_mtx);

            if (m_messages.empty())
                return std::nullopt;

            auto msg = m_messages.front();
            m_messages.pop_front();
            return msg;
        }

        auto num_messages() const { return m_messages.size(); }

    private:
        ix::WebSocket            m_socket;
        ix::WebSocketHttpHeaders m_headers;

        std::deque<std::pair<std::string, ix::WebSocketMessage>> m_messages;
        std::mutex                                               m_messages_mtx;

        bool m_started = false;
        bool m_closed  = false;
    };

    class ClientPool {
    public:
        ~ClientPool() {
            for (auto& [index, client] : m_clients)
                client->close();
        }

        int spawn(const std::string& uri) {
            auto client               = new Client(uri);
            m_clients[m_next_index++] = client;
            return m_next_index - 1;
        }

        void kill(int id) {
            if (m_clients.contains(id)) {
                delete m_clients.at(id);
                m_clients.erase(id);
            }
        }

        Client* get(int id) {
            if (m_clients.contains(id)) {
                return m_clients.at(id);
            }

            return nullptr;
        }

    private:
        int                              m_next_index = 0;
        std::unordered_map<int, Client*> m_clients;
    };

} // namespace GWebSocket
