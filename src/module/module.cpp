#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "ws_client.hpp"

template <typename... T>
void call_lua_error(sol::state_view s, T... args) {
    sol::function err = s.globals()["error"];
    err(args...);
}

static GWebSocket::ClientPool g_client_pool;

struct ILuaClient {
    int         id;
    std::string uri;

    ILuaClient(std::string u)
        : id(g_client_pool.spawn(u))
        , uri(u) {}

    sol::object next_event(sol::this_state s) {
        sol::state_view lua(s);

        auto sock = g_client_pool.get(id);
        if (sock == nullptr || (sock != nullptr && sock->closed())) {
            call_lua_error(lua, "gwebsocket: socket is dead\n");
            return sol::nil;
        }

        if (auto msg = sock->next_message(); msg.has_value()) {
            auto& val = msg.value();

            // clang-format off
            switch (val.second.type) {
            case ix::WebSocketMessageType::Message:
                return lua.create_table_with("size", val.second.wireSize,
                                             "type", "message",
                                             "is_binary", val.second.binary,
                                             "payload", val.first);

            case ix::WebSocketMessageType::Fragment:
                return lua.create_table_with("size", val.second.wireSize,
                                             "type", "message_fragment",
                                             "is_binary", val.second.binary,
                                             "payload", val.first);

            case ix::WebSocketMessageType::Open:
                return lua.create_table_with("type", "opened");

            case ix::WebSocketMessageType::Close:
                return lua.create_table_with("type", "closed");

            case ix::WebSocketMessageType::Error:
                return lua.create_table_with("type", "error",
                                             "reason", val.second.errorInfo.reason,
                                             "status", val.second.errorInfo.http_status);

            case ix::WebSocketMessageType::Ping: break;
            case ix::WebSocketMessageType::Pong: break;
            }
            // clang-format on
        }

        return sol::nil;
    }

    sol::object num_events(sol::this_state s) {
        sol::state_view lua(s);

        auto sock = g_client_pool.get(id);
        if (sock == nullptr || (sock != nullptr && sock->closed())) {
            call_lua_error(lua, "gwebsocket: socket is dead");
            return sol::nil;
        }

        return sol::make_object(lua, sock->num_messages());
    }

    void send(const std::string& payload, sol::this_state s) {
        sol::state_view lua(s);

        auto sock = g_client_pool.get(id);
        if (sock == nullptr || (sock != nullptr && sock->closed())) {
            call_lua_error(lua, "gwebsocket: socket is dead");
            return;
        }

        sock->socket().send(payload);
    }

    void send_binary(const std::string& payload, sol::this_state s) {
        sol::state_view lua(s);

        auto sock = g_client_pool.get(id);
        if (sock == nullptr || (sock != nullptr && sock->closed())) {
            call_lua_error(lua, "gwebsocket: socket is dead");
            return;
        }

        sock->socket().send(payload, true);
    }

    void connect(sol::this_state s) {
        sol::state_view lua(s);

        auto sock = g_client_pool.get(id);
        if (sock == nullptr || (sock != nullptr && sock->closed())) {
            call_lua_error(lua, "gwebsocket: socket is dead");
            return;
        }

        if (sock->connect()) {
            call_lua_error(lua, "gwebsocket: socket already started");
            return;
        }
    }

    void reconnect(sol::this_state s) {
        sol::state_view lua(s);

        auto sock = g_client_pool.get(id);
        if (sock != nullptr && !sock->closed()) {
            call_lua_error(lua, "gwebsocket: socket already connected");
            return;
        }

        id            = g_client_pool.spawn(uri);
        auto new_sock = g_client_pool.get(id);

        new_sock->connect();
    }

    void close(sol::this_state s) {
        sol::state_view lua(s);

        auto sock = g_client_pool.get(id);
        if (sock == nullptr || (sock != nullptr && sock->closed())) {
            call_lua_error(lua, "gwebsocket: socket is dead");
            return;
        }

        sock->close();
    }

    sol::object state(sol::this_state s) {
        sol::state_view lua(s);

        auto sock = g_client_pool.get(id);
        if (sock == nullptr || (sock != nullptr && sock->closed())) {
            call_lua_error(lua, "gwebsocket: socket is dead");
            return sol::nil;
        }

        if (!sock->started())
            return sol::make_object(lua, "offline");

        switch (sock->socket().getReadyState()) {
        case ix::ReadyState::Connecting: return sol::make_object(lua, "connecting");
        case ix::ReadyState::Open: return sol::make_object(lua, "open");
        case ix::ReadyState::Closing: return sol::make_object(lua, "closing");
        case ix::ReadyState::Closed: return sol::make_object(lua, "closed");
        }

        return sol::nil;
    }

    void set_header(sol::this_state s, const std::string& key, const std::string& value) {
        sol::state_view lua(s);

        auto sock = g_client_pool.get(id);
        if (sock == nullptr || (sock != nullptr && sock->closed())) {
            call_lua_error(lua, "gwebsocket: socket is dead");
            return;
        }

        sock->set_header(key, value);
    }
};

DLLEXPORT int gmod13_open(lua_State* L) {
    Lunar::Loader::Initialize();

    sol::state_view lua(L);

    sol::table websocket = lua.create_table();

    auto wsclient =
        lua.new_usertype<ILuaClient>("client", sol::constructors<ILuaClient(std::string)>());

    wsclient["connect"] = &ILuaClient::connect; // Connect the socket
    wsclient["close"] = &ILuaClient::close; // Disconnect the socket, warning: closing is syncronous
                                            // and may lock up your game for a bit

    wsclient["reconnect"] = &ILuaClient::reconnect; // Reconnects a dead socket

    wsclient["state"] = &ILuaClient::state; // Get its state

    wsclient["send"]        = &ILuaClient::send;        // Async send ascii data
    wsclient["send_binary"] = &ILuaClient::send_binary; // Async send binary data

    wsclient["next_event"] = &ILuaClient::next_event; // Get the next event in the queue

    wsclient["num_events"] = &ILuaClient::num_events; // Get number of events in the queue

    wsclient["set_header"] = &ILuaClient::set_header; // Set a header before connecting

    websocket["client"] = wsclient;

    lua["gwebsocket"] = websocket;

    return 0;
}

DLLEXPORT int gmod13_close(lua_State*) {
    Lunar::Loader::Deinitialize();

    g_client_pool.~ClientPool();

    return 0;
}