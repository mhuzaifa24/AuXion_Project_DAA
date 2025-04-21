#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include "auction_data.h"


#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define PORT 8080
#define MAX_BIDDERS 3


class Coordinator
{
public:
    void startAuction(Item item)
    {
        cout << "[Coordinator] Auction started for: " << item.name << " (Base Price: " << item.base_price << ")\n";
    }
}; 
