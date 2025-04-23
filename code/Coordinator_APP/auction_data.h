#pragma once
#include <string>
#include <ctime>

using namespace std;

const int MAX_BIDDERS = 3;

struct Item
{
    string item_id;
    string name;
    int base_price;
};

struct Bid
{
    string bidder_id;
    string item_id;
    int amount;
    time_t timestamp;
};


