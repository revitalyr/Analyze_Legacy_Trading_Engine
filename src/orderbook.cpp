#include "core/orderbook.h"
#include "core/order.h"

#ifdef _WIN32
#include <algorithm>
#define MIN(a,b) std::min(a,b)
#else
#include <sys/param.h>
#endif
#include <functional>

#define LOCK_BOOK() std::lock_guard<std::recursive_mutex> lock(mu)

void OrderBook::insertOrder(std::shared_ptr<Order> order) {
    // Add null pointer check
    if (!order) {
        return;
    }
    
    // Add safety check for order state
    if (order->remaining <= 0) {
        return;
    }
    
    auto list = order->side == Order::BUY ? &bids : &asks;
    
    // Add safety check for list
    if (!list) {
        return;
    }
    
    list->insertOrder(order);
    listener.onOrder(*order);
    matchOrders(order->side);
}

void OrderBook::matchOrders(Order::Side aggressorSide) {
    while (!bids.empty() && !asks.empty()) {
        auto bid = bids.front();
        auto ask = asks.front();

        if (bid->_price >= ask->_price) {
            int qty = MIN(bid->remaining, ask->remaining);
            F price = MIN(bid->_price, ask->_price);

            std::shared_ptr<Order> aggressor = aggressorSide == Order::BUY ? bid : ask;
            std::shared_ptr<Order> opposite = aggressorSide == Order::BUY ? ask : bid;

            bid->fill(qty,price);
            ask->fill(qty,price);

            const Trade trade(price, qty, *aggressor, *opposite);

            if (bid->remaining == 0) {
                bids.removeOrder(bid);
            }
            if (ask->remaining == 0) {
                asks.removeOrder(ask);
            }
            listener.onOrder(*bid);
            listener.onOrder(*ask);
            listener.onTrade(trade);
        } else {
            break;
        }
    }
    // cancel remaining market order
    // TODO support convert to limit order
    auto orders = aggressorSide == Order::BUY ? &bids : &asks;
    if (!orders->empty()) {
        auto order = orders->front();
        if (order && order->isMarket()) {
            order->cancel();
            orders->removeOrder(order);
            listener.onOrder(*order);
        }
    }
}

QuoteOrders OrderBook::getQuotes(const std::string& sessionId, const std::string& quoteId, std::function<QuoteOrders()> createOrders) {
    auto key = SessionQuoteId(sessionId, quoteId);
    auto itr = quotes.find(key);
    if (itr == quotes.end()) {
        return quotes[key] = createOrders();
    } else {
        return itr->second;
    }
}

void OrderBook::quote(const QuoteOrders& quotes, F bidPrice, int bidQuantity, F askPrice, int askQuantity) {
    auto bid = quotes.bid;
    auto ask = quotes.ask;
    if(bid->isOnList()) {
        bids.removeOrder(bid);
    }
    if(ask->isOnList()) {
        asks.removeOrder(ask);
    }
    if (bidQuantity != 0) {
        bid->_price = bidPrice;
        bid->_quantity = bidQuantity;
        bid->remaining = bidQuantity;
        bid->filled = 0;
        bids.insertOrder(bid);
        matchOrders(Order::BUY);
    }
    if (askQuantity != 0) {
        ask->_price = askPrice;
        ask->_quantity = askQuantity;
        ask->remaining = askQuantity;
        ask->filled = 0;
        asks.insertOrder(ask);
        matchOrders(Order::SELL);
    }
}

int OrderBook::cancelOrder(std::shared_ptr<Order> order) {
    // Add null pointer check
    if (!order) {
        return -1;
    }
    
    // Add bounds checking for remaining quantity
    if (order->remaining > 0) {
        order->cancel();
        auto orders = order->side == Order::BUY ? &bids : &asks;
        
        // Add safety check before removal
        if (orders && order->isOnList()) {
            orders->removeOrder(order);
            listener.onOrder(*order);
            return 0;
        } else {
            // Order not found in lists or not on list
            return -1;
        }
    } else {
        return -1;
    }
}

const Book OrderBook::book() const {
    Book book;
    auto snap = [](const PriceLevels& src, std::vector<BookLevel>& dst, std::vector<long>& oids) {
        auto fn = [&](const OrderList* orders) {
            int quantity(0);
            for (auto itr = orders->begin(); itr != orders->end(); ++itr) {
                quantity = quantity + (*itr)->remainingQuantity();
                oids.push_back((*itr)->exchangeId);
            }
            dst.push_back({orders->price(), quantity});
        };
        src.forEach(fn);
    };
    book.bids.reserve(bids.size());
    book.asks.reserve(asks.size());
    snap(bids, book.bids, book.bidOrderIds);
    snap(asks, book.asks, book.askOrderIds);
    return book;
}

const Order OrderBook::getOrder(std::shared_ptr<Order> order) {
    if (!order) {
        throw std::invalid_argument("Order cannot be null");
    }
    return *order;
}
