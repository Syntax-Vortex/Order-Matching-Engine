#pragma once

#include "OrderType.h"
#include "Side.h"
#include "Usings.h"

#include <list>
#include <memory>
#include <stdexcept>
#include <string>

class Order{
public:
    Order(
        OrderType orderType,
        OrderId orderId,
        Side side,
        Price price,
        Quantity quantity
    ):
        orderType_{orderType},
        orderId_{orderId},
        side_{side},
        price_{price},
        initialQuantity_{quantity},
        remainingQuantity_{quantity}
    {
    }

    OrderId GetOrderId() const{
        return orderId_;
    }

    Side GetSide() const{
        return side_;
    }

    Price GetPrice() const{
        return price_;
    }

    OrderType GetOrderType() const{
        return orderType_;
    }

    Quantity GetInitialQuantity() const{
        return initialQuantity_;
    }

    Quantity GetRemainingQuantity() const{
        return remainingQuantity_;
    }

    Quantity GetFilledQuantity() const{
        return initialQuantity_ - remainingQuantity_;
    }

    bool IsFilled() const{
        return remainingQuantity_ == 0;
    }

    void Fill(Quantity quantity){
        if(quantity > remainingQuantity_){
            throw std::logic_error(
                "Order (" +
                std::to_string(GetOrderId()) +
                ") cannot be overfilled"
            );
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