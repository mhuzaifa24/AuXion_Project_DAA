#define NOMINMAX
#include "UDP_broadcaster.h"
#include "coordinator.h"
#include "bidder.h"
#include "auction_data.h"
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

void start_tcp_server(int port);  // NEW (we will define this below)

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "WSAStartup failed: " << WSAGetLastError() << endl;
        return 1;
    }

    string self_ip = get_self_ip();
    string my_id;
    int my_udp_port;

    cout << "Enter your Node ID (e.g., Node1): ";
    cin >> my_id;
    g_node_id = my_id;

    cout << "Enter your UDP Port (different for each node, e.g., 8001, 8002...): ";
    cin >> my_udp_port;

 
    cout << "\n\t\t\t\t=== Starting Discovery Phase (10 seconds) ===" << endl;

    auto discovery_start = chrono::steady_clock::now();
    thread broadcaster(udp_broadcaster, my_id, my_udp_port);
    thread listener(udp_listener, self_ip);
    thread printer(print_network_map);

    
    while (chrono::steady_clock::now() - discovery_start < chrono::seconds(10)) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    // Signal completion
    discovery_complete = true;
    should_terminate = true;
    print_map = true;

    broadcaster.join();
    listener.join();
    printer.join();

    clear_console();

    {
        lock_guard<mutex> lock(mtx);
        if (network_map.empty()) {
            cout << "No peers discovered. Exiting..." << endl;
            WSACleanup();
            return 0;
        }
        network_map[my_id] = { self_ip, my_udp_port };
        cout << "\n=== Discovery Results ===" << endl;
        for (auto& entry : network_map) {
            cout << "Node: " << entry.first << " | IP: " << entry.second.first
                << " | Port: " << entry.second.second << endl;
        }
    }

    // ?? NEW: Start TCP Server
    int tcp_port = my_udp_port + 1000; // Just an example: 8001 UDP ? 9001 TCP
    thread tcp_server_thread(start_tcp_server, tcp_port);

    // ?? NEW: Sleep a little to let TCP servers start
    this_thread::sleep_for(chrono::seconds(2));

    // ?? NEW: Connect to all other peers as TCP client
    vector<SOCKET> peer_sockets;
    {
        lock_guard<mutex> lock(mtx);
        for (auto& entry : network_map)
        {
            if (entry.first != my_id)
            {
                string peer_ip = entry.second.first;
                int peer_tcp_port = entry.second.second + 1000; // 8002 UDP ? 9002 TCP

                SOCKET peer_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (peer_sock == INVALID_SOCKET)
                {
                    cerr << "Failed to create TCP socket for " << entry.first << endl;
                    continue;
                }

                sockaddr_in peer_addr;
                peer_addr.sin_family = AF_INET;
                peer_addr.sin_port = htons(peer_tcp_port);
                inet_pton(AF_INET, peer_ip.c_str(), &peer_addr.sin_addr);

                if (connect(peer_sock, (sockaddr*)&peer_addr, sizeof(peer_addr)) == SOCKET_ERROR)
                {
                    cerr << "Failed to connect to " << entry.first << endl;
                    closesocket(peer_sock);
                }
                else
                {
                    cout << "Connected to " << entry.first << " on TCP." << endl;
                    peer_sockets.push_back(peer_sock);
                }
            }
        }
    }


    std::cout << "\n----------------------------------------------- Welcome to AuXion----------------------------------------------------\n\n";

    int choice = 0;
    while (choice != 1 && choice != 2) {
        cout << "\nSelect your role:\n";
        cout << "1. Starter (Start auction)\n";
        cout << "2. Bidder (Place bids)\n";
        cout << "Enter choice (1 or 2): ";
        cin >> choice;
    }

    if (choice == 1) {
        Coordinator coord;
        coord.run_as_coordinator(peer_sockets); // pass sockets
    }
    else {
        Bidder bidder;
        bidder.run_as_bidder(peer_sockets); // pass sockets
    }

    tcp_server_thread.join();
    WSACleanup();
    return 0;
}

void start_tcp_server(int port)
{
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET)
    {
        cerr << "TCP Server socket creation failed\n";
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        cerr << "TCP Server bind failed\n";
        closesocket(listen_sock);
        return;
    }

    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "TCP Server listen failed\n";
        closesocket(listen_sock);
        return;
    }

    cout << "[TCP Server] Listening on port " << port << endl;

    while (true) // optionally put a timeout/exit signal here
    {
        SOCKET client_sock = accept(listen_sock, NULL, NULL);
        if (client_sock != INVALID_SOCKET)
        {
            cout << "[TCP Server] Accepted a connection.\n";
            // ?? Now you can handle incoming auction messages
            // OR push into some global vector<SOCKET> if needed
        }
    }

    closesocket(listen_sock);
}


