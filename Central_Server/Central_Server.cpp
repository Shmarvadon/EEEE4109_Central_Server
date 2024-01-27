#include <iostream>

#include "Server.hpp"

int main()
{
    std::cout << "EEEE4109 Central Control Server.\n";

    // Initialise WinSoc
    WSADATA wsaData;
    
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSA startup failed. Control server can not start.\n";
        return 1;
    }

    server controlServer = server();
}
