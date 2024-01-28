#include <iostream>
#include <chrono>

#include "Server.hpp"

int main()
{
    std::cout << "EEEE4109 Central Control Server.\n";

    // Initialise WinSoc.
    WSADATA wsaData;
    
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSA startup failed. Control server can not start.\n";
        return 1;
    }

    // Run the server.
    server controlServer;

    // Test that I can interface with the independent threads data correctly and call methods independent of its executing main function. (Yes, I can).

    while (controlServer.numberOfPoles() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "Sending some random data now.\n";
    controlServer.getPole(0).sendData((std::string)"Sus");

}
