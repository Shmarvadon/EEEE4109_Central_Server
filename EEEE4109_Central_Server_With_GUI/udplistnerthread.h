#pragma once
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <qthread.h>

#include "pole.h"

#define UDP_BUFF_SIZE 512

#define START_GATE_TYPE 3
#define FINSIH_GATE_TYPE 4

class Pole;

class udplistnerthread : public QThread {
    Q_OBJECT
public:

    explicit udplistnerthread(QObject* parent, int UDPListnerPort, std::pair<uint32_t, uint32_t> TCPPortRange) : QThread(parent), _UDPListnerPort(UDPListnerPort), _TCPPortsRange(TCPPortRange), _NumberOfPolesConncted(0) {};
    void run() override;

signals:
    void appendNewPole(sockaddr_in poleAddress, int port, uint64_t HWID, uint8_t type);

    Pole* getPoleByHWID(uint64_t HWID);

private:
    std::pair<uint32_t, uint32_t> _TCPPortsRange;
    SOCKET UDPListener;
    SOCKET UDPSender;
    int _UDPListnerPort;
    int _NumberOfPolesConncted;
};