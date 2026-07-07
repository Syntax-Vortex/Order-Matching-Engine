#include <bits/stdc++.h>

enum class OrderType{
    GoodTillCancel,
    FillAndKill
};

enum class Side{
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

struct LevelInfo{
    Price price_;
    Quantity quantity_;
};
using Levels = std::vector<LevelInfo>; //he named this LevelInfos

class OrderbookLevels{
    public:
        OrderbookLevels(const Levels &bids, const Levels &asks):    //initialiser list
            bids_{bids},
            asks_{asks}
        {}
        
        const Levels& GetBids(){ //this has Levels& cause it returns a reference to bids_, not a copy
            return bids_;
        }
        const Levels& GetAsks(){
            return asks_;
        }
    
    private:
        Levels bids_;
        Levels asks_;
};

class Order{
    public:
        Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity):
            orderType_{orderType},
            orderId_{orderId},
            side_{side_},
            price_{price},
            initialQuantity_{quantity},
            remainingQuantity_{quantity}
        {}

        OrderId GetOrderId() const {return orderId_;} //the const here marks the method const safe, meaning its guaranteed that it cant change its object variables
        Side GetSide() const {return side_;}
        Price GetPrice() const {return price_;}
        OrderType GetOrderType() const {return orderType_;}
        Quantity GetInitialQuantity() const {return initialQuantity_;}
        Quantity GetRemainingQuantity() const {return remainingQuantity_;}
        Quantity GetFilledQuantity() const {return initialQuantity_ - remainingQuantity_;}
        void Fill(Quantity quantity){
            if(quantity > remainingQuantity_){
                throw std::logic_error("Order (" + std::to_string(GetOrderId()) + ") cannot be overfilled");
            }
            remainingQuantity_ -= quantity;
        }

    private:
        OrderType orderType_;
        OrderId orderId_;
        Side side_;
        Price price_;
        Quantity initialQuantity_;
        Quantity remainingQuantity_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;

class OrderModify{
    public:
        OrderModify(OrderId orderId, Side side, Price price, Quantity quantity):
            orderId_{orderId},
            side_{side},
            price_{price},
            quantity_{quantity}
        {}

        OrderId GetOrderId() const {return orderId_;}
        Side GetSide() const {return side_;}
        Price GetPrice() const {return price_;}
        Quantity GetQuantity() const {return quantity_;}
        
        OrderPointer ToOrderPointer(OrderType type) const {
            return std::make_shared<Order>(type, orderId_, side_, price_, quantity_);
        }
    
    private:
        OrderId orderId_;
        Side side_;
        Price price_;
        Quantity quantity_; 
};

struct TradeInfo{
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
};

class Trade{
    public:
        Trade(const TradeInfo &bidTrade, const TradeInfo &askTrade):
            bidTrade_{bidTrade},
            askTrade_{askTrade}
        {}

        const TradeInfo& GetBidTrade() const {return bidTrade_;}
        const TradeInfo& GetAskTrade() const {return askTrade_;}

    private:
        TradeInfo bidTrade_;
        TradeInfo askTrade_;
};

using Trades = std::vector<Trade>;

class OrderBook{
    private:
        struct OrderEntry{
            OrderPointer order_{nullptr};
            OrderPointers::iterator location_;
        };

        std::map<Price, OrderPointers, std::less<Price>> asks_;
        std::map<Price, OrderPointers, std::greater<Price>> bids_;
        std::unordered_map<OrderId, OrderEntry> orders_;
};

int main(){
    return 0;
}