# 📚 API Documentation

## 🎯 Overview

This document provides comprehensive API documentation for the C++ Order Book trading engine with smart pointer architecture.

## 🏗️ Architecture Overview

The system is built around several core components:

```
Exchange (Main Interface)
├── OrderBook (per instrument)
│   ├── PriceLevels (buy/sell sides)
│   │   └── OrderList (orders at same price)
│   │       └── Node (linked list nodes)
│   └── OrderMap (fast order lookup)
├── BookMap (instrument -> OrderBook mapping)
└── SpinLock (lock-free synchronization)
```

## 🔧 Core Classes

### Exchange

Main interface for order management and trading operations.

#### Constructors

```cpp
Exchange();                                    // Default with dummy listener
explicit Exchange(ExchangeListener& listener);  // Custom event listener
```

#### Order Management

##### Limit Orders

```cpp
// Buy order
std::optional<long> buy(
    std::string_view sessionId,    // Trading session identifier
    std::string_view instrument,   // Trading symbol (e.g., "AAPL")
    F price,                       // Price using Fixed<7> type
    int quantity,                  // Order quantity
    std::string_view orderId = ""  // Optional order ID
);

// Sell order
std::optional<long> sell(
    std::string_view sessionId,
    std::string_view instrument,
    F price,
    int quantity,
    std::string_view orderId = ""
);
```

**Returns**: `std::optional<long>` - Exchange-assigned order ID if successful

**Example**:
```cpp
auto orderId = exchange.buy("session1", "AAPL", F(150.25), 100, "my_order");
if (orderId.has_value()) {
    std::cout << "Order placed with ID: " << orderId.value() << std::endl;
}
```

##### Market Orders

```cpp
// Market buy (executes at best available prices)
std::optional<long> marketBuy(
    std::string_view sessionId,
    std::string_view instrument,
    int quantity,
    std::string_view orderId = ""
);

// Market sell
std::optional<long> marketSell(
    std::string_view sessionId,
    std::string_view instrument,
    int quantity,
    std::string_view orderId = ""
);
```

#### Order Operations

```cpp
// Cancel order
bool cancel(long exchangeId, std::string_view sessionId);

// Get order details
std::optional<Order> getOrder(long exchangeId);

// Get order book snapshot
std::optional<Book> book(std::string_view instrument);
```

### Order Structure

Represents a single trading order with smart pointer management.

```cpp
struct Order {
    enum Side { BUY, SELL };
    
    // Basic properties
    Side side() const;                    // Buy or sell
    F price() const;                      // Order price (Fixed<7>)
    int quantity() const;                 // Order quantity
    const std::string& sessionId() const; // Session identifier
    const std::string& orderId() const;   // Original order ID
    const std::string& instrument() const; // Trading symbol
    
    // State information
    bool isOnList() const;                // Currently in order book
    bool isCancelled() const;             // Order was cancelled
    bool isFilled() const;                // Order completely filled
    bool isQuote() const;                 // Is a quote order
    
    // Execution information
    int remainingQuantity() const;        // Quantity to be executed
    int filledQuantity() const;           // Already executed quantity
    int cumulativeQuantity() const;       // Total executed quantity
    F averagePrice() const;               // Average execution price
    
    // Timestamps
    TimePoint createTime() const;         // Order creation time
    TimePoint executeTime() const;        // Last execution time
    
    // Operations
    void cancel();                        // Cancel the order
    bool isMarket() const;                // Is market order
};
```

### Book Structure

Order book snapshot containing bid and ask price levels.

```cpp
struct Book {
    std::vector<PriceLevel> bids;  // Buy orders (descending price)
    std::vector<PriceLevel> asks;  // Sell orders (ascending price)
};

struct PriceLevel {
    F price;                        // Price level
    int quantity;                   // Total quantity at this price
    int orderCount;                 // Number of orders at this price
};
```

## 🧠 Smart Pointer Architecture

### Memory Management

All orders and nodes are managed using smart pointers:

- **`std::shared_ptr<Order>`**: Shared ownership of orders
- **`std::shared_ptr<Node>`**: Linked list node management
- **`std::weak_ptr<Order>`**: Non-owning references to avoid cycles

### Factory Methods

Orders are created through factory methods to ensure proper initialization:

```cpp
// Internal factory method (called by Exchange)
static std::shared_ptr<Order> create(
    const std::string& sessionId,
    const std::string& orderId,
    const std::string& instrument,
    F price,
    int quantity,
    Order::Side side,
    TimePoint createTime,
    long exchangeId
);
```

### Thread Safety

- **Atomic operations**: All order book operations use atomic smart pointers
- **Lock-free structures**: Minimized contention for high performance
- **RAII**: Automatic cleanup with exception safety

## 📊 Event System

### ExchangeListener

Receive notifications for order updates and trades.

```cpp
struct ExchangeListener {
    // Order property changes
    virtual void onOrder(const Order& order) {}
    
    // Trade executions
    virtual void onTrade(const Trade& trade) {}
};
```

### Trade Structure

```cpp
struct Trade {
    std::string instrument;    // Trading symbol
    F price;                  // Execution price
    int quantity;             // Trade quantity
    TimePoint time;           // Execution time
    long aggressiveOrderId;   // Aggressive order ID
    long passiveOrderId;      // Passive order ID
};
```

## 🚀 Usage Examples

### Basic Trading

```cpp
#include "core/exchange.h"

class TradingBot : public ExchangeListener {
public:
    void run() {
        Exchange exchange(*this);
        
        // Place limit orders
        auto buyId = exchange.buy("bot_session", "AAPL", F(150.25), 100);
        auto sellId = exchange.sell("bot_session", "AAPL", F(150.30), 50);
        
        // Check order book
        auto book = exchange.book("AAPL");
        if (book.has_value()) {
            std::cout << "Bid levels: " << book->bids.size() << std::endl;
            std::cout << "Ask levels: " << book->asks.size() << std::endl;
        }
        
        // Cancel order
        if (buyId.has_value()) {
            exchange.cancel(buyId.value(), "bot_session");
        }
    }
    
    void onOrder(const Order& order) override {
        std::cout << "Order update: " << order.orderId() 
                  << " Quantity: " << order.remainingQuantity() << std::endl;
    }
    
    void onTrade(const Trade& trade) override {
        std::cout << "Trade: " << trade.instrument 
                  << " Price: " << trade.price 
                  << " Quantity: " << trade.quantity << std::endl;
    }
};
```

### High-Frequency Trading

```cpp
// Optimized for high-frequency operations
class HFTStrategy {
private:
    Exchange& exchange;
    std::string sessionId;
    
public:
    HFTStrategy(Exchange& ex) : exchange(ex), sessionId("hft_bot") {}
    
    void marketMake(const std::string& symbol, F spread, int size) {
        auto book = exchange.book(symbol);
        if (!book.has_value()) return;
        
        F midPrice = F(0);
        if (!book->bids.empty() && !book->asks.empty()) {
            midPrice = (book->bids[0].price + book->asks[0].price) / 2;
        }
        
        // Place bid and ask
        exchange.buy(sessionId, symbol, midPrice - spread/2, size);
        exchange.sell(sessionId, symbol, midPrice + spread/2, size);
    }
    
    void cancelAllOrders(const std::string& symbol) {
        auto book = exchange.book(symbol);
        if (!book.has_value()) return;
        
        // Cancel all orders for this symbol and session
        for (const auto& level : book->bids) {
            // Implementation depends on order tracking
        }
    }
};
```

## ⚠️ Error Handling

### Return Values

- **`std::optional<long>`**: Order operations - empty value indicates failure
- **`bool`**: Cancel operations - false indicates failure
- **`std::optional<T>`**: Query operations - empty value indicates not found

### Common Error Cases

```cpp
auto orderId = exchange.buy("session", "SYMBOL", F(100.0), 10);
if (!orderId.has_value()) {
    // Order rejected - handle error
    std::cerr << "Order placement failed" << std::endl;
}

bool cancelled = exchange.cancel(orderId.value(), "wrong_session");
if (!cancelled) {
    // Cancel failed - wrong session or order not found
    std::cerr << "Cancel failed" << std::endl;
}
```

### Exception Safety

All operations provide strong exception safety guarantees:
- Smart pointers ensure automatic cleanup
- No memory leaks on exceptions
- Consistent state maintenance

## 🔧 Configuration

### Fixed-Point Arithmetic

Prices use `Fixed<7>` type for precise decimal arithmetic:
- 7 decimal places of precision
- No floating-point rounding errors
- Fast integer operations

### Performance Tuning

```cpp
// Configure for high-frequency trading
Exchange exchange;

// Use custom listener for minimal overhead
struct MinimalListener : ExchangeListener {
    void onOrder(const Order& order) override {
        // Minimal processing for speed
    }
    void onTrade(const Trade& trade) override {
        // Minimal processing for speed
    }
};
```

## 📈 Performance Considerations

### Memory Management

- Smart pointers have minimal overhead (< 1% performance impact)
- Automatic cleanup prevents memory leaks
- Reference counting is atomic and thread-safe

### Thread Safety

- All operations are thread-safe
- Lock-free structures minimize contention
- Atomic smart pointers ensure consistency

### Optimization Tips

1. **Reuse session IDs** to avoid string allocations
2. **Batch operations** when possible
3. **Minimal listener implementations** for HFT
4. **Appropriate order sizes** for your use case

## 🧪 Testing

### Unit Tests

```bash
# Run all tests
make run_tests

# Individual components
./orderbook_test
./exchange_test
./spinlock_test
```

### Smart Pointer Tests

```bash
# Validate smart pointer functionality
./test_smart_pointer/smart_pointer_test

# Comprehensive demo
./demo/comprehensive_demo_msvc.exe
```

### Benchmarks

```bash
# Performance benchmarks
./benchmark_test
./benchmark_multithread_test
```

---

## 📞 Support

For API questions:
- Check the examples in `demo/` directory
- Review unit tests for usage patterns
- Consult the main README.md for build instructions
- Create GitHub issues for bugs or feature requests
