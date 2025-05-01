    #include "UDP_broadcaster.h"
    #include <iostream>


    std::map<std::string, std::pair<std::string, int>> network_map;
    std::mutex mtx;
    std::atomic<bool> discovery_complete(false);
    std::atomic<bool> new_peer_discovered(false);
    std::atomic<bool> should_terminate(false);
    std::atomic<bool> print_map(false);
    std::string g_node_id;
    void udp_broadcaster(std::string my_id, int my_udp_port)
    {
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET) 
        {
            std::cerr << "UDP Socket creation failed\n";
            return;
        }

        BOOL broadcast = TRUE;
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast));

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(BROADCAST_PORT);


        if (inet_pton(AF_INET, "255.255.255.255", &addr.sin_addr) <= 0)
        {
           std:: cerr << "Failed to convert broadcast address" <<std:: endl;

        }


        auto discovery_start = std::chrono::steady_clock::now();

        while (!should_terminate &&
            std::chrono::steady_clock::now() - discovery_start < DISCOVERY_TIMEOUT)
        {
            std::string msg = my_id + ":" + std::to_string(my_udp_port);
            sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&addr, sizeof(addr));
            std::this_thread::sleep_for(std::chrono::milliseconds(BROADCAST_INTERVAL));
        }
        closesocket(sock);

    }

    void udp_listener(std::string self_ip) {
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cerr << "UDP Listener Socket creation failed\n";
            return;
        }

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(BROADCAST_PORT);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "UDP bind failed\n";
            closesocket(sock);
            return;
        }

        char buffer[1024];
        sockaddr_in sender_addr;
        int sender_len = sizeof(sender_addr);

        while (!discovery_complete) {
            int bytes_received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                (sockaddr*)&sender_addr, &sender_len);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                std::string message(buffer);

                size_t colon_pos = message.find(':');
                if (colon_pos != std::string::npos) {
                    std::string node_id = message.substr(0, colon_pos);

                    int udp_port = std::stoi(message.substr(colon_pos + 1));

                    char sender_ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(sender_addr.sin_addr), sender_ip_str, INET_ADDRSTRLEN);
                    std::string sender_ip(sender_ip_str);

                    std::lock_guard<std::mutex> lock(mtx);
                    if (network_map.find(node_id) == network_map.end()) 
                    {
                        std::cout << "\n[Discovered] " << node_id << " at " << sender_ip << ":" << udp_port << std::endl;
                        new_peer_discovered = true;
                    }
                    network_map[node_id] = { sender_ip, udp_port };
                }
            }
        }
        closesocket(sock);
    }

    std::string get_self_ip() 
    {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname))) 
        {
            std::cerr << "gethostname failed\n";
            return "127.0.0.1";
        }

        addrinfo hints = {}, * result = nullptr;
        hints.ai_family = AF_INET;

        if (getaddrinfo(hostname, nullptr, &hints, &result)) {
            std::cerr << "getaddrinfo failed\n";
            return "127.0.0.1";
        }

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((sockaddr_in*)result->ai_addr)->sin_addr, ip_str, sizeof(ip_str));
        freeaddrinfo(result);
        return std::string(ip_str);
    }

    void print_network_map() 
    {
        while (!print_map) {
            if (new_peer_discovered) {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "\n--- Current Live Network ---\n";
                for (auto& entry : network_map) {
                    std::cout << "Node: " << entry.first << " | IP: " << entry.second.first
                        << " | Port: " << entry.second.second << "\n";
                }
                new_peer_discovered = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            discovery_complete = true;
        }
    }

    void clear_console() {
        system("cls");  // Windows
        // system("clear");  // Linux/Mac
    }
