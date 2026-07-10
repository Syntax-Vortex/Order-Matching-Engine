#pragma once

#include "Order.h"
#include "OrderModify.h"
#include "OrderbookLevels.h"
#include "Trade.h"

#include <functional>
#include <map>
#include <unordered_map>

class OrderBook{
private:
    struct OrderEntry{
        OrderPointer order_{nullptr};
        OrderPointers::iterator location_;
    };

    std::map<
        Price,
        OrderPointers,
        std::less<Price>
    > asks_;

    std::map<
        Price,
        OrderPointers,
        std::greater<Price>
    > bids_;

    std::unordered_map<
        OrderId,
        OrderEntry
    > orders_;

    bool CanMatch(Side side, Price price) const;

    Trades MatchOrders();

public:
    Trades AddOrder(OrderPointer order);

    void CancelOrder(OrderId orderId);

    Trades MatchOrder(OrderModify order);

    std::size_t Size() const;

    OrderbookLevels GetOrderLevels() const;
};