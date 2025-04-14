#include <iostream>
#include <vector>
#include "auction_data.h"
using namespace std;
class Coordinator 
{
public:
    void startAuction(Item item) 
    {
        cout << "[Coordinator] Auction started for: " << item.name << "\n";
        broadcastStart(item);
    }

    void receiveBid(Bid bid) 
    {
        cout << "[Coordinator] Received bid of " << bid.amount << " from " << bid.bidder_id << "\n";
        bids.push_back(bid);
    }

private:
    vector<Bid> bids;

    void broadcastStart(Item item) {
        // Simulated message
        cout << "[Coordinator] Sending auction start to bidders...\n";
    }
};
