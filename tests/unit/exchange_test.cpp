#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "core/test.h"
#include "safety/production_safety_inline.h"

struct TestListener : ExchangeListener {
    std::vector<Trade> trades;
    void onTrade(const Trade& trade) override {
        trades.push_back(trade);
    }
    std::vector<Order> orders;
    void onOrder(const Order& order) override {
        orders.push_back(order);
    }
};

TEST(ExchangeTest, InsertOrderBuy) {
    // Disable production safety in tests to prevent recursion with Boost Test Framework
    ProductionSafety::enableSafety(false);
    
    TestExchange ob;

    auto o1_id = ob.buy(1.0,10,"1");
    auto o2_id = ob.buy(2.0,10,"2");

    EXPECT_EQ(ob.bidCount(), 2);
    EXPECT_EQ(ob.askCount(), 0);

    EXPECT_EQ(ob.bidIndex(o2_id), 0) << o2_id;
    EXPECT_EQ(ob.bidIndex(o1_id), 1) << o1_id;
}

TEST(ExchangeTest, InsertOrderBuy2) {
    TestExchange ob;

    auto o2_id = ob.buy(2.0,10,"2");
    auto o1_id = ob.buy(1.0,10,"1");

    // should end up with same ordering

    EXPECT_EQ(ob.bidCount(), 2);
    EXPECT_EQ(ob.askCount(), 0);

    EXPECT_EQ(ob.bidIndex(o2_id), 0);
    EXPECT_EQ(ob.bidIndex(o1_id), 1);
}

TEST(ExchangeTest, InsertOrderSell) {
    TestExchange ob;

    auto o1_id = ob.sell(1.0,10,"1");
    auto o2_id = ob.sell(2.0,10,"2");

    EXPECT_EQ(ob.bidCount(), 0);
    EXPECT_EQ(ob.askCount(), 2);

    EXPECT_EQ(ob.askIndex(o2_id), 1);
    EXPECT_EQ(ob.askIndex(o1_id), 0);
}

TEST(ExchangeTest, InsertOrderSell2) {
    TestExchange ob;

    auto o2_id = ob.sell(2.0,10,"2");
    auto o1_id = ob.sell(1.0,10,"1");

    EXPECT_EQ(ob.bidCount(), 0);
    EXPECT_EQ(ob.askCount(), 2);

    EXPECT_EQ(ob.askIndex(o2_id), 1);
    EXPECT_EQ(ob.askIndex(o1_id), 0);
}

TEST(ExchangeTest, InsertOrderBuySamePrice) {
    TestExchange ob;

    auto o1_id = ob.buy(1.0,10,"1");
    auto o2_id = ob.buy(2.0,10,"2");
    auto o3_id = ob.buy(2.0,25,"3");

    EXPECT_EQ(ob.bidCount(), 2);
    EXPECT_EQ(ob.book().bidOrderIds.size(), 3u);
    EXPECT_EQ(ob.askCount(), 0);

    EXPECT_EQ(ob.bidIndex(o2_id), 0);
    EXPECT_EQ(ob.bidIndex(o3_id), 1);
    EXPECT_EQ(ob.bidIndex(o1_id), 2);
}

TEST(ExchangeTest, Fill) {

    TestListener listener;

    TestExchange ob(listener);

    auto o1_id = ob.buy(1.0,10,"1");

    EXPECT_EQ(listener.trades.size(), 0u);

    auto o2_id = ob.sell(0.75,10,"2");

    ASSERT_EQ(listener.trades.size(), 1u);
    EXPECT_EQ(listener.trades[0].aggressor.exchangeId, o2_id);
    EXPECT_EQ(listener.trades[0].opposite.exchangeId, o1_id);
}

TEST(ExchangeTest, PartialFill) {

    TestListener listener;
    TestExchange ob(listener);

    auto o1_id = ob.buy(1.0,20,"1");

    EXPECT_EQ(listener.trades.size(), 0u);

    auto o2_id = ob.sell(.75,10,"2");

    EXPECT_EQ(listener.trades.size(), 1u);
    EXPECT_EQ(listener.trades[0].aggressor.exchangeId, o2_id);
    EXPECT_EQ(listener.trades[0].opposite.exchangeId, o1_id);

    EXPECT_EQ(ob.getOrder(o2_id).remainingQuantity(), 0);
    EXPECT_EQ(ob.getOrder(o1_id).remainingQuantity(), 10);

    EXPECT_EQ(ob.bidCount(), 1);
    EXPECT_EQ(ob.askCount(), 0);

}

TEST(ExchangeTest, Cancel) {

    TestListener listener;

    TestExchange ob(listener);

    auto o1_id = ob.buy(1.0,20,"1");
    EXPECT_EQ(ob.cancel(o1_id, "dummy"), 0);
    EXPECT_EQ(ob.cancel(o1_id, "dummy"), -1);

    // should be an event for the order and the cancel
    EXPECT_EQ(listener.orders.size(), 2u);
    EXPECT_EQ(ob.bidCount(), 0);
}

TEST(ExchangeTest, CancelInvalid) {

    TestListener listener;
    TestExchange ob(listener);

    auto o1_id = ob.buy(1.0,20,"1");
    EXPECT_THROW(ob.cancel(o1_id + 1, "dummy"), std::runtime_error);
}

TEST(ExchangeTest, MarketBuy) {

    TestListener listener;

    TestExchange ob(listener);

    ob.sell(1.0,20,"1");
    ob.marketBuy(10,"2");

    EXPECT_EQ(listener.orders.size(), 4u);
    EXPECT_EQ(listener.trades.size(), 1u);
    EXPECT_EQ(ob.bidCount(), 0);
    EXPECT_EQ(ob.askCount(), 1);
}

TEST(ExchangeTest, MarketBuyCancelRemaining) {

    TestListener listener;

    TestExchange ob(listener);

    ob.sell(1.0,20,"1");
    ob.marketBuy(30,"2");

    EXPECT_EQ(listener.orders.size(), 5u);
    EXPECT_EQ(listener.trades.size(), 1u);
    EXPECT_EQ(ob.bidCount(), 0);
    EXPECT_EQ(ob.askCount(), 0);
}

TEST(ExchangeTest, MarketBuyMultiLevel) {

    TestListener listener;

    TestExchange ob(listener);

    ob.sell(1.0,20,"1");
    ob.sell(2.0,20,"2");

    ob.marketBuy(30,"3");

    // initial orders (3) + 2x2 updates due to trades (1 full and 1 partial) = 7 statuses
    EXPECT_EQ(listener.orders.size(), 7u);
    EXPECT_EQ(listener.trades.size(), 2u);
    EXPECT_EQ(ob.bidCount(), 0);
    EXPECT_EQ(ob.askCount(), 1);
    ASSERT_FALSE(ob.book().asks.empty());
    EXPECT_EQ(ob.book().asks[0].quantity, 10);
}

TEST(ExchangeTest, MarketBuyOneSided) {

    TestListener listener;

    TestExchange ob(listener);

    ob.marketBuy(30,"1");

    EXPECT_EQ(listener.orders.size(), 2u);
    EXPECT_EQ(listener.trades.size(), 0u);
    EXPECT_EQ(ob.bidCount(), 0);
    EXPECT_EQ(ob.askCount(), 0);
}

TEST(ExchangeTest, OrderImmutability) {

    TestListener listener;

    TestExchange ob(listener);

    auto o1_id = ob.buy(1.0,30,"1");
    Order order = ob.getOrder(o1_id);
    ob.sell(1.0,10,"2");
    Order latest = ob.getOrder(o1_id);

    EXPECT_EQ(order.remainingQuantity(), 30);
    EXPECT_EQ(latest.remainingQuantity(), 20);
}

