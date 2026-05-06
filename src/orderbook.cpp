#include "core/orderbook.h"
#include "core/order.h"
#include "core/insert_result.h"
#include "safety/production_safety_inline.h"

#include <algorithm>
#include <functional>
#include <source_location>
#include "core/engine_constants.h"

#define LOCK_BOOK_MUTEX() std::lock_guard<std::recursive_mutex> lock(m_mu)

OrderInsertResult OrderBook::insertOrder(
    std::shared_ptr<Order> order,
    std::source_location location
) {
    // Prevents stack exhaustion during highly recursive matching/callback scenarios
    ProductionSafety::StackGuard stack_guard;
    
    if (!stack_guard.isValid()) {
        auto error = ErrorContext(
            InsertError::StackOverflowProtection,
            EngineConstants::kRecursionDepthExceeded,
            location
        );
        error.recursion_depth = StackProtection::currentDepth();
        return OrderInsertResult(error);
    }
    
    // Validate order pointer
    if (!order) {
        return OrderInsertResult(ErrorContext(
            InsertError::NullOrder,
            EngineConstants::kOrderPointerNull,
            location
        ));
    }
    
    // Validate order quantity
    if (order->remainingQuantity() <= 0) {
        return OrderInsertResult(ErrorContext(
            InsertError::InvalidQuantity,
            std::string(EngineConstants::kInvalidOrderQuantity) + std::to_string(order->remainingQuantity().count()),
            location
        ));
    }
    
    // Check if already on list (prevent duplicates)
    if (order->isOnList()) {
        return OrderInsertResult(ErrorContext(
            InsertError::OrderAlreadyExists,
            std::string(EngineConstants::kOrderAlreadyOnList) + " " + std::to_string(order->m_exchangeId),
            location
        ));
    }
    
    auto orderList = order->m_side == Order::Side::BUY ? &m_bids : &m_asks;
    
    // Insert the order
    orderList->insertOrder(order);
    m_listener.onOrder(*order);
    
    // Perform immediate execution against opposite side
    matchOrders(order->m_side);
    
    return order->m_exchangeId;
}

void OrderBook::matchOrders(Order::Side aggressorSide) {
    while (!m_bids.empty() && !m_asks.empty()) {
        auto bid = m_bids.front();
        auto ask = m_asks.front();

        if (bid->isMarket() || ask->isMarket() || bid->m_price >= ask->m_price) {
            Quantity qty = std::min(bid->m_remaining, ask->m_remaining);
            // Trade at the price of the resting order
            Price price = (aggressorSide == Order::Side::BUY) ? ask->m_price : bid->m_price;

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
    // Cancel any unexecuted volume for market orders
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

void OrderBook::quote(const QuoteOrders& quotes, Price bidPrice, Quantity bidQuantity, Price askPrice, Quantity askQuantity) {
    auto bid = quotes.m_bid;
    auto ask = quotes.m_ask;
    if(bid->isOnList()) {
        m_bids.removeOrder(bid);
    }
    if(ask->isOnList()) {
        m_asks.removeOrder(ask);
    }
    if (bidQuantity != Quantity(0)) {
        bid->m_price = bidPrice;
        bid->m_quantity = bidQuantity;
        bid->m_remaining = bidQuantity;
        bid->m_filled = Quantity(0);
        m_bids.insertOrder(bid);
        matchOrders(Order::Side::BUY);
    }
    if (askQuantity != Quantity(0)) {
        ask->m_price = askPrice;
        ask->m_quantity = askQuantity;
        ask->m_remaining = askQuantity;
        ask->m_filled = Quantity(0);
        m_asks.insertOrder(ask);
        matchOrders(Order::Side::SELL);
    }
}

int OrderBook::cancelOrder(std::shared_ptr<Order> order) {
    if (!order) {
        return -1;
    }
    
    if (order && order->m_remaining > Quantity(0)) {
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
            Quantity quantity(0);
            for (auto itr = orders->begin(); itr != orders->end(); ++itr) {
                auto order = *itr;
                if (order) {
                    quantity = quantity + order->remainingQuantity();
                    oids.push_back(order->m_exchangeId);
                }
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
        throw std::invalid_argument(std::string(EngineConstants::kOrderCannotBeNull));
    }
    return *order;
}
