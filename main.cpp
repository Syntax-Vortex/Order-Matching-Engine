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
            side_{side},
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
        bool IsFilled() const {return remainingQuantity_ == 0;}

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

        bool CanMatch(Side side, Price price) const {   //checks whether an order can currently be matched or not
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

        Trades MatchOrders(){
            Trades trades;
            trades.reserve(orders_.size());
            
            while(1){
                if(bids_.empty() || asks_.empty()) break;

                const Price& bidPrice = bids_.begin()->first, &askPrice = asks_.begin()->first;
                OrderPointers& bestBids = bids_.begin()->second;
                OrderPointers& bestAsks = asks_.begin()->second;
                
                if(bidPrice < askPrice) break; //guard clause. getting past this means that the bestBids and bestAsk prices can be matched.

                while(bestBids.size() && bestAsks.size()){
                    auto bestBid = bestBids.front(); //get the earliest submitted order from tbe bestBids level
                    auto bestAsk = bestAsks.front(); //we do this cause we're following price time priority
                    
                    Quantity matchedQuantity = std::min(bestBid->GetRemainingQuantity(), bestAsk->GetRemainingQuantity());
                    bestBid->Fill(matchedQuantity);
                    bestAsk->Fill(matchedQuantity);
                    
                    if(bestBid->IsFilled()){
                        bestBids.pop_front();
                        orders_.erase(bestBid->GetOrderId());
                    }
                    if(bestAsk->IsFilled()){
                        bestAsks.pop_front();
                        orders_.erase(bestAsk->GetOrderId());
                    }
                    trades.push_back(Trade{
                        TradeInfo{bestBid->GetOrderId(), bestBid->GetPrice(), matchedQuantity},
                        TradeInfo{bestAsk->GetOrderId(), bestAsk->GetPrice(), matchedQuantity}});
                }
                if(bestBids.empty()) bids_.erase(bidPrice);
                if(bestAsks.empty()) asks_.erase(askPrice);
            }

            if(!bids_.empty()){
                OrderPointers& bids = bids_.begin()->second;
                OrderPointer& order = bids.front();
                if(order->GetOrderType() == OrderType::FillAndKill){
                    CancelOrder(order->GetOrderId());
                }
            }
            if(!asks_.empty()){
                OrderPointers& asks = asks_.begin()->second;
                OrderPointer& order = asks.front();
                if(order->GetOrderType() == OrderType::FillAndKill){
                    CancelOrder(order->GetOrderId());
                }
            }

            return trades;
        }

    public:
        Trades AddOrder(OrderPointer order){
            if(orders_.find(order->GetOrderId()) != orders_.end()){
                return {};
            }else if(order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice())){
                return {};
            }

            OrderPointers::iterator iterator;
            if(order->GetSide() == Side::Buy){
                OrderPointers& orders = bids_[order->GetPrice()];
                orders.push_back(order);
                iterator = std::next(orders.begin(), orders.size() - 1);

            }else{
                OrderPointers& orders = asks_[order->GetPrice()];
                orders.push_back(order);
                iterator = std::next(orders.begin(), orders.size() - 1);
            }

            orders_[order->GetOrderId()] = OrderEntry{order, iterator};

            return MatchOrders();
        }

        void CancelOrder(OrderId orderId){
            if(orders_.find(orderId) == orders_.end()) return;

            OrderPointer& order = orders_.at(orderId).order_;
            OrderPointers::iterator orderIterator = orders_.at(orderId).location_;
            
            if(order->GetSide() == Side::Sell){
                Price price = order->GetPrice();
                OrderPointers& orders = asks_.at(price);
                orders.erase(orderIterator);
                if(orders.empty()) asks_.erase(price);
            }else{
                Price price = order->GetPrice();
                OrderPointers& orders = bids_.at(price);
                orders.erase(orderIterator);
                if(orders.empty()) bids_.erase(price);
            }

            orders_.erase(orderId);
        }

        Trades MatchOrder(OrderModify order){
            if(orders_.find(order.GetOrderId()) == orders_.end()) return {};

            OrderPointer existingOrderPointer = orders_.at(order.GetOrderId()).order_;
            CancelOrder(order.GetOrderId());
            return AddOrder(order.ToOrderPointer(existingOrderPointer->GetOrderType()));
        }

        std::size_t Size() const {return orders_.size();}

        OrderbookLevels GetOrderLevels() const{
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
};

int main(){
    return 0;
}