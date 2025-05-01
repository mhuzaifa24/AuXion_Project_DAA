#pragma once
#define NOMINMAX
#include "auction_data.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include<vector>
#pragma comment(lib, "ws2_32.lib")

class Bidder {
public:
    void run_as_bidder(const std::vector<SOCKET>& peer_sockets);

private:
    const int TCP_PORT = 8080;
};
