/*
    Note: to test this on an empty server, set sv_hibernate_think to 1
    for the Tick hook to be called
*/

require("gwebsocket")

local listener = coroutine.wrap(function()

    local socket = gwebsocket.client.new("ws://10.10.1.171:8080")

    print("connecting")

    coroutine.wait(0.1) -- give it a moment to connect

    while true do
        local event = socket:next_event()

        if event != nil then
            print("got message")
            print("size:", event.size)
            print("type:", event.type)
            print("payload:", event.payload)
            print()
        end

        coroutine.yield()
    end

end)

hook.Add("Tick", "listener_dispatcher", listener)