#include "core/orderbook.h"
#include "core/order.h"
#include "core/insert_result.h"
#include "safety/production_safety_inline.h"

#ifdef _WIN32
#include <algorithm>
#define MIN(a,b) std::min(a,b)
#else
#include <sys/param.h>
#endif
#include <functional>
#include <source_location>

// Mutex lock guard macro for consistent locking
#define LOCK_BOOK_MUTEX() std::lock_guard<std::recursive_mutex> lock(m_mu) // Renamed to SCREAMING_SNAKE_CASE

// C++23: Modern insertOrder with explicit error handling and stack overflow protection
OrderInsertResult OrderBook::insertOrder(
    std::shared_ptr<Order> order,
    std::source_location location
) {
    // Stack overflow protection using RAII guard from ProductionSafety
    ProductionSafety::StackGuard stack_guard;
    
    if (!stack_guard.isValid()) {
        auto error = ErrorContext(
            InsertError::StackOverflowProtection,
            "Recursion depth limit exceeded in insertOrder",
            location
        );
        error.recursion_depth = StackProtection::currentDepth();
        return OrderInsertResult(error);
    }
    
    // Validate order pointer
    if (!order) {
        return OrderInsertResult(ErrorContext(
            InsertError::NullOrder,
            "Order pointer is null",
            location
        ));
    }
    
    // Validate order quantity
    if (order->remainingQuantity() <= 0) {
        return OrderInsertResult(ErrorContext(
            InsertError::InvalidQuantity,
            "Invalid order quantity: " + std::to_string(order->remainingQuantity()),
            location
        ));
    }
    
    // Check if already on list (prevent duplicates)
    if (order->isOnList()) {
        return OrderInsertResult(ErrorContext(
            InsertError::OrderAlreadyExists,
            "Order " + std::to_string(order->m_exchangeId) + " is already on a list",
            location
        ));
    }
    
    auto orderList = order->m_side == Order::Side::BUY ? &m_bids : &m_asks;
    
    // Insert the order
    orderList->insertOrder(order);
    m_listener.onOrder(*order);
    
    // Match orders - this may trigger callbacks that could call insertOrder recursively
    // The stack guard above protects against stack overflow here
    matchOrders(order->m_side);
    
    return order->m_exchangeId;
}

void OrderBook::matchOrders(Order::Side aggressorSide) {
    while (!m_bids.empty() && !m_asks.empty()) {
        auto bid = m_bids.front();
        auto ask = m_asks.front();

        if (bid->m_price >= ask->m_price) {
            int qty = MIN(bid->m_remaining, ask->m_remaining);
            F price = MIN(bid->m_price, ask->m_price);

            std::shared_ptr<Order> aggressor = aggressorSide == Order::Side::BUY ? bid : ask;
            std::shared_ptr<Order> opposite = aggressorSide == Order::Side::BUY ? ask : bid;

            bid->fill(qty,price);
            ask->fill(qty,price);

            const Trade trade(price, qty, *aggressor, *opposite);

            if (bid->m_remaining == 0) {
                m_bids.removeOrder(bid);
            }
            if (ask->m_remaining == 0) {
                m_asks.removeOrder(ask);
            }
            m_listener.onOrder(*bid);
            m_listener.onOrder(*ask);
            m_listener.onTrade(trade);
        } else {
            break;
        }
    }
    // cancel remaining market order
    // TODO support convert to limit order
    auto ordersOnSide = aggressorSide == Order::Side::BUY ? &m_bids : &m_asks;
    if (!ordersOnSide->empty()) {
        auto order = ordersOnSide->front();
        if (order && order->isMarket()) {
            order->cancel();
            ordersOnSide->removeOrder(order);
            m_listener.onOrder(*order);
        }
    }
}

QuoteOrders OrderBook::getQuotes(const std::string& sessionId, const std::string& quoteId, std::function<QuoteOrders()> createOrders) {
    auto key = SessionQuoteId(sessionId, quoteId);
    auto it = m_quotes.find(key);
    if (it == m_quotes.end()) {
        return m_quotes[key] = createOrders();
    } else {
        return it->second;
    }
}

void OrderBook::quote(const QuoteOrders& quotes, F bidPrice, int bidQuantity, F askPrice, int askQuantity) {
    auto bid = quotes.m_bid;
    auto ask = quotes.m_ask;
    if(bid->isOnList()) {
        m_bids.removeOrder(bid);
    }
    if(ask->isOnList()) {
        m_asks.removeOrder(ask);
    }
    if (bidQuantity != 0) {
        bid->m_price = bidPrice;
        bid->m_quantity = bidQuantity;
        bid->m_remaining = bidQuantity;
        bid->m_filled = 0;
        m_bids.insertOrder(bid);
        matchOrders(Order::Side::BUY);
    }
    if (askQuantity != 0) {
        ask->m_price = askPrice;
        ask->m_quantity = askQuantity;
        ask->m_remaining = askQuantity;
        ask->m_filled = 0;
        m_asks.insertOrder(ask);
        matchOrders(Order::Side::SELL);
    }
}

int OrderBook::cancelOrder(std::shared_ptr<Order> order) {
    // Add null pointer check
    if (!order) {
        return -1;
    }
    
    // Add bounds checking for remaining quantity
    if (order->m_remaining > 0) {
        order->cancel();
        auto ordersOnSide = order->m_side == Order::Side::BUY ? &m_bids : &m_asks;
        
        // Add safety check before removal
        if (ordersOnSide && order->isOnList()) {
            ordersOnSide->removeOrder(order);
            m_listener.onOrder(*order);
            return 0;
        } else {
            // Order not found in lists or not on list
            return -1;
        }
    } else {
        return -1;
    }
}

const Book OrderBook::getBook() const {
    Book orderBookSnapshot;
    auto snap = [](const PriceLevels& src, std::vector<BookLevel>& dst, std::vector<ExchangeId>& oids) {
        auto fn = [&](const OrderList* orders) {
            int quantity(0);
            for (auto itr = orders->begin(); itr != orders->end(); ++itr) {
                quantity = quantity + (*itr)->remainingQuantity();
                oids.push_back((*itr)->m_exchangeId);
            }
            dst.push_back({orders->m_price, quantity});
        };
        src.forEach(fn);
    };
    orderBookSnapshot.m_bids.reserve(m_bids.size());
    orderBookSnapshot.m_asks.reserve(m_asks.size());
    snap(m_bids, orderBookSnapshot.m_bids, orderBookSnapshot.m_bidOrderIds);
    snap(m_asks, orderBookSnapshot.m_asks, orderBookSnapshot.m_askOrderIds);
    return orderBookSnapshot;
}

const Order OrderBook::getOrder(std::shared_ptr<Order> order) {
    if (!order) {
        throw std::invalid_argument("Order cannot be null");
    }
    return *order;
}
