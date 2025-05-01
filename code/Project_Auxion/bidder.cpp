#define NOMINMAX
#include "bidder.h"
#include "UDP_broadcaster.h"
#include <iostream>
#include <limits>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

void Bidder::run_as_bidder(const std::vector<SOCKET>& peer_sockets)
{
    std::cout << "\n=== Running as Bidder ===" << std::endl;

    if (network_map.empty())
    {
        std::cerr << "No coordinator found!" << std::endl;
        return;
    }

    auto coordinator = network_map.begin();
    std::cout << "Connecting to coordinator: " << coordinator->first
        << " (" << coordinator->second.first << ":" << TCP_PORT << ")" << std::endl;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_PORT);
    if (inet_pton(AF_INET, coordinator->second.first.c_str(), &addr.sin_addr) <= 0)
    {
        std::cerr << "Failed to convert coordinator address: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return;
    }

    // Set socket timeout for connect
    timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to connect to coordinator: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return;
    }

    // Receive welcome message
    char buffer[1024] = { 0 };
    int valread = recv(sock, buffer, sizeof(buffer), 0);
    if (valread > 0) {
        buffer[valread] = '\0';
        std::cout << "Coordinator: " << buffer << std::endl;
    }
    else {
        std::cerr << "Failed to receive welcome message: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return;
    }

    // Clear input buffer
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

    // Send bid
    std::string bid_value;
    std::cout << "\nEnter your bid amount (must be >= 1500): ";
    std::getline(std::cin, bid_value);
    std::cout << "Debug: bid_value = '" << bid_value << "'" << std::endl;

    if (bid_value.empty()) {
        std::cerr << "Error: No bid entered" << std::endl;
        closesocket(sock);
        return;
    }

    try {
        int amount = std::stoi(bid_value);
        if (amount < 1500) {
            std::cerr << "Bid must be >= 1500" << std::endl;
            closesocket(sock);
            return;
        }

        // Simplified message format
        std::string bid_message = g_node_id + ":" + bid_value;
        std::cout << "Debug: Sending bid_message = '" << bid_message << "'" << std::endl;

        // Add message terminator
        bid_message += "\n";

        int bytes_sent = send(sock, bid_message.c_str(), bid_message.length(), 0);
        if (bytes_sent == SOCKET_ERROR) {
            std::cerr << "Failed to send bid: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            return;
        }
        std::cout << "Debug: Sent " << bytes_sent << " bytes" << std::endl;

        // Wait for confirmation with timeout
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(sock, &readset);
        timeval timeout;
        timeout.tv_sec = 5;  // 5 second timeout
        timeout.tv_usec = 0;

        int sel = select(0, &readset, NULL, NULL, &timeout);
        if (sel > 0) {
            char confirm_buffer[1024] = { 0 };
            int confirm_valread = recv(sock, confirm_buffer, sizeof(confirm_buffer), 0);
            if (confirm_valread > 0) {
                confirm_buffer[confirm_valread] = '\0';
                std::cout << "Coordinator confirmation: " << confirm_buffer << std::endl;
            }
            else {
                std::cerr << "Connection closed by coordinator" << std::endl;
            }
        }
        else {
            std::cerr << "Confirmation timeout or error" << std::endl;
        }
    }
    
    catch (const std::exception& e) {
        std::cerr << "Invalid bid amount: " << e.what() << std::endl;
    }

    closesocket(sock);
}
