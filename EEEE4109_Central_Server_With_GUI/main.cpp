#include "mainwindow.h"

#include <QApplication>
#include <QtUiTools/QUiLoader>

#include <ws2tcpip.h>
#include <WinSock2.h>


int main(int argc, char *argv[])
{

    // Initialise WinSoc.
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSA startup failed. Control server can not start.\n");
        return 1;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    auto return_val = a.exec();


    // Clean up.
    WSACleanup();
    return return_val;
}
