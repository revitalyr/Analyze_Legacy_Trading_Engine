#pragma once

#include <list>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "order.h"
#include "spinlock.h"
#include "pricelevels.h"

struct Trade {
    friend class OrderBook;
private:
    Trade(F price, int quantity, const Order& aggressor, const Order& opposite) 
        : price(price), quantity(quantity), aggressor(aggressor), opposite(opposite) {}
public:
    const F price;
    const int quantity;
    const Order& aggressor;
    const Order& opposite;
    const long execId = epoch().count();
};

typedef void (*TradeReceiver)(Trade);

class OrderBookListener {
public:
    virtual void onOrder(const Order& order) {}
    virtual void onTrade(const Trade& trade) {}
};

struct BookLevel {
    F price;
    int quantity;
};

struct Book {
    std::vector<BookLevel> bids;
    std::vector<long> bidOrderIds;
    std::vector<BookLevel> asks;
    std::vector<long> askOrderIds;
};

inline std::ostream& operator<<(std::ostream& os, const Book& book) {
    bool first=true;
    for (auto side : {book.asks,book.bids}) {
        for(auto level : side) {
            os << level.price << " " << level.quantity << "\n";
        }
        if(first) { os << "----------\n"; first=false; }
    }
    return os;
}

// map of Session+QuoteId to the associated orders, or null if no quote on that side
struct QuoteOrders {
    std::shared_ptr<Order> bid = nullptr;
    std::shared_ptr<Order> ask = nullptr;
};

struct SessionQuoteId {
    const std::string sessionId;
    const std::string quoteId;
    SessionQuoteId(const std::string& sessionId, const std::string_view& quoteId) : sessionId(sessionId), quoteId(quoteId){}
    bool operator<(const SessionQuoteId& other) const {
        return sessionId<other.sessionId || (sessionId==other.sessionId && quoteId<other.quoteId);
    }
    bool operator==(const SessionQuoteId& other) const {
        return sessionId==other.sessionId && quoteId==other.quoteId;
    }
};

inline std::ostream& operator<<(std::ostream& os, const SessionQuoteId& id) {
    return os << "[" << id.sessionId << ":" << id.quoteId << "]";
}

class Exchange;

/** OrderBook instances are single threaded and must be externally synchronized using mu or lock() */
class OrderBook {
private:
    SpinLock mu;
    PriceLevels bids = PriceLevels(false);    
    PriceLevels asks = PriceLevels(true);
    OrderBookListener& listener;
    void matchOrders(Order::Side aggressorSide);
    std::map<SessionQuoteId,QuoteOrders> quotes;
    
public:
    const std::string instrument;
    OrderBook(const std::string &instrument, OrderBookListener& listener) : listener(listener), instrument(instrument) {}
    ~OrderBook() = default;

    void insertOrder(std::shared_ptr<Order> order);
    int cancelOrder(std::shared_ptr<Order> order);

    QuoteOrders getQuotes(const std::string& sessionId, const std::string& quoteId, std::function<QuoteOrders()> createOrders);
    void quote(const QuoteOrders& quotes, F bidPrice, int bidQuantity, F askPrice, int askQuantity);

    const Book book() const;
    const Order getOrder(std::shared_ptr<Order> order);
    std::vector<std::string> instruments() const {
        return {instrument};
    }
    Guard lock() {
        return Guard(mu);
    }
    
    // Modern factory method for creating orders
    template<typename OrderType, typename... Args>
    std::shared_ptr<OrderType> createOrder(Args&&... args) {
        return std::make_shared<OrderType>(std::forward<Args>(args)...);
    }
};