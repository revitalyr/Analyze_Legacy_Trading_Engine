#pragma once

#include <string>
#include <optional>
#include <ranges>
#include <memory>
#include <vector>

#include "order.h"
#include "orderbook.h"
#include "bookmap.h"
#include "spinlock.h"
#include "ordermap.h"
#include "semantic_types.h"
#include "constants.h"

struct ExchangeListener {
    /** callback when order properties change */
    virtual void onOrder(const Order& ) {}
    /** Callback when a trade occurs */
    virtual void onTrade(const Trade& ) {}
};

inline ExchangeListener g_dummyListener; // A default listener for convenience

// Simplified result types for compatibility
using OrderResult = std::optional<ExchangeId>;
using CancelResult = bool;

class Exchange : OrderBookListener {
public: // Public interface for the exchange
    Exchange() : m_listener(g_dummyListener) {}
    explicit Exchange(ExchangeListener& listener) : m_listener(listener) {}
    
    // Simplified API using std::optional for now
    OrderResult placeBuyOrder(
        SessionIdView sessionId,
        InstrumentSymbolView instrument,
        Price price,
        Quantity quantity,
        OrderIdStrView orderId = ""
    );
    
    OrderResult placeMarketBuyOrder(
        SessionIdView sessionId,
        InstrumentSymbolView instrument,
        Quantity quantity,
        OrderIdStrView orderId = ""
    ) {
        return placeBuyOrder(sessionId, instrument, Price(kMarketBuyPrice), quantity, orderId); // Renamed to kPascalCase
    }
    
    OrderResult placeSellOrder(
        SessionIdView sessionId,
        InstrumentSymbolView instrument,
        Price price,
        Quantity quantity,
        OrderIdStrView orderId = ""
    );
    
    OrderResult placeMarketSellOrder(
        SessionIdView sessionId,
        InstrumentSymbolView instrument,
        Quantity quantity,
        OrderIdStrView orderId = ""
    ) {
        return placeSellOrder(sessionId, instrument, Price(kMarketSellPrice), quantity, orderId); // Renamed to kPascalCase
    }
    
    void quote(
        SessionIdView sessionId,
        InstrumentSymbolView instrument,
        Price bidPrice,
        Quantity bidQuantity,
        Price askPrice,
        Quantity askQuantity,
        QuoteIdView quoteId
    );
    
    // Simplified error handling
    CancelResult cancelOrder(ExchangeId exchangeId, SessionIdView sessionId);
    
    std::optional<Book> getBook(InstrumentSymbolView instrument) const;
    std::optional<Order> getOrder(ExchangeId exchangeId) const;
    
    // Modern range-based API
    auto getAllOrders() const {
        return m_allOrders.all() | std::views::transform([](const std::shared_ptr<const Order>& order) { return order; }); // Renamed to m_snake_case
    }
    
    auto getInstruments() const {
        return m_books.instruments() | std::views::transform([](const InstrumentSymbol& instrument) { return instrument; }); // Renamed to m_snake_case
    }
    
    void onOrder(const Order& order) override {
        m_listener.onOrder(order); // Renamed to m_snake_case
    }
    void onTrade(const Trade& trade) override {
        m_listener.onTrade(trade); // Renamed to m_snake_case
    }
    Guard lock() { // Provides a lock guard for external synchronization
        return Guard(m_mu); // Renamed to m_snake_case
    }
    
    // Legacy API for compatibility
    std::vector<std::string> instruments() {
        return m_books.instruments(); // Renamed to m_snake_case
    }
    
    std::vector<std::shared_ptr<const Order>> orders() {
        return m_allOrders.all(); // Renamed to m_snake_case
    }
    
private: // Internal state and helper methods
    BookMap m_books; // Map of instrument symbols to order books
    OrderMap m_allOrders; // Map of all active orders by ExchangeId
    SpinLock m_mu; // Mutex for internal synchronization
    
    // C++26: Modern atomic ID generation
    ExchangeId nextId();
    
    OrderResult insertOrderInternal(
        SessionIdView sessionId,
        InstrumentSymbolView instrument,
        Price price,
        Quantity quantity,
        Order::Side side,
        OrderIdStrView orderId
    );
    
    ExchangeListener& m_listener; // Reference to the external listener
};