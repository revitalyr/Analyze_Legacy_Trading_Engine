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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier
#endif
/**
 * @struct Order
 * @brief Highly optimized order representation designed for cache-line alignment.
 * 
 * Members are ordered to minimize padding and group frequently accessed hot data.
 * Uses StringInterning to reduce session and instrument IDs to 32-bit integers.
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
    std::shared_ptr<Node> m_node; 
    
    std::atomic<Order*> m_nextPtr{nullptr};
    
    const SubmissionTime m_timeSubmitted;
    ExchangeId m_orderIdNum = ExchangeId(0);

    Price m_price;
    Price m_averagePrice = Price(0);
    Quantity m_remaining = Quantity(0);
    Quantity m_filled = Quantity(0);
    Quantity m_quantity = Quantity(0);
    Quantity m_cumulativeQuantity = Quantity(0);

    StringInterner::StringId m_sessionId = StringInterner::INVALID_ID;
    StringInterner::StringId m_orderId = StringInterner::INVALID_ID;
    StringInterner::StringId m_instrumentId = StringInterner::INVALID_ID;

public:
    const ExchangeId m_exchangeId;
    const Side m_side;
    bool m_isQuote = false;
private:
    void fill(Quantity quantity, Price price) { 
        m_remaining -= quantity; // Renamed to m_snake_case
        m_filled += quantity;  // Renamed to m_snake_case
        m_averagePrice = (m_averagePrice * m_cumulativeQuantity + price * quantity) / (m_cumulativeQuantity + quantity); // Renamed to m_snake_case
        m_cumulativeQuantity += quantity; // Renamed to m_snake_case
    }
    
    void cancel() { m_remaining = Quantity(0); } // Sets remaining quantity to 0 // Renamed to m_snake_case
    
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
        return m_remaining == Quantity(0) && m_filled != m_quantity; // Renamed to m_snake_case
    }
    
    bool isFilled() const { // Renamed to camelCase
        return m_remaining == Quantity(0) && m_filled == m_quantity; // Renamed to m_snake_case
    }
    
    bool isPartiallyFilled() const { // Renamed to camelCase
        return m_remaining == Quantity(0) && m_filled > Quantity(0); // Renamed to m_snake_case
    }
    
    bool isActive() const { // Renamed to camelCase
        return m_remaining > 0; // Renamed to m_snake_case
    }

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
        try { // Renamed to camelCase
            m_orderIdNum = std::stoll(std::string(orderId)); // Renamed to m_snake_case
        } catch (...) {
            m_orderId = g_globalStringInterner().intern(orderId); // Renamed to m_snake_case, g_camelCase
            m_orderIdNum = 0; // Renamed to m_snake_case
        }
    }
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif
