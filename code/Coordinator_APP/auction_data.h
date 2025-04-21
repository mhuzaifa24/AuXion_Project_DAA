#pragma once
#include <string>
#include <ctime>

using namespace std;

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


