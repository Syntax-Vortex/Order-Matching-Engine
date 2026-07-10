#include "../include/Orderbook.h"

#include <algorithm>
#include <iterator>
#include <numeric>


bool OrderBook::CanMatch(Side side, Price price) const{

    if(side == Side::Buy){
        if(asks_.empty()) return false;

        Price bestAsk = asks_.begin()->first;

        return price >= bestAsk;
    }else{
        if(bids_.empty()) return false;

        Price bestBid = bids_.begin()->first;

        return price <= bestBid;
    }
}


Trades OrderBook::MatchOrders(){

    Trades trades;
    trades.reserve(orders_.size());

    while(1){

        if(bids_.empty() || asks_.empty()) break;

        Price bidPrice = bids_.begin()->first;
        Price askPrice = asks_.begin()->first;

        if(bidPrice < askPrice) break;

        OrderPointers &bestBids = bids_.begin()->second;
        OrderPointers &bestAsks = asks_.begin()->second;

        while(!bestBids.empty() && !bestAsks.empty()){

            auto bestBid = bestBids.front();
            auto bestAsk = bestAsks.front();

            Quantity matchedQuantity = std::min(
                bestBid->GetRemainingQuantity(),
                bestAsk->GetRemainingQuantity()
            );

            bestBid->Fill(matchedQuantity);
            bestAsk->Fill(matchedQuantity);

            trades.push_back(Trade{
                TradeInfo{
                    bestBid->GetOrderId(),
                    bestBid->GetPrice(),
                    matchedQuantity
                },
                TradeInfo{
                    bestAsk->GetOrderId(),
                    bestAsk->GetPrice(),
                    matchedQuantity
                }
            });

            if(bestBid->IsFilled()){
                bestBids.pop_front();
                orders_.erase(bestBid->GetOrderId());
            }

            if(bestAsk->IsFilled()){
                bestAsks.pop_front();
                orders_.erase(bestAsk->GetOrderId());
            }
        }

        if(bestBids.empty()){
            bids_.erase(bidPrice);
        }

        if(bestAsks.empty()){
            asks_.erase(askPrice);
        }
    }

    return trades;
}


Trades OrderBook::AddOrder(OrderPointer order){

    OrderId orderId = order->GetOrderId();

    if(orders_.find(orderId) != orders_.end()){
        return {};
    }

    if(
        order->GetOrderType() == OrderType::FillAndKill &&
        !CanMatch(order->GetSide(), order->GetPrice())
    ){
        return {};
    }

    OrderPointers::iterator location;

    if(order->GetSide() == Side::Buy){

        OrderPointers &orders = bids_[order->GetPrice()];

        orders.push_back(order);

        location = std::prev(orders.end());

    }else{

        OrderPointers &orders = asks_[order->GetPrice()];

        orders.push_back(order);

        location = std::prev(orders.end());
    }

    orders_[orderId] = OrderEntry{
        order,
        location
    };

    Trades trades = MatchOrders();

    if(
        order->GetOrderType() == OrderType::FillAndKill &&
        orders_.find(orderId) != orders_.end()
    ){
        CancelOrder(orderId);
    }

    return trades;
}


void OrderBook::CancelOrder(OrderId orderId){

    auto entryIterator = orders_.find(orderId);

    if(entryIterator == orders_.end()){
        return;
    }

    OrderPointer order = entryIterator->second.order_;

    OrderPointers::iterator orderIterator =
        entryIterator->second.location_;

    Price price = order->GetPrice();

    if(order->GetSide() == Side::Sell){

        OrderPointers &orders = asks_.at(price);

        orders.erase(orderIterator);

        if(orders.empty()){
            asks_.erase(price);
        }

    }else{

        OrderPointers &orders = bids_.at(price);

        orders.erase(orderIterator);

        if(orders.empty()){
            bids_.erase(price);
        }
    }

    orders_.erase(orderId);
}


Trades OrderBook::MatchOrder(OrderModify order){

    OrderId orderId = order.GetOrderId();

    auto entryIterator = orders_.find(orderId);

    if(entryIterator == orders_.end()){
        return {};
    }

    OrderType orderType =
        entryIterator->second.order_->GetOrderType();

    CancelOrder(orderId);

    return AddOrder(
        order.ToOrderPointer(orderType)
    );
}


std::size_t OrderBook::Size() const{

    return orders_.size();
}


OrderbookLevels OrderBook::GetOrderLevels() const{
    Levels bids, asks;
    bids.reserve(orders_.size());
    asks.reserve(orders_.size());

    auto CreateLevelInfos = [](Price price, const OrderPointers& orders){
        return LevelInfo{price, std::accumulate(orders.begin(), orders.end(), (Quantity)0,
            [](Quantity runningSum, const OrderPointer& order){
                return runningSum + order->GetRemainingQuantity();
        })};
    };

    for(const auto &p : bids_){
        Price price = p.first;
        const OrderPointers &orders = p.second;
        bids.push_back(CreateLevelInfos(price, orders));
    }

    for(const auto &p : asks_){
        Price price = p.first;
        const OrderPointers &orders = p.second;
        asks.push_back(CreateLevelInfos(price, orders));
    }

    return OrderbookLevels{bids, asks};
}