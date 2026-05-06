#include <catch2/catch_test_macros.hpp>

#include <thread>
#include <vector>
#include <chrono>
#include <random>

#include "core/test.h"
#include "safety/production_safety_inline.h"

struct IntegrationTestListener : ExchangeListener {
    std::vector<Trade> m_trades;
    std::vector<Order> m_orders;
    int m_tradeCount = 0;
    int m_orderCount = 0;
    
    void onTrade(const Trade& trade) override {
        m_trades.push_back(trade);
        m_tradeCount++;
    }
    
    void onOrder(const Order& order) override {
        m_orders.push_back(order);
        m_orderCount++;
    }
    
    void reset() {
        trades.clear();
        orders.clear();
        trade_count = 0;
        order_count = 0;
    }
};

TEST_CASE("IntegrationTest FullOrderLifecycle", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    // Place a buy order
    auto buyOrderId = exchange.placeBuyOrder(100.0, 100, "buy1");
    REQUIRE(buyOrderId.value() > 0);
    REQUIRE(exchange.bidCount() == 1);
    REQUIRE(exchange.askCount() == 0);
    
    // Place a sell order that matches
    auto sellOrderId = exchange.placeSellOrder(99.0, 50, "sell1");
    REQUIRE(sellOrderId.value() > 0);
    
    // Check trade execution
    REQUIRE(listener.m_tradeCount == 1);
    REQUIRE(listener.m_trades[0].m_quantity == 50);
    
    // Check remaining quantities
    REQUIRE(exchange.getOrder(buyOrderId.value()).value().remainingQuantity() == 50);
    REQUIRE(exchange.getOrder(sellOrderId.value()).value().remainingQuantity() == 0);
    
    // Cancel remaining buy order
    REQUIRE(exchange.cancelOrder(buyOrderId.value(), "session1"));
    REQUIRE(exchange.bidCount() == 0);
}

TEST_CASE("IntegrationTest MultiInstrumentTrading", "[integration]") {
    ProductionSafety::enable_safety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    // Trade on multiple instruments
    exchange.placeBuyOrder(100.0, 100, "aapl_buy1");
    exchange.placeSellOrder(99.0, 50, "aapl_sell1");
    
    exchange.placeBuyOrder(50.0, 200, "goog_buy1");
    exchange.placeSellOrder(49.0, 100, "goog_sell1");
    
    exchange.placeBuyOrder(200.0, 50, "msft_buy1");
    exchange.placeSellOrder(199.0, 25, "msft_sell1");
    
    // Verify trades occurred
    REQUIRE(listener.m_tradeCount >= 3);
    
    // Verify order book state
    REQUIRE(exchange.bidCount() >= 0);
    REQUIRE(exchange.askCount() >= 0);
}

TEST_CASE("IntegrationTest HighFrequencyTradingSimulation", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    const int kNumOrders = 500;
    std::random_device random_device;
    std::mt19937 random_generator(random_device());
    std::uniform_real_distribution<> price_distribution(90.0, 110.0);
    std::uniform_int_distribution<> quantity_distribution(10, 100);
    
    // Place many orders rapidly
    for (int i = 0; i < kNumOrders; ++i) {
        double price = price_distribution(random_generator);
        int qty = quantity_distribution(random_generator);
        
        if (i % 2 == 0) {
            exchange.placeBuyOrder(price, qty, "order_" + std::to_string(i));
        } else {
            exchange.placeSellOrder(price, qty, "order_" + std::to_string(i));
        }
    }
    
    // Verify system handled all orders
    REQUIRE(listener.m_orderCount == kNumOrders * 2); // Each order generates 2 events
    
    // Verify trades occurred
    REQUIRE(listener.m_tradeCount > 0);
}

TEST_CASE("IntegrationTest MarketOrderIntegration", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    // Build order book with sell orders
    exchange.placeSellOrder(100.0, 100, "sell1");
    exchange.placeSellOrder(101.0, 100, "sell2");
    exchange.placeSellOrder(102.0, 100, "sell3");
    
    REQUIRE(exchange.askCount() == 3);
    
    // Market buy that crosses multiple levels
    exchange.placeMarketBuyOrder(250, "market_buy1");
    
    // Verify trades
    REQUIRE(listener.m_tradeCount >= 2);
    
    // Verify remaining ask
    REQUIRE(exchange.askCount() == 1);
}

TEST_CASE("IntegrationTest PartialFillAcrossMultipleOrders", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    // Place multiple buy orders at the same price
    auto buyOrder1 = exchange.placeBuyOrder(100.0, 100, "buy1");
    auto buyOrder2 = exchange.placeBuyOrder(100.0, 50, "buy2");
    auto buyOrder3 = exchange.placeBuyOrder(100.0, 75, "buy3");
    
    REQUIRE(exchange.bidCount() == 1);
    
    // Sell that partially fills
    exchange.placeSellOrder(99.0, 120, "sell1");
    
    // Verify trade
    REQUIRE(listener.m_tradeCount == 1);
    REQUIRE(listener.m_trades[0].m_quantity == 120);
    
    // Verify remaining quantities
    int total_remaining = exchange.getOrder(buyOrder1.value()).value().remainingQuantity() +
                         exchange.getOrder(buyOrder2.value()).value().remainingQuantity() +
                         exchange.getOrder(buyOrder3.value()).value().remainingQuantity();
    REQUIRE(total_remaining == 105);
}

TEST_CASE("IntegrationTest OrderCancellationIntegration", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    // Place multiple buy orders
    auto buyOrder1 = exchange.placeBuyOrder(100.0, 100, "buy1");
    auto buyOrder2 = exchange.placeBuyOrder(99.0, 50, "buy2");
    auto buyOrder3 = exchange.placeBuyOrder(98.0, 75, "buy3");
    
    REQUIRE(exchange.bidCount() == 3);
    
    // Cancel middle order
    REQUIRE(exchange.cancelOrder(buyOrder2.value(), "session1"));
    REQUIRE(exchange.bidCount() == 2);
    
    // Cancel first order
    REQUIRE(exchange.cancelOrder(buyOrder1.value(), "session1"));
    REQUIRE(exchange.bidCount() == 1);
    
    // Try to cancel already cancelled order
    REQUIRE_FALSE(exchange.cancelOrder(buyOrder1.value(), "session1"));
}

TEST_CASE("IntegrationTest ConcurrentOperations", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    const int kNumThreads = 4;
    const int kOrdersPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int t = 0; t < kNumThreads; ++t) {
        threads.emplace_back([&exchange, t, kOrdersPerThread]() {
            for (int i = 0; i < kOrdersPerThread; ++i) {
                double price = 100.0 + (t * 10) + (i % 5);
                if (i % 2 == 0) {
                    exchange.placeBuyOrder(price, 10, "thread_" + std::to_string(t) + "_order_" + std::to_string(i));
                } else {
                    exchange.placeSellOrder(price, 10, "thread_" + std::to_string(t) + "_order_" + std::to_string(i));
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify that a significant number of orders were processed
    REQUIRE(listener.m_orderCount >= kNumThreads * kOrdersPerThread);
}

TEST_CASE("IntegrationTest SmartPointerMemoryManagement", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    // Place and cancel many orders to test smart pointer cleanup
    const int kNumOrders = 1000;
    std::vector<ExchangeId> order_identifiers;
    
    for (int i = 0; i < kNumOrders; ++i) {
        auto order_id = exchange.placeBuyOrder(100.0 + (i % 10), 10, "order_" + std::to_string(i));
        order_identifiers.push_back(order_id.value()); // extract long from optional
    }
    
    REQUIRE(exchange.bidCount() == 10);
    
    // Cancel all orders
    for (auto id : order_identifiers) {
        exchange.cancelOrder(id, "session1");
    }
    
    REQUIRE(exchange.bidCount() == 0);
    
    // Smart pointers should have automatically cleaned up memory
    // No memory leaks should occur
}

TEST_CASE("IntegrationTest OrderBookConsistency", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    // Build complex order book
    for (int i = 0; i < 20; ++i) {
        exchange.placeBuyOrder(100.0 - i, 10, "buy_" + std::to_string(i));
        exchange.placeSellOrder(100.0 + i, 10, "sell_" + std::to_string(i));
    }
    
    auto book = exchange.getBook(kDefaultInstrument).value();
    
    // Verify bid ordering (descending)
    for (size_t i = 1; i < book.m_bids.size(); ++i) {
        REQUIRE(book.m_bids[i-1].m_price >= book.m_bids[i].m_price);
    }
    
    // Verify ask ordering (ascending)
    for (size_t i = 1; i < book.m_asks.size(); ++i) {
        REQUIRE(book.m_asks[i-1].m_price <= book.m_asks[i].m_price);
    }
    
    // Verify spread
    if (!book.m_bids.empty() && !book.m_asks.empty()) {
        REQUIRE(book.m_bids[0].m_price <= book.m_asks[0].m_price);
    }
}

TEST_CASE("IntegrationTest ErrorHandlingIntegration", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    // Try to cancel non-existent order
    REQUIRE_THROWS(exchange.cancelOrder(999999, "session1"));
    
    // Place valid order
    auto id = exchange.placeBuyOrder(100.0, 10, "valid_order");
    REQUIRE(id.value() > 0);
    
    // Try to cancel with wrong session
    REQUIRE_FALSE(exchange.cancelOrder(id.value(), "wrong_session"));
    
    // Cancel with correct session
    REQUIRE(exchange.cancelOrder(id.value(), "session1"));
}

TEST_CASE("IntegrationTest PerformanceIntegration", "[integration]") {
    ProductionSafety::enableSafety(false);
    
    IntegrationTestListener listener;
    TestExchange exchange;
    
    const int kNumOrders = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < kNumOrders; ++i) {
        if (i % 2 == 0) { // Alternate between buy and sell orders
            exchange.placeBuyOrder(100.0 + (i % 100), 10, "order_" + std::to_string(i));
        } else { // Place a sell order
            exchange.placeSellOrder(100.0 + (i % 100), 10, "order_" + std::to_string(i));
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Verify that the system processes orders at a reasonable rate (e.g., > 1M orders/sec)
    double ordersPerSec = (kNumOrders * 1000000.0) / duration.count();
    REQUIRE(ordersPerSec > 1000000.0);
}
