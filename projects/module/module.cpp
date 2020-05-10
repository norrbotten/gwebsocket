
#include <sol/sol.hpp>

#include "websocket-client.hpp"

websocket_client_manager ws_client_manager;

// TODO: handle exceptions
struct lua_wsclient {
    int connection_id;

    lua_wsclient(std::string uri)
        : connection_id(ws_client_manager.new_websocket(uri)) {
    }

    sol::object next_event(sol::this_state s) {
        sol::state_view lua(s);

        auto socket = ws_client_manager.get_websocket(connection_id);
        auto evnt   = socket->next_event();

        if (!evnt)
            return sol::make_object(lua, sol::nil);

        // clang-format off
        return lua.create_table_with("size", evnt.value().size,
                                     "type", client_event_name(evnt.value().type),
                                     "payload", evnt.value().payload);
        // clang-format on
    }

    void send(const std::string& payload, sol::this_state s) {
        sol::state_view lua(s);

        auto socket = ws_client_manager.get_websocket(connection_id);
        socket->send(payload);
    }

    void send_binary(const std::string& payload, sol::this_state s) {
        sol::state_view lua(s);

        auto socket = ws_client_manager.get_websocket(connection_id);
        socket->send_binary(payload);
    }
};

DLLEXPORT int gmod13_open(lua_State* L) {
    Lunar::Loader::Initialize();

    sol::state_view lua(L);

    sol::table websocket = lua.create_table();

    sol::usertype<lua_wsclient> wsclient =
        lua.new_usertype<lua_wsclient>("client", sol::constructors<lua_wsclient(std::string)>());

    wsclient["next_event"] = &lua_wsclient::next_event;

    wsclient["send"]        = &lua_wsclient::send;
    wsclient["send_binary"] = &lua_wsclient::send_binary;

    websocket["client"] = wsclient;

    lua["gwebsocket"] = websocket;

    return 0;
}

DLLEXPORT int gmod13_close(lua_State*) {
    Lunar::Loader::Deinitialize();

    ws_client_manager.~websocket_client_manager();

    return 0;
}