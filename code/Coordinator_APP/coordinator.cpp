#include "coordinator.h"
int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, MAX_BIDDERS);

    Item item;
    item.item_id = "ITEM001";
    item.name = "Gaming Laptop";
    item.base_price = 1500;

    Coordinator coord;
    coord.startAuction(item);

    map<int, int> bids;

    for (int i = 1; i <= MAX_BIDDERS; ++i) 
    {
        cout << "[Coordinator] Waiting for Node " << i << "...\n";

        sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);
        SOCKET clientSocket = accept(server_fd, (sockaddr*)&client_addr, &addrlen);

        char buffer[1024] = { 0 };
        int valread = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (valread <= 0) {
            closesocket(clientSocket);
            continue;
        }

        string msg(buffer);
        cout << "[Coordinator] Received: " << msg << "\n";

        istringstream iss(msg);
        string nodeStr, label, bidVal;
        getline(iss, nodeStr, ':');
        getline(iss, label, ':');
        getline(iss, bidVal);

        int nodeNum = atoi(nodeStr.substr(4).c_str());  // "Node1" → 1
        int bid = atoi(bidVal.c_str());
        bids[nodeNum] = bid;

        // Determine current leader
        int leader = -1;
        int highest = -1;
        for (map<int, int>::const_iterator it = bids.begin(); it != bids.end(); ++it) {
            if (it->second > highest) {
                highest = it->second;
                leader = it->first;
            }
        }

        string reply = "Bid received. Current leader: Node " + to_string(leader);
        send(clientSocket, reply.c_str(), reply.length(), 0);

        closesocket(clientSocket);
    }

    cout << "\nAll bids received:\n";
    for (map<int, int>::const_iterator it = bids.begin(); it != bids.end(); ++it) {
        cout << " - Node " << it->first << " | Bid: " << it->second << "\n";
    }

    int winner = -1;
    int maxBid = -1;
    for (map<int, int>::const_iterator it = bids.begin(); it != bids.end(); ++it) {
        if (it->second > maxBid) {
            maxBid = it->second;
            winner = it->first;
        }
    }

    cout << "\nWinner: Node " << winner << " with highest bid: " << maxBid << "\n";

    closesocket(server_fd);
    WSACleanup();
    return 0;
}
