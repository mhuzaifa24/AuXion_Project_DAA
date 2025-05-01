#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <map>
#include <mutex>
#include <atomic>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")


extern std::map<std::string, std::pair<std::string, int>> network_map;
extern std::mutex mtx;
extern std::atomic<bool> discovery_complete;
extern std::atomic<bool> new_peer_discovered;
extern std::atomic<bool> should_terminate;
extern std::atomic<bool> print_map;
extern std::string g_node_id;

const int BROADCAST_PORT = 9000;
const int BROADCAST_INTERVAL = 1000;
const std::chrono::seconds DISCOVERY_TIMEOUT(5);


void udp_broadcaster(std::string my_id, int my_udp_port);
void udp_listener(std::string self_ip);
std::string get_self_ip();
void print_network_map();
void clear_console();
