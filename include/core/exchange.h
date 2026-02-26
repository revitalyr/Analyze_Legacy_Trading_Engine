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

struct ExchangeListener {
    /** callback when order properties change */
    virtual void onOrder(const Order& order) {}
    /** callback when trade occurs */
    virtual void onTrade(const Trade& trade) {}
};

static ExchangeListener dummy;

// Simplified result types for compatibility
using OrderResult = std::optional<long>;
using CancelResult = bool;

class Exchange : OrderBookListener {
public:
    Exchange() : listener(dummy) {}
    explicit Exchange(ExchangeListener& listener) : listener(listener) {}
    
    // Simplified API using std::optional for now
    OrderResult buy(
        std::string_view sessionId,
        std::string_view instrument,
        F price,
        int quantity,
        std::string_view orderId = ""
    );
    
    OrderResult marketBuy(
        std::string_view sessionId,
        std::string_view instrument,
        int quantity,
        std::string_view orderId = ""
    ) {
        return buy(sessionId, instrument, F(DBL_MAX), quantity, orderId);
    }
    
    OrderResult sell(
        std::string_view sessionId,
        std::string_view instrument,
        F price,
        int quantity,
        std::string_view orderId = ""
    );
    
    OrderResult marketSell(
        std::string_view sessionId,
        std::string_view instrument,
        int quantity,
        std::string_view orderId = ""
    ) {
        return sell(sessionId, instrument, F(-DBL_MAX), quantity, orderId);
    }
    
    void quote(
        std::string_view sessionId,
        std::string_view instrument,
        F bidPrice,
        int bidQuantity,
        F askPrice,
        int askQuantity,
        std::string_view quoteId
    );
    
    // Simplified error handling
    CancelResult cancel(long exchangeId, std::string_view sessionId);
    
    std::optional<Book> book(std::string_view instrument) const;
    std::optional<Order> getOrder(long exchangeId) const;
    
    // Modern range-based API
    auto getAllOrders() const {
        return allOrders.all() | std::views::transform([](const std::shared_ptr<const Order>& order) { return order; });
    }
    
    auto getInstruments() const {
        return books.instruments() | std::views::transform([](const std::string& instrument) { return instrument; });
    }
    
    void onOrder(const Order& order) override {
        listener.onOrder(order);
    }
    void onTrade(const Trade& trade) override {
        listener.onTrade(trade);
    }
    Guard lock() {
        return Guard(mu);
    }
    
    // Legacy API for compatibility
    std::vector<std::string> instruments() {
        return books.instruments();
    }
    
    std::vector<std::shared_ptr<const Order>> orders() {
        return allOrders.all();
    }
    
private:
    BookMap books;
    OrderMap allOrders;
    SpinLock mu;
    
    // C++26: Modern atomic ID generation
    long nextID();
    
    OrderResult insertOrder(
        std::string_view sessionId,
        std::string_view instrument,
        F price,
        int quantity,
        Order::Side side,
        std::string_view orderId
    );
    
    ExchangeListener& listener;
};