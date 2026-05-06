# C++ Orderbook Semantic Type Refactoring Guide

## Overview

This refactoring introduces **semantic type aliases** and **consolidated constants** to improve code clarity, maintainability, and type safety across the C++ orderbook trading engine. All Russian comments have been translated to English.

---

## New Headers

### 1. `include/core/semantic_types.h`

**Purpose**: Defines domain-meaningful type aliases that encode semantic information directly into the type system.

#### Key Type Aliases

**Numeric Types:**
```cpp
using ExchangeId = int64_t;           // Unique order identifier
using ExecutionId = int64_t;          // Unique execution/trade ID
using Quantity = int32_t;             // Order quantity (shares, contracts)
using ObjectCount = size_t;           // Collection size
using RecursionDepth = int32_t;       // Recursion depth counter
using FailureCount = int32_t;         // Failure count for circuit breaker
```

**String Types:**
```cpp
using SessionId = std::string;        // Trading session identifier
using SessionIdView = std::string_view;
using InstrumentSymbol = std::string; // Trading symbol (AAPL, EUR/USD)
using InstrumentSymbolView = std::string_view;
using OrderIdStr = std::string;       // Order identifier string
using OrderIdStrView = std::string_view;
using QuoteId = std::string;          // Multi-leg quote identifier
using QuoteIdView = std::string_view;
```

**Time Types:**
```cpp
using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
using Duration = std::chrono::duration<int64_t, std::milli>;
using NanosecondTimestamp = int64_t;  // Nanosecond precision
```

#### Benefits

- **Self-Documenting**: Function signatures immediately convey semantic meaning
- **Type Safety**: Prevents accidental parameter confusion at compile time
- **FFI Clarity**: Makes C-to-Rust or C++-to-Rust bindings explicit and verifiable

**Example:**
```cpp
// Before: What does this mean?
OrderResult buy(std::string_view a, F b, int c);

// After: Crystal clear!
OrderResult buy(SessionIdView sessionId, Price price, Quantity quantity);
```

---

### 2. `include/core/constants.h`

**Purpose**: Consolidates all hardcoded constants for easy configuration and maintenance.

#### Organized Constant Groups

**Buffer and Collection Limits:**
```cpp
constexpr size_t MAX_INSTRUMENTS = 1024;      // Maximum order books
constexpr size_t NODE_SIZE = 64;              // Cache-line aligned
```

**Recursion and Stack Protection:**
```cpp
constexpr int MAX_RECURSION_DEPTH = 50;       // Stack overflow threshold
constexpr int WARNING_THRESHOLD = 80;         // Near-limit warning
```

**Circuit Breaker Configuration:**
```cpp
constexpr int FAILURE_THRESHOLD = 10;         // Failures before breaker opens
constexpr std::chrono::seconds RESET_INTERVAL{1};         // Recursion reset period
constexpr std::chrono::seconds COOLDOWN_PERIOD{30};       // Breaker cooldown
```

**Default Values:**
```cpp
constexpr const char* DEFAULT_INSTRUMENT = "SYM1";
constexpr double MARKET_BUY_PRICE = DBL_MAX;              // Buy infinity
constexpr double MARKET_SELL_PRICE = -DBL_MAX;            // Sell infinity
constexpr uint32_t INVALID_STRING_ID = 0;                 // String interner sentinel
```

#### Benefits

- **Centralized Configuration**: Change once, updates everywhere
- **Self-Documenting**: Constant names explain their purpose
- **No Magic Numbers**: Improves code readability and maintainability
- **Easy Testing**: Configure constants for different test scenarios

---

## Updated Files

### Header Changes Summary

| File | Changes |
|------|---------|
| `order.h` | ✅ Added semantic types for all parameters; replaced `F` with `Price`; replaced DBL_MAX constants; updated member types |
| `orderbook.h` | ✅ Updated Trade struct; BookLevel struct; Book struct to use semantic types |
| `exchange.h` | ✅ Changed OrderResult type; updated all method signatures with semantic types |
| `bookmap.h` | ✅ Replaced `#define MAX_INSTRUMENTS` with constant; updated function parameters |
| `test.h` | ✅ Updated TestExchange/TestOrder to use semantic types; replaced "SYM1" with constant |
| `memory_pool.h` | ✅ Updated NODE_SIZE to use constant from constants.h |
| `insert_result.h` | ✅ Updated StackProtection to use constants from constants.h |
| `production_safety_inline.h` | ✅ **Translated all Russian comments to English** |

---

## Translation Guide - Russian to English

### Comments Translated

| Russian | English | Location |
|---------|---------|----------|
| "единый источник истины для глубины рекурсии" | "single source of truth for recursion depth" | production_safety_inline.h |
| "Периодический сброс для предотвращения permanent lockout" | "periodic reset to prevent permanent lockout" | production_safety_inline.h |
| "сброс для восстановления" | "reset for recovery" | production_safety_inline.h |
| "Не копируется, не перемещается" | "not copyable, not movable" | production_safety_inline.h |
| "безопасные для Boost.Test" | "safe for Boost.Test" | production_safety_inline.h |

---

## Usage Examples

### Example 1: Creating an Order

**Before (Unclear types):**
```cpp
auto order = Order::create(
    "SESSION_001",      // string_view - but what does it represent?
    "ORDER_123",        // string_view - order ID
    "AAPL",             // string_view - instrument
    Price(100.50),      // F - price
    1000,               // int - what unit? shares? bytes?
    Order::Side::BUY,   // Order::Side
    12345               // long - what ID is this?
);
```

**After (Self-documenting):**
```cpp
auto order = Order::create(
    SessionIdView("SESSION_001"),           // ✓ Clearly a session identifier
    OrderIdStrView("ORDER_123"),            // ✓ Clearly an order ID
    InstrumentSymbolView("AAPL"),           // ✓ Clearly the trading symbol
    Price(100.50),                          // ✓ Price type
    Quantity(1000),                         // ✓ Quantity in units
    Order::Side::BUY,
    ExchangeId(12345)                       // ✓ Clearly the exchange ID
);
```

### Example 2: Order Book Method

**Before:**
```cpp
OrderResult buy(
    std::string_view sessionId,
    std::string_view instrument,
    F price,
    int quantity,
    std::string_view orderId = ""
);
```

**After:**
```cpp
OrderResult buy(
    SessionIdView sessionId,
    InstrumentSymbolView instrument,
    Price price,
    Quantity quantity,
    OrderIdStrView orderId = ""
);
```

### Example 3: Using Constants

**Before:**
```cpp
if (recursion_depth > 50) {  // Magic number - what does 50 mean?
    // Handle recursion
}

// Default instrument string scattered throughout
auto book = orderbook.get("SYM1");
```

**After:**
```cpp
if (recursion_depth > MAX_RECURSION_DEPTH) {  // ✓ Clear intent
    // Handle recursion
}

// Centralized default
auto book = orderbook.get(DEFAULT_INSTRUMENT);
```

---

## Naming Conventions Applied

This refactoring follows **Google C++ Style Guide** conventions:

| Element | Convention | Example |
|---------|-----------|---------|
| Classes/Structs | PascalCase | `OrderBook`, `Exchange` |
| Functions | PascalCase | `GetOrder()`, `InsertOrder()` |
| Member Variables | snake_case_ | `order_id_`, `remaining_` |
| Type Aliases | PascalCase | `ExchangeId`, `SessionId` |
| Constants | kPascalCase or SCREAMING_SNAKE_CASE | `kMaxSize`, `MAX_INSTRUMENTS` |
| Namespaces | snake_case | `orderbook::trading` |

---

## FFI Boundary Clarity

### C to Rust Mapping

With semantic type aliases, the FFI boundary becomes explicit:

```c
// C header with semantic types
typedef int64_t ExchangeId;        // Maps to Rust: pub type ExchangeId = i64;
typedef int32_t Quantity;          // Maps to Rust: pub type Quantity = i32;
typedef uint32_t StringId;         // Maps to Rust: pub type StringId = u32;

struct Order {
    ExchangeId exchange_id;        // Clear what this field represents
    Quantity remaining;            // Clear units
    StringId session_id;           // Clear this is an interned string ID
};
```

---

## Migration Guide for Existing Code

### Step 1: Add New Headers to Compilation

```cmake
# CMakeLists.txt
target_include_directories(orderbook PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/core)
```

### Step 2: Update Include Statements

```cpp
// Old
#include "order.h"

// New (automatic - order.h now includes semantic_types.h and constants.h)
#include "order.h"
```

### Step 3: Update Type Usage

```cpp
// Old
long exchangeId;
int quantity;
std::string sessionId;

// New
ExchangeId exchangeId;
Quantity quantity;
SessionId sessionId;
```

### Step 4: Replace Magic Numbers

```cpp
// Old
if (depth > 50) { /* ... */ }

// New
if (depth > MAX_RECURSION_DEPTH) { /* ... */ }
```

---

## Verification Checklist

- ✅ All type aliases defined in `semantic_types.h`
- ✅ All constants defined in `constants.h`
- ✅ All Russian comments translated to English
- ✅ Core headers updated with semantic types
- ✅ Function signatures updated with semantic types
- ✅ Member variables updated with semantic types
- ✅ Test utilities updated with semantic types
- ✅ Default constants used instead of magic strings
- ✅ File compiles without errors
- ✅ Tests pass successfully

---

## Benefits Summary

| Benefit | Description |
|---------|-----------|
| **Type Safety** | Prevents wrong types from being passed to functions |
| **Self-Documentation** | Function signatures are immediately understandable |
| **Maintainability** | Single point of change for constants and type definitions |
| **FFI Clarity** | Crystal clear mapping between C and other languages |
| **Reduced Bugs** | Type system catches semantic errors at compile time |
| **Code Review** | Reviewers see intent through types, not comments |
| **Internationalization** | All comments now in English |

---

## Next Steps (Optional Enhancements)

1. **Rust FFI**: Generate Rust type bindings from these semantic aliases
2. **Type Audit**: Review remaining raw types for potential semantic aliasing
3. **Runtime Validation**: Add assertions to verify semantic type constraints
4. **Documentation**: Generate API docs that emphasize semantic meaning
5. **Performance Testing**: Verify zero-overhead for type aliases

---

## References

- [Google C++ Style Guide - Naming Conventions](https://google.github.io/styleguide/cppguide.html#Naming_Convention)
- [Domain-Driven Design in C++](https://www.pluralsight.com/courses/domain-driven-design-in-practice)
- [Strong Types Pattern](https://www.fluentcpp.com/2016/12/08/strong-types/)
- [Type-driven Development](https://learnyouahaskell.com/types-and-typeclasses)
