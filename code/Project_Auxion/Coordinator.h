#pragma once
#include "auction_data.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <string>
#include<vector>
#pragma comment(lib, "ws2_32.lib")

class Coordinator {
public:
    void run_as_coordinator(const std::vector<SOCKET>& peer_sockets);
private:
    const int TCP_PORT = 8080;
};
