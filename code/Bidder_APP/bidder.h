#pragma once
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "auction_data.h"

#pragma comment(lib, "ws2_32.lib")
#define PORT 8080

using namespace std;

class Bidder
{
public:
    Bidder(string id) : bidder_id(id)
    {
        srand(time(0));
    }

    Bid placeBid(const Item& item)
    {
        int bidAmount = item.base_price + (rand() % 5000);
        cout << "[" << bidder_id << "] Placing bid: " << bidAmount << "\n";
        return { bidder_id, item.item_id, bidAmount, time(0) };
    }

private:
    string bidder_id;
}; 
