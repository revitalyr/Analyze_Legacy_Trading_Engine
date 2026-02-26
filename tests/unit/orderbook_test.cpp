#include <gtest/gtest.h>

#include "core/order.h"
#include "core/orderbook.h"
#include "core/test.h"

TEST(OrderBookTest, OrderbookCancel) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument, listener);

    auto o1 = new TestOrder(1, 100, 10, Order::BUY);
    ob.insertOrder(o1);
    ob.cancelOrder(o1);

    auto levels = ob.book();
    EXPECT_EQ(levels.bids.size(), 0u);

    auto o2 = new TestOrder(1, 100, 10, Order::BUY);
    ob.insertOrder(o2);
    auto o3 = new TestOrder(1, 90, 10, Order::BUY);
    ob.insertOrder(o3);
    auto o4 = new TestOrder(1, 80, 10, Order::BUY);
    ob.insertOrder(o4);

    ob.cancelOrder(o3);

    auto book = ob.book();

    ASSERT_EQ(book.bids.size(), 2u);
    EXPECT_EQ(book.bids[0].price, 100);
    EXPECT_EQ(book.bids[1].price, 80);
}

TEST(OrderBookTest, Booklevels) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument, listener);

    auto o1 = new TestOrder(1, 100, 10, Order::BUY);
    ob.insertOrder(o1);

    auto levels = ob.book();

    ASSERT_FALSE(levels.bids.empty());
    EXPECT_EQ(levels.bids[0].price, 100);
    EXPECT_EQ(levels.bids[0].quantity, 10);
}

TEST(OrderBookTest, BooklevelsSum) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument, listener);

    auto o1 = new TestOrder(1, 100, 10, Order::BUY);
    ob.insertOrder(o1);
    auto o2 = new TestOrder(2, 100, 10, Order::BUY);
    ob.insertOrder(o2);

    auto levels = ob.book();

    ASSERT_FALSE(levels.bids.empty());
    EXPECT_EQ(levels.bids[0].price, 100);
    EXPECT_EQ(levels.bids[0].quantity, 20);
}

TEST(OrderBookTest, BooklevelsMultiple) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument, listener);

    auto o1 = new TestOrder(1, 100, 10, Order::BUY);
    ob.insertOrder(o1);
    auto o2 = new TestOrder(2, 100, 10, Order::BUY);
    ob.insertOrder(o2);
    auto o3 = new TestOrder(2, 200, 30, Order::BUY);
    ob.insertOrder(o3);

    auto levels = ob.book();

    ASSERT_GE(levels.bids.size(), 2u);
    EXPECT_EQ(levels.bids[0].price, 200);
    EXPECT_EQ(levels.bids[0].quantity, 30);
    EXPECT_EQ(levels.bids[1].price, 100);
    EXPECT_EQ(levels.bids[1].quantity, 20);
}

TEST(OrderBookTest, BooklevelsOrder) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument, listener);

    ob.insertOrder(new TestOrder(1, 100, 10, Order::BUY));
    ob.insertOrder(new TestOrder(1, 101, 10, Order::BUY));
    ob.insertOrder(new TestOrder(1, 99, 10, Order::BUY));
    ob.insertOrder(new TestOrder(1, 98, 10, Order::BUY));

    ob.insertOrder(new TestOrder(1, 200, 10, Order::SELL));
    ob.insertOrder(new TestOrder(1, 199, 10, Order::SELL));
    ob.insertOrder(new TestOrder(1, 201, 10, Order::SELL));
    ob.insertOrder(new TestOrder(1, 202, 10, Order::SELL));

    auto levels = ob.book();

    ASSERT_GE(levels.bids.size(), 4u);
    EXPECT_EQ(levels.bids[0].price, 101);
    EXPECT_EQ(levels.bids[1].price, 100);
    EXPECT_EQ(levels.bids[2].price, 99);
    EXPECT_EQ(levels.bids[3].price, 98);

    ASSERT_GE(levels.asks.size(), 4u);
    EXPECT_EQ(levels.asks[0].price, 199);
    EXPECT_EQ(levels.asks[1].price, 200);
    EXPECT_EQ(levels.asks[2].price, 201);
    EXPECT_EQ(levels.asks[3].price, 202);
}

TEST(OrderBookTest, SessionId) {
    const std::string s1("session1");
    const std::string s2("session2");
    const std::string s3("session1");

    EXPECT_TRUE(s1 < s2);
    EXPECT_FALSE(s2 < s1);
    EXPECT_TRUE(s1 == s1);
    EXPECT_FALSE(s1 == s2);
    EXPECT_TRUE(s1 == s3);
}

TEST(OrderBookTest, SessionQuoteId) {
    std::string session1("session1");
    std::string session2("session2");
    std::string session3("session1");

    SessionQuoteId s1(session1, "quote1");
    SessionQuoteId s2(session2, "quote2");
    SessionQuoteId s3(session3, "quote1");

    EXPECT_TRUE(s1 < s2);
    EXPECT_FALSE(s2 < s1);
    EXPECT_TRUE(s1 == s1);
    EXPECT_FALSE(s1 == s2);
    EXPECT_TRUE(s1 == s3);
}

TEST(OrderBookTest, Quoting) {
    OrderBookListener listener;
    OrderBook ob(dummy_instrument,listener);

    std::string sessionId("session");
    std::string quoteId("myquote");

    auto quotes = QuoteOrders{new TestOrder(1,100,10,Order::BUY),new TestOrder(2,101,20,Order::SELL)};

    ob.quote(quotes,100,10,101,20);

    auto levels = ob.book();

    ASSERT_EQ(levels.bids.size(), 1u);
    EXPECT_EQ(levels.bids[0].price, 100);
    EXPECT_EQ(levels.bids[0].quantity, 10);

    ASSERT_EQ(levels.asks.size(), 1u);
    EXPECT_EQ(levels.asks[0].price, 101);
    EXPECT_EQ(levels.asks[0].quantity, 20);

    ob.quote(quotes,100,20,101,30);

    levels = ob.book();

    ASSERT_EQ(levels.bids.size(), 1u);
    EXPECT_EQ(levels.bids[0].price, 100);
    EXPECT_EQ(levels.bids[0].quantity, 20);

    ASSERT_EQ(levels.asks.size(), 1u);
    EXPECT_EQ(levels.asks[0].price, 101);
    EXPECT_EQ(levels.asks[0].quantity, 30);

    ob.quote(quotes,100,0,101,30);

    levels = ob.book();

    EXPECT_EQ(levels.bids.size(), 0u);
    ASSERT_EQ(levels.asks.size(), 1u);

    EXPECT_EQ(levels.asks[0].price, 101);
    EXPECT_EQ(levels.asks[0].quantity, 30);

    ob.quote(quotes,100,0,101,0);
    levels = ob.book();
    EXPECT_EQ(levels.bids.size(), 0u);
    EXPECT_EQ(levels.asks.size(), 0u);
}
