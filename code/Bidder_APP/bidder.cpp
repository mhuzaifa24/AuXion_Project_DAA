#include "bidder.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: Bidder.exe <Node_ID>\n";
        return 1;
    }

    int nodeID = std::atoi(argv[1]);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection to Coordinator failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    int bid;
    std::cout << "[Node " << nodeID << "] Enter your bid amount: ";
    std::cin >> bid;

    std::ostringstream msg;
    msg << "Node" << nodeID << ":Bid:" << bid;
    send(sock, msg.str().c_str(), msg.str().length(), 0);

    char buffer[1024] = { 0 };
    int valread = recv(sock, buffer, sizeof(buffer), 0);
    if (valread > 0) {
        std::cout << "[Node " << nodeID << "] \n Coordinator says: " << buffer << "\n";
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
