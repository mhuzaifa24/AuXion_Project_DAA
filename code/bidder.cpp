#include <iostream>
#include <cstdlib>
#include <ctime>
#include "auction_data.h"

class Bidder 
{
public:
    Bidder(string id) : bidder_id(id) {}

    Bid placeBid(Item item) 
    {
        int bidAmount = item.base_price + (rand() % 5000);
        cout << "[" << bidder_id << "] Bidding: " << bidAmount << "\n";

        Bid bid { bidder_id, item.item_id, bidAmount, time(0) };
        return bid;
    }

private:
    string bidder_id;
};
