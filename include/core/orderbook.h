#pragma once

#include <list>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <source_location>

#include "order.h"
#include "spinlock.h"
#include "pricelevels.h"
#include "insert_result.h"
#include "semantic_types.h"
#include "constants.h"

struct Trade {
    friend class OrderBook;
private:
    Trade(Price price, Quantity quantity, const Order& aggressor, const Order& opposite) 
        : m_price(price), m_quantity(quantity), m_aggressor(aggressor), m_opposite(opposite) {}
public:
    const Price m_price;
    const Quantity m_quantity;
    const Order& m_aggressor;
    const Order& m_opposite;
    /** execution timestamp in nanoseconds since epoch */
    const NanosecondTimestamp execId = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch().time_since_epoch()).count();
};

typedef void (*TradeReceiver)(Trade);

class OrderBookListener {
public: // Callbacks for order book events
    virtual void onOrder(const Order& ) {}
    virtual void onTrade(const Trade& ) {}
};

struct BookLevel {
    Price m_price; // Renamed to m_snake_case
    Quantity m_quantity; // Renamed to m_snake_case
};

struct Book {
    std::vector<BookLevel> m_bids; // Renamed to m_snake_case
    std::vector<ExchangeId> m_bidOrderIds; // Renamed to m_snake_case
    std::vector<BookLevel> m_asks; // Renamed to m_snake_case
    std::vector<ExchangeId> m_askOrderIds; // Renamed to m_snake_case
};

inline std::ostream& operator<<(std::ostream& os, const Book& book) {
    bool first = true;
    for (auto side : {book.m_asks, book.m_bids}) {
        for(auto level : side) {
            os << level.m_price << " " << level.m_quantity << "\n";
        }
        if(first) { os << "----------\n"; first=false; }
    }
    return os;
}

// map of Session+QuoteId to the associated orders, or null if no quote on that side
struct QuoteOrders { // Renamed to PascalCase
    std::shared_ptr<Order> m_bid = nullptr; // Renamed to m_snake_case
    std::shared_ptr<Order> m_ask = nullptr; // Renamed to m_snake_case
};

struct SessionQuoteId {
    const std::string m_sessionId; // Renamed to m_snake_case
    const std::string m_quoteId; // Renamed to m_snake_case
    SessionQuoteId(const std::string& sessionId, const std::string_view& quoteId) : m_sessionId(sessionId), m_quoteId(quoteId){} // Renamed to m_snake_case
    bool operator<(const SessionQuoteId& other) const {
        return m_sessionId < other.m_sessionId || (m_sessionId == other.m_sessionId && m_quoteId < other.m_quoteId); // Renamed to m_snake_case
    }
    bool operator==(const SessionQuoteId& other) const {
        return m_sessionId == other.m_sessionId && m_quoteId == other.m_quoteId; // Renamed to m_snake_case
    }
};

inline std::ostream& operator<<(std::ostream& os, const SessionQuoteId& id) {
    return os << "[" << id.m_sessionId << ":" << id.m_quoteId << "]";
}

class Exchange;

/** OrderBook instances are single threaded and must be externally synchronized using mu or lock() */
class OrderBook {
private: // Internal state and helper methods
    SpinLock m_mu; // Mutex for external synchronization // Renamed to m_snake_case
    PriceLevels m_bids = PriceLevels(false); // Bid price levels (descending) // Renamed to m_snake_case
    PriceLevels m_asks = PriceLevels(true); // Ask price levels (ascending) // Renamed to m_snake_case
    OrderBookListener& m_listener; // Listener for trade and order events // Renamed to m_snake_case
    void matchOrders(Order::Side aggressorSide); // Attempts to match orders
    std::map<SessionQuoteId,QuoteOrders> m_quotes; // Map of session/quote ID to active quotes // Renamed to m_snake_case
    
public: // Public interface
    const std::string m_instrument; // The instrument this order book is for // Renamed to m_snake_case
    OrderBook(const std::string &instrument, OrderBookListener& listener) : m_listener(listener), m_instrument(instrument) {} // Renamed to m_snake_case
    ~OrderBook() = default;

    // C++20: Modern insertOrder with explicit error handling and stack overflow protection
    OrderInsertResult insertOrder(
        std::shared_ptr<Order> order,
        std::source_location location = std::source_location::current()
    );
    
    // Legacy overload for backward compatibility (deprecated)
    void insertOrderLegacy(std::shared_ptr<Order> order);
    
    int cancelOrder(std::shared_ptr<Order> order);

    QuoteOrders getQuotes(const std::string& sessionId, const std::string& quoteId, std::function<QuoteOrders()> createOrders);
    void quote(const QuoteOrders& quotes, F bidPrice, int bidQuantity, F askPrice, int askQuantity);

    const Book getBook() const;
    const Order getOrder(std::shared_ptr<Order> order);
    std::vector<std::string> instruments() const {
        return {m_instrument}; // Renamed to m_snake_case
    }
    Guard lock() { // Provides a lock guard for external synchronization
        return Guard(m_mu); // Renamed to m_snake_case
    }
    
    // Modern factory method for creating orders
    template<typename OrderType, typename... Args>
    std::shared_ptr<OrderType> createOrder(Args&&... args) {
        return std::make_shared<OrderType>(std::forward<Args>(args)...);
    }
};