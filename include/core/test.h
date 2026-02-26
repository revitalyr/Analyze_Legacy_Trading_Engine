#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <ranges>

#include "exchange.h"
#include "orderbook.h"

// Simplified result types for compatibility
using OrderResult = std::optional<long>;

// C++26: Modern test utilities with smart pointers
class TestExchange : public Exchange {
public:
    TestExchange() : Exchange(listener) {}
    
    // Simplified API using std::optional
    OrderResult buy(std::string_view sessionId, F price, int quantity, std::string_view orderId = "") {
        return Exchange::buy(sessionId, "SYM1", price, quantity, orderId);
    }
    
    OrderResult sell(std::string_view sessionId, F price, int quantity, std::string_view orderId = "") {
        return Exchange::sell(sessionId, "SYM1", price, quantity, orderId);
    }
    
    OrderResult marketBuy(std::string_view sessionId, int quantity, std::string_view orderId = "") {
        return Exchange::marketBuy(sessionId, "SYM1", quantity, orderId);
    }
    
    OrderResult marketSell(std::string_view sessionId, int quantity, std::string_view orderId = "") {
        return Exchange::marketSell(sessionId, "SYM1", quantity, orderId);
    }
    
    // C++26: Modern range-based queries
    auto getOrdersBySide(Order::Side side) const {
        return getAllOrders() 
            | std::views::filter([side](const auto& order) { return order->side == side; });
    }
    
    auto getOrdersBySession(std::string_view sessionId) const {
        return getAllOrders()
            | std::views::filter([sessionId](const auto& order) { return order->sessionId() == sessionId; });
    }
    
private:
    ExchangeListener listener;
};

// C++26: Modern TestOrder with smart pointers and factory methods
class TestOrder : public Order {
public:
    // C++26: Factory methods returning smart pointers
    static std::shared_ptr<TestOrder> create(long id, F price, int quantity, Order::Side side) {
        return std::make_shared<TestOrder>(id, price, quantity, side);
    }
    
    static std::shared_ptr<TestOrder> create(
        std::string_view sessionId,
        std::string_view orderId,
        F price,
        int quantity,
        Order::Side side,
        long exchangeId
    ) {
        return std::make_shared<TestOrder>(
            std::string(sessionId),
            std::string(orderId),
            price,
            quantity,
            side,
            exchangeId
        );
    }
    
    // Legacy constructors for compatibility
    TestOrder(long id, F price, int quantity, Order::Side side) 
        : Order("session", std::to_string(id), "SYM1", price, quantity, side, id) {}
    
    TestOrder(
        std::string_view orderId,
        long id,
        F price,
        int quantity,
        Order::Side side
    ) : Order("session", std::string(orderId), "SYM1", price, quantity, side, id) {}
    
    TestOrder(
        std::string_view sessionId,
        std::string_view orderId,
        F price,
        int quantity,
        Order::Side side,
        long exchangeId
    ) : Order(std::string(sessionId), std::string(orderId), "SYM1", price, quantity, side, exchangeId) {}
};

// C++26: Modern test utilities
namespace TestUtils {
    // Create a test order book with smart pointers
    inline std::unique_ptr<OrderBook> createTestOrderBook(OrderBookListener& listener) {
        return std::make_unique<OrderBook>("TEST", listener);
    }
    
    // Helper to validate order book state
    inline bool validateOrderBook(const OrderBook& book) {
        auto book_snapshot = book.book();
        return !book_snapshot.bids.empty() || !book_snapshot.asks.empty();
    }
    
    // C++26: Range-based order validation
    inline auto validateOrders(std::ranges::range auto orders) {
        return std::ranges::all_of(orders, [](const auto& order) {
            return order && order->remaining > 0;
        });
    }
}

// C++26: Constants for testing
inline constexpr std::string_view dummy_instrument = "SYM1";
