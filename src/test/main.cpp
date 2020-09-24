/*
#include <iostream>
#include <thread>

#include "../module/websocket-client.hpp"

int main() {
    websocket_client socket("wss://10.10.1.171:8080");

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        while (auto evnt = socket.next_event()) {
            std::cout << evnt.value().size << " bytes: " << evnt.value().payload << "\n";
        }
    }
}
*/

int main() {}