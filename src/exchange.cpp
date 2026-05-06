#include "core/exchange.h"
#include "core/orderbook.h"
#include "core/insert_result.h"
#include <stdexcept>
#include <string>
#include <iostream>

// Mutex lock guard macro for consistent locking // Renamed to SCREAMING_SNAKE_CASE
#define LOCK_EXCHANGE_MUTEX() std::lock_guard<std::mutex> lock(m_mu)

// Get order details by exchange ID with thread-safe access
std::optional<Order> Exchange::getOrder(ExchangeId exchangeId) const {
    auto order = m_allOrders.get(exchangeId);
    if (!order) return std::nullopt;
    
    auto orderBook = m_books.getOrderBook(order->instrument());
    if (!orderBook) return std::nullopt;
    
    auto bookGuard = orderBook->lock();
    return std::optional<Order>(orderBook->getOrder(order));
}

// Get order book snapshot for specified instrument
std::optional<Book> Exchange::getBook(std::string_view instrument) const {
    auto orderBook = m_books.getOrderBook(std::string(instrument));
    if (!orderBook) return std::nullopt;
    
    auto bookGuard = orderBook->lock();
    return orderBook->getBook();
}

// Cancel order with session validation and thread safety
CancelResult Exchange::cancelOrder(ExchangeId exchangeId, SessionIdView sessionId) {
    auto order = m_allOrders.get(exchangeId);
    if (!order) {
        return false;
    }
    
    if (order->sessionId() != sessionId) {
        return false;
    }
    
    auto orderBook = m_books.getOrderBook(order->instrument());
    if (!orderBook) {
        return false;
    }

    auto bookGuard = orderBook->lock();
    auto result = orderBook->cancelOrder(order);
    return result == 0;
}

OrderResult Exchange::insertOrderInternal(
    SessionIdView sessionId,
    std::string_view instrument,
    F price,
    int quantity,
    Order::Side side,
    OrderIdStrView orderId
) {
    try {
        auto orderBook = m_books.getOrCreate(std::string(instrument), *this);
        if (!orderBook) {
            return std::nullopt;
        }
        
        auto bookGuard = orderBook->lock();
        ExchangeId id = nextId();
        
        auto order = Order::create(
            std::string(sessionId),
            std::string(orderId),
            orderBook->m_instrument,
            price,
            quantity,
            side,
            id
        );
        
        m_allOrders.add(order);
        
        // C++20: Use new insertOrder with explicit result checking
        auto result = orderBook->insertOrder(order);
        
        // Mandatory result checking with configurable error handling
        if (!result.hasValue()) {
            const auto& error = result.error();
            
            // Log error details
            std::cerr << "[EXCHANGE ERROR] Order insertion failed: " << error.toString() << "\n";
            
            // Handle specific error types with configurable strategies
            switch (error.code) {
                case InsertError::StackOverflowProtection:
                    // Critical: Remove order from allOrders to prevent orphaned orders
                    m_allOrders.remove(order->m_exchangeId);
                    // Could implement retry with backoff here
                    return std::nullopt;
                    
                case InsertError::OrderAlreadyExists:
                    // Order already exists - this is idempotent, return existing ID
                    return id;
                    
                case InsertError::NullOrder:
                case InsertError::InvalidQuantity:
                    // Validation errors - remove from allOrders
                    m_allOrders.remove(order->m_exchangeId);
                    return std::nullopt;
                    
                default:
                    // Unknown error - safe fallback
                    m_allOrders.remove(order->m_exchangeId);
                    return std::nullopt;
            }
        }
        
        return id;
    } catch (const std::exception& e) {
        std::cerr << "[EXCHANGE ERROR] Exception in insertOrder: " << e.what() << "\n";
        return std::nullopt;
    }
}

OrderResult Exchange::placeBuyOrder(
    std::string_view sessionId,
    std::string_view instrument,
    F price,
    int quantity,
    std::string_view orderId
) {
    return insertOrderInternal(sessionId, instrument, price, quantity, Order::Side::BUY, orderId);
}

OrderResult Exchange::placeSellOrder(
    std::string_view sessionId,
    std::string_view instrument,
    F price,
    int quantity,
    std::string_view orderId
) {
    return insertOrderInternal(sessionId, instrument, price, quantity, Order::Side::SELL, orderId);
}

void Exchange::quote(
    std::string_view sessionId,
    std::string_view instrument,
    F bidPrice,
    int bidQuantity,
    F askPrice,
    int askQuantity,
    std::string_view quoteId
) {
    auto orderBook = m_books.getOrCreate(std::string(instrument), *this);
    auto bookGuard = orderBook->lock();
    
    auto orders = orderBook->getQuotes(
        std::string(sessionId),
        std::string(quoteId),
        [&]() -> QuoteOrders {
            QuoteOrders result;
            
            if (bidQuantity > 0) {
                result.m_bid = Order::create(
                    std::string(sessionId),
                    std::string(quoteId),
                    orderBook->m_instrument,
                    bidPrice,
                    bidQuantity,
                    Order::Side::BUY,
                    nextId()
                );
                m_allOrders.add(result.m_bid);
            }
            
            if (askQuantity > 0) {
                result.m_ask = Order::create(
                    std::string(sessionId),
                    std::string(quoteId),
                    orderBook->m_instrument,
                    askPrice,
                    askQuantity,
                    Order::Side::SELL,
                    nextId()
                );
                m_allOrders.add(result.m_ask);
            }
            
            return result;
        }
    );
    
    orderBook->quote(orders, bidPrice, bidQuantity, askPrice, askQuantity);
}

// C++26: Modern atomic ID generation // Renamed to camelCase
ExchangeId Exchange::nextId() {
    static std::atomic<ExchangeId> id = 0;
    return ++id;
}
