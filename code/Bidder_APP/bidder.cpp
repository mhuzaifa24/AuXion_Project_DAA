#include "bidder.h"

int main()
{
    
    cout << "\n--------------------------------------    Hey Bidders! Welcome to AuXion    -----------------------------------------\n\n";

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

   
    for (int nodeID = 1; nodeID <= MAX_BIDDERS; ++nodeID) 
    {

        
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) 
        {
            cerr << "Socket creation failed for Node " << nodeID << "\n";
            WSACleanup();
            return 1;
        }

        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

       
        if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
        {
            cerr << "Connection to Coordinator failed for Node " << nodeID << "\n";
            closesocket(sock);
            continue; 
        }

        
        cout << "\n[Node " << nodeID << "] Enter your bid amount: ";
        int bid;
        cin >> bid;

       
        ostringstream msg;
        msg << "Node" << nodeID << ":Bid:" << bid;
        send(sock, msg.str().c_str(), msg.str().length(), 0);

        char buffer[1024] = { 0 };
        int valread = recv(sock, buffer, sizeof(buffer), 0);
        if (valread > 0) {
            cout << "[Node " << nodeID << "] Coordinator says: " << buffer << "\n";
        }


        closesocket(sock);
    }

    WSACleanup();
    return 0;
}
