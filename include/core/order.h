#pragma once

#include <chrono>
#include <cfloat>
#include <memory>
#include <atomic>
#include <cstdint>
#include <string>

#include "fixed.h"
#include "string_interner.h"
#include "semantic_types.h"
#include "constants.h"

// ============================================================================
// TYPE ALIASES
// ============================================================================

/** Semantic alias for price representation using fixed-point arithmetic */
using Price = Fixed<7>;

/** Legacy alias for backward compatibility */
typedef Fixed<7> F;

using StringId = orderbook::StringInterner::StringId;
using orderbook::StringInterner;
using orderbook::g_globalStringInterner;

/** Timestamp representing when an order was submitted */
using SubmissionTime = Timestamp;

/** Helper function for getting current time */
inline Timestamp CurrentTimestamp() {
    return std::chrono::system_clock::now();
}

#define epoch() CurrentTimestamp()

class Exchange;
class OrderList; // Renamed to PascalCase
class OrderMap;

struct Order;

class Node {
friend class OrderList; // OrderList needs access to Node's members
friend struct Order;
private:
    std::shared_ptr<Node> m_prev = nullptr; // Renamed to m_snake_case
    std::shared_ptr<Node> m_next = nullptr; // Renamed to m_snake_case
    /** order is non-null if enqueued on an OrderList */ // Renamed to PascalCase
    std::weak_ptr<Order> m_order;  // weak_ptr to avoid circular references // Renamed to m_snake_case
};

/**
 * Memory-optimized Order structure
 * - Uses StringInterner for strings (4 bytes each instead of 32+)
 * - Compact Side enum (1 byte)
 * - Aligned for cache efficiency
 */
struct alignas(64) Order {
public:
    enum class Side : uint8_t { BUY = 0, SELL = 1 };

    friend class OrderBook;
    friend class OrderList;
    friend class OrderMap;
    friend class Exchange;
    friend class TestOrder;
    template<typename> friend class PointerPriceLevels;
    template<typename> friend class StructPriceLevels;
    template<typename> friend class MapPriceLevels;
    template<typename> friend class MapPtrPriceLevels;

    // Public factory method - creates shared_ptr for consistent memory management
    static std::shared_ptr<Order> create(
        SessionIdView sessionId,
        OrderIdStrView orderId,
        InstrumentSymbolView instrument,
        Price price,
        Quantity quantity,
        Order::Side side,
        ExchangeId exchangeId
    ) {
        return std::shared_ptr<Order>(new Order(sessionId, orderId, instrument, price, quantity, side, exchangeId));
    }

private:
    /** Used to enqueue Order in OrderMap - raw pointer for lock-free list */
    std::atomic<Order*> m_nextPtr{nullptr}; // Renamed to m_snake_case
    
    /** Holds Node in OrderList for quick removal */
    std::shared_ptr<Node> m_node; // Renamed to m_snake_case
    
    /** Timestamp when order was submitted to the exchange */
    const SubmissionTime m_timeSubmitted; // Renamed to m_snake_case

    // Compact numeric fields
    Quantity m_remaining = 0; // Remaining quantity of the order // Renamed to m_snake_case
    Quantity m_filled = 0; // Quantity already filled // Renamed to m_snake_case
    Quantity m_quantity = 0; // Original quantity of the order // Renamed to m_snake_case
    Quantity m_cumulativeQuantity = 0; // Cumulative quantity filled // Renamed to m_snake_case
    
    Price m_price; // Limit price of the order // Renamed to m_snake_case
    Price m_averagePrice = 0; // Average filled price // Renamed to m_snake_case

    // String interning IDs (4 bytes each vs 32+ bytes for std::string)
    StringInterner::StringId m_sessionId = StringInterner::INVALID_ID; // Interned session ID // Renamed to m_snake_case
    StringInterner::StringId m_orderId = StringInterner::INVALID_ID; // Interned order ID // Renamed to m_snake_case
    StringInterner::StringId m_instrumentId = StringInterner::INVALID_ID; // Interned instrument ID // Renamed to m_snake_case
    
    // Original numeric identifier for orderId (if numeric ID is used)
    ExchangeId m_orderIdNum = 0; // Renamed to m_snake_case

    void fill(Quantity quantity, Price price) { 
        m_remaining -= quantity; // Renamed to m_snake_case
        m_filled += quantity;  // Renamed to m_snake_case
        m_averagePrice = (m_averagePrice * m_cumulativeQuantity + price * quantity) / (m_cumulativeQuantity + quantity); // Renamed to m_snake_case
        m_cumulativeQuantity += quantity; // Renamed to m_snake_case
    }
    
    void cancel() { m_remaining = 0; } // Sets remaining quantity to 0 // Renamed to m_snake_case
    
    bool isMarket() const { // Renamed to camelCase
        return m_price == kMarketBuyPrice || m_price == kMarketSellPrice; // Renamed to m_snake_case, kPascalCase
    }

public:
    // Public accessor methods
    std::string sessionId() const { 
        return std::string(g_globalStringInterner().get(m_sessionId)); // Renamed to g_camelCase, m_snake_case
    }
    
    std::string orderId() const { 
        if (m_orderId != StringInterner::INVALID_ID) { // Renamed to m_snake_case
            return std::string(g_globalStringInterner().get(m_orderId)); // Renamed to g_camelCase, m_snake_case
        }
        return std::to_string(m_orderIdNum); // Renamed to m_snake_case
    }
    
    std::string instrument() const { 
        return std::string(g_globalStringInterner().get(m_instrumentId)); // Renamed to g_camelCase, m_snake_case
    }
    
    ExchangeId orderIdNum() const { return m_orderIdNum; } // Renamed to m_snake_case
    
    const ExchangeId m_exchangeId; // Unique ID assigned by the exchange // Renamed to m_snake_case
    const Side m_side; // Side of the order (BUY/SELL) // Renamed to m_snake_case

    Price price() const { return m_price; } // Returns the limit price // Renamed to m_snake_case
    Quantity quantity() const { return m_quantity; } // Returns the original quantity // Renamed to m_snake_case

    bool isOnList() const { // Renamed to camelCase
        return !m_node->m_order.expired(); // Renamed to m_snake_case
    }

    Quantity remainingQuantity() const { return m_remaining; } // Remaining quantity to be filled // Renamed to m_snake_case
    Quantity filledQuantity() const { return m_filled; } // Quantity already filled // Renamed to m_snake_case
    Quantity cumulativeQuantity() const { return m_cumulativeQuantity; } // Total quantity filled over multiple trades // Renamed to m_snake_case
    Price averagePrice() const { return m_averagePrice; } // Average price of filled quantity // Renamed to m_snake_case
    
    bool isCancelled() const { // Renamed to camelCase
        return m_remaining == 0 && m_filled != m_quantity; // Renamed to m_snake_case
    }
    
    bool isFilled() const { // Renamed to camelCase
        return m_remaining == 0 && m_filled == m_quantity; // Renamed to m_snake_case
    }
    
    bool isPartiallyFilled() const { // Renamed to camelCase
        return m_remaining == 0 && m_filled > 0; // Renamed to m_snake_case
    }
    
    bool isActive() const { // Renamed to camelCase
        return m_remaining > 0; // Renamed to m_snake_case
    }
    
    bool m_isQuote = false; // Flag indicating if this order is part of a quote // Renamed to m_snake_case

    // Copy constructor - atomic next is not copied (initialized to nullptr)
    Order(const Order& other)
        : m_nextPtr(nullptr), // Renamed to m_snake_case
          m_node(other.m_node), // Renamed to m_snake_case
          m_timeSubmitted(other.m_timeSubmitted), // Renamed to m_snake_case
          m_remaining(other.m_remaining), // Renamed to m_snake_case
          m_filled(other.m_filled), // Renamed to m_snake_case
          m_quantity(other.m_quantity), // Renamed to m_snake_case
          m_cumulativeQuantity(other.m_cumulativeQuantity), // Renamed to m_snake_case
          m_price(other.m_price), // Renamed to m_snake_case
          m_averagePrice(other.m_averagePrice), // Renamed to m_snake_case
          m_sessionId(other.m_sessionId), // Renamed to m_snake_case
          m_orderId(other.m_orderId), // Renamed to m_snake_case
          m_instrumentId(other.m_instrumentId), // Renamed to m_snake_case
          m_orderIdNum(other.m_orderIdNum), // Renamed to m_snake_case
          m_exchangeId(other.m_exchangeId), // Renamed to m_snake_case
          m_side(other.m_side), // Renamed to m_snake_case
          m_isQuote(other.m_isQuote) {} // Renamed to m_snake_case

protected:
    // Protected to allow testcase and friend classes
    Order(SessionIdView sessionId, OrderIdStrView orderId, 
          InstrumentSymbolView instrument, Price price, Quantity quantity, 
          Order::Side side, ExchangeId exchangeId) 
        : m_node(std::make_shared<Node>()), // Renamed to m_snake_case
          m_timeSubmitted(epoch()), // Renamed to m_snake_case
          m_remaining(quantity), // Renamed to m_snake_case
          m_quantity(quantity), // Renamed to m_snake_case
          m_price(price), // Renamed to m_snake_case
          m_sessionId(g_globalStringInterner().intern(sessionId)), // Renamed to m_snake_case, g_camelCase
          m_instrumentId(g_globalStringInterner().intern(instrument)), // Renamed to m_snake_case, g_camelCase
          m_exchangeId(exchangeId), // Renamed to m_snake_case
          m_side(side) // Renamed to m_snake_case
    {
        // Try to parse orderId as number for efficiency
        try {
            m_orderIdNum = std::stoll(std::string(orderId)); // Renamed to m_snake_case
        } catch (...) {
            m_orderId = g_globalStringInterner().intern(orderId); // Renamed to m_snake_case, g_camelCase
            m_orderIdNum = 0; // Renamed to m_snake_case
        }
    }
};
