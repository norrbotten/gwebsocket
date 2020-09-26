/*
    Note: to test this on an empty server, set sv_hibernate_think to 1
    for the Tick hook to be called
*/

require("gwebsocket")

local sock = gwebsocket.client.new("wss://echo.websocket.org")

sock:connect()

timer.Create("a", 0.5, 0, function()
    if sock:state() == "open" then
        sock:send("works?")
    end

    while sock:num_events() > 0 do
        local evnt = sock:next_event()
        -- evnt may still be nil here since not everything is covered yet
        print("received event: ", (evnt == nil) and "nil" or table.ToString(evnt))
    end
end)
