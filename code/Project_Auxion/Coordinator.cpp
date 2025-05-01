    #define NOMINMAX
    #include "coordinator.h"
    #include "UDP_broadcaster.h"
    #include <iostream>
    #include <thread>
    #include <vector>
    #include <chrono>
    #include <winsock2.h>
    #include <ws2tcpip.h>

    void handle_bid(SOCKET client_socket, std::string peer_name, Item& item, std::map<std::string, int>& bids);

    void Coordinator::run_as_coordinator(const std::vector<SOCKET>& peer_sockets)
    {

        std::cout << "\n\t\t\t\t=== Running as Auction Coordinator ===" << std::endl;

        SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == INVALID_SOCKET)
        {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
            return;
        }
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
        address.sin_port = htons(TCP_PORT);    // Ensure TCP_PORT is correct (e.g., 9001)

        if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0)
        {
            std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
            closesocket(server_fd);
            return;
        }

        if (listen(server_fd, network_map.size()) < 0)
        {
            std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
            closesocket(server_fd);
            return;
        }

       // std::cout << "Coordinator listening on port " << TCP_PORT << std::endl;

        Item item;
        item.item_id = "ITEM001";
        item.name = "Gaming Laptop";
        item.base_price = 1500;

        std::cout << "\n[Auction Coordinator] Auction started for: "
            << item.name << " (Base Price: " << item.base_price << ")" << std::endl;

        std::map<std::string, int> bids;
        std::vector<std::thread> threads;

        // Handle self bid
        if (network_map.find(g_node_id) != network_map.end())
        {
            int bid_amount;
            std::cout << "\nYour turn to bid (must be >= " << item.base_price << "): ";
            std::cin >> bid_amount;
            if (bid_amount >= item.base_price) {
                bids[g_node_id] = bid_amount;
                std::cout << "Your bid received. \t\t\t\t\t\t\t\tCurrent leader: " << g_node_id<<":"
                    << bid_amount << ")\n";
            }
            else {
                std::cerr << "Invalid bid: Must be >= " << item.base_price << std::endl;
            }
        }

        // Handle other bidders
        for (auto& peer : network_map)
        {
            if (peer.first == g_node_id) continue;

            std::cout << "\nWaiting for " << peer.first << " to bid...  " << std::endl;

            // Set timeout for accept
            timeval timeout;
            timeout.tv_sec = 15;
            timeout.tv_usec = 0;
            setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

            sockaddr_in client_addr;
            int addrlen = sizeof(client_addr);
            SOCKET client_socket = accept(server_fd, (sockaddr*)&client_addr, &addrlen);
            if (client_socket == INVALID_SOCKET)
            {
                std::cerr << "Accept failed for " << peer.first << ": " << WSAGetLastError() << std::endl;
                continue;
            }

            threads.emplace_back(handle_bid, client_socket, peer.first, std::ref(item), std::ref(bids));
        }

        // Wait for all bid threads to complete
        for (auto& t : threads) t.join();

        // Determine winner
        int highest = item.base_price;
        std::string winner;
        for (auto& b : bids)
        {
            if (b.second > highest)
            {
                highest = b.second;
                winner = b.first;
            }
        }
        std::cout << "\n[Auction Closed] Winner: " << (winner.empty() ? "None" : winner)
            << " with bid: " << highest << std::endl;

        closesocket(server_fd);
    }

    void handle_bid(SOCKET client_socket, std::string peer_name, Item& item, std::map<std::string, int>& bids)
    {
        // Set socket to non-blocking
        u_long mode = 1;
        ioctlsocket(client_socket, FIONBIO, &mode);

        // Send welcome message
        std::string welcome = "Connected to coordinator. Please send your bid in format: bid_amount\n";
        send(client_socket, welcome.c_str(), welcome.length(), 0);

        // Receive with timeout
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(client_socket, &readset);
        timeval timeout;
        timeout.tv_sec = 10;  // 10 second timeout
        timeout.tv_usec = 0;

        int sel = select(0, &readset, NULL, NULL, &timeout);
        {
            if (sel > 0) {
                char buffer[1024] = { 0 };
                int valread = recv(client_socket, buffer, sizeof(buffer), 0);
                if (valread > 0) {
                    buffer[valread] = '\0';
                    std::string msg(buffer);
                    std::cout << "Received raw bid: " << msg << std::endl;  // Debug log

                    // Handle both formats for backward compatibility
                    size_t first_colon = msg.find(':');
                    if (first_colon != std::string::npos) {
                        std::string bidder_id = msg.substr(0, first_colon);

                        // Find the actual bid amount (could be after second colon)
                        size_t second_colon = msg.find(':', first_colon + 1);
                        std::string bid_str = (second_colon != std::string::npos) ?
                            msg.substr(second_colon + 1) : msg.substr(first_colon + 1);

                        try {
                            int bid_amount = std::stoi(bid_str);
                            if (bid_amount >= item.base_price) {
                                std::lock_guard<std::mutex> lock(mtx);  // Add mutex if needed
                                bids[bidder_id] = bid_amount;

                                // Find current highest bid
                                int highest = item.base_price;
                                std::string leader;
                                for (const auto& b : bids) {
                                    if (b.second > highest) {
                                        highest = b.second;
                                        leader = b.first;
                                    }
                                }

                                std::string reply = "Bid received.\t\t\t\t\tCurrent leader: " +
                                    (leader.empty() ? "None" : leader) + ":" + std::to_string(highest) + "\n";
                                send(client_socket, reply.c_str(), reply.length(), 0);
                            }
                            else {
                                std::string reply = "Bid too low. Minimum is " + std::to_string(item.base_price) + "\n";
                                send(client_socket, reply.c_str(), reply.length(), 0);
                            }
                        }
                        catch (...) {
                            std::string reply = "Invalid bid format. Please use: node_id:bid_amount\n";
                            send(client_socket, reply.c_str(), reply.length(), 0);
                        }
                    }
                }
            }
            // ... (rest of the function)

            else
            {
                std::cerr << "Timeout waiting for bid from " << peer_name << std::endl;
            }
            closesocket(client_socket);
        }
    }
