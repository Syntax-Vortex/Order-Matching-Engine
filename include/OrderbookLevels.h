#pragma once

#include "Levels.h"

class OrderbookLevels{
public:
    OrderbookLevels(const Levels &bids, const Levels &asks):
        bids_{bids},
        asks_{asks}
    {}

    const Levels& GetBids() const{return bids_;}
    const Levels& GetAsks() const{return asks_;}

private:
    Levels bids_;
    Levels asks_;
};