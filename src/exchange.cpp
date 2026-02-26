#include "core/exchange.h"
#include "core/orderbook.h"
#include <stdexcept>
#include <string>

// Mutex lock guard macro for consistent locking
#define LOCK_EXCHANGE() std::lock_guard<std::mutex> lock(mu)

// Get order details by exchange ID with thread-safe access
std::optional<Order> Exchange::getOrder(long exchangeId) const {
    auto order = allOrders.get(exchangeId);
    if (!order) return std::nullopt;
    
    auto book = books.get(order->instrument);
    if (!book) return std::nullopt;
    
    auto bookGuard = book->lock();
    return book->getOrder(order);
}

// Get order book snapshot for specified instrument
std::optional<Book> Exchange::book(std::string_view instrument) const {
    auto book = books.get(std::string(instrument));
    if (!book) return std::nullopt;
    
    auto bookGuard = book->lock();
    return book->book();
}

// Cancel order with session validation and thread safety
CancelResult Exchange::cancel(long exchangeId, std::string_view sessionId) {
    auto order = allOrders.get(exchangeId);
    if (!order) {
        return false;
    }
    
    if (order->sessionId() != sessionId) {
        return false;
    }
    
    auto book = books.get(order->instrument);
    if (!book) {
        return false;
    }

    auto bookGuard = book->lock();
    auto result = book->cancelOrder(order);
    return result == 0;
}

OrderResult Exchange::insertOrder(
    std::string_view sessionId,
    std::string_view instrument,
    F price,
    int quantity,
    Order::Side side,
    std::string_view orderId
) {
    try {
        auto book = books.getOrCreate(std::string(instrument), *this);
        if (!book) {
            return std::nullopt;
        }
        
        auto bookGuard = book->lock();
        long id = nextID();
        
        auto order = Order::create(
            std::string(sessionId),
            std::string(orderId),
            book->instrument,
            price,
            quantity,
            side,
            id
        );
        
        allOrders.add(order);
        book->insertOrder(order);
        return id;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

OrderResult Exchange::buy(
    std::string_view sessionId,
    std::string_view instrument,
    F price,
    int quantity,
    std::string_view orderId
) {
    return insertOrder(sessionId, instrument, price, quantity, Order::BUY, orderId);
}

OrderResult Exchange::sell(
    std::string_view sessionId,
    std::string_view instrument,
    F price,
    int quantity,
    std::string_view orderId
) {
    return insertOrder(sessionId, instrument, price, quantity, Order::SELL, orderId);
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
    auto book = books.getOrCreate(std::string(instrument), *this);
    auto bookGuard = book->lock();
    
    auto orders = book->getQuotes(
        std::string(sessionId),
        std::string(quoteId),
        [&]() -> QuoteOrders {
            QuoteOrders result;
            
            if (bidQuantity > 0) {
                result.bid = Order::create(
                    std::string(sessionId),
                    std::string(quoteId),
                    book->instrument,
                    bidPrice,
                    bidQuantity,
                    Order::BUY,
                    nextID()
                );
                allOrders.add(result.bid);
            }
            
            if (askQuantity > 0) {
                result.ask = Order::create(
                    std::string(sessionId),
                    std::string(quoteId),
                    book->instrument,
                    askPrice,
                    askQuantity,
                    Order::SELL,
                    nextID()
                );
                allOrders.add(result.ask);
            }
            
            return result;
        }
    );
    
    book->quote(orders, bidPrice, bidQuantity, askPrice, askQuantity);
}

// C++26: Modern atomic ID generation
long Exchange::nextID() {
    static std::atomic<long> id = 0;
    return ++id;
}
