#include <catch2/catch_test_macros.hpp>

#include "core/order.h"
#include "core/orderbook.h"
#include "core/test.h"

TEST_CASE("OrderBook cancel order", "[orderbook]") {
    OrderBookListener testListener; // Renamed to camelCase
    OrderBook orderBook(kDummyInstrument, testListener); // Renamed to kPascalCase, camelCase

    auto order1 = std::make_shared<TestOrder>(1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order1); // Renamed to camelCase
    orderBook.cancelOrder(order1); // Renamed to camelCase

    auto bookLevels = orderBook.getBook(); // Renamed to camelCase
    REQUIRE(bookLevels.m_bids.size() == 0); // Renamed to m_snake_case

    auto order2 = std::make_shared<TestOrder>(1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order2); // Renamed to camelCase
    auto order3 = std::make_shared<TestOrder>(1, 90, 10, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order3); // Renamed to camelCase
    auto order4 = std::make_shared<TestOrder>(1, 80, 10, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order4); // Renamed to camelCase

    orderBook.cancelOrder(order3); // Renamed to camelCase

    auto bookSnapshot = orderBook.getBook(); // Renamed to camelCase

    REQUIRE(bookSnapshot.m_bids.size() == 2); // Renamed to m_snake_case
    REQUIRE(bookSnapshot.m_bids[0].m_price == 100); // Renamed to m_snake_case
    REQUIRE(bookSnapshot.m_bids[1].m_price == 80); // Renamed to m_snake_case
}

TEST_CASE("OrderBook book levels", "[orderbook]") {
    OrderBookListener testListener; // Renamed to camelCase
    OrderBook orderBook(kDummyInstrument, testListener); // Renamed to kPascalCase, camelCase

    auto order1 = std::make_shared<TestOrder>(1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order1); // Renamed to camelCase

    auto bookLevels = orderBook.getBook(); // Renamed to camelCase

    REQUIRE_FALSE(bookLevels.m_bids.empty()); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[0].m_price == 100); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[0].m_quantity == 10); // Renamed to m_snake_case
}

TEST_CASE("OrderBook book levels sum", "[orderbook]") {
    OrderBookListener testListener; // Renamed to camelCase
    OrderBook orderBook(kDummyInstrument, testListener); // Renamed to kPascalCase, camelCase

    auto order1 = std::make_shared<TestOrder>(1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order1); // Renamed to camelCase
    auto order2 = std::make_shared<TestOrder>(2, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order2); // Renamed to camelCase

    auto bookLevels = orderBook.getBook(); // Renamed to camelCase

    REQUIRE_FALSE(bookLevels.m_bids.empty()); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[0].m_price == 100); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[0].m_quantity == 20); // Renamed to m_snake_case
}

TEST_CASE("OrderBook book levels multiple", "[orderbook]") {
    OrderBookListener testListener; // Renamed to camelCase
    OrderBook orderBook(kDummyInstrument, testListener); // Renamed to kPascalCase, camelCase

    auto order1 = std::make_shared<TestOrder>(1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order1); // Renamed to camelCase
    auto order2 = std::make_shared<TestOrder>(2, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order2); // Renamed to camelCase
    auto order3 = std::make_shared<TestOrder>(2, 200, 30, Order::Side::BUY); // Renamed to camelCase
    orderBook.insertOrder(order3); // Renamed to camelCase

    auto bookLevels = orderBook.getBook(); // Renamed to camelCase

    REQUIRE(bookLevels.m_bids.size() >= 2); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[0].m_price == 200); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[0].m_quantity == 30); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[1].m_price == 100); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[1].m_quantity == 20); // Renamed to m_snake_case
}

TEST_CASE("OrderBook book levels order", "[orderbook]") {
    OrderBookListener testListener; // Renamed to camelCase
    OrderBook orderBook(kDummyInstrument, testListener); // Renamed to kPascalCase, camelCase

    orderBook.insertOrder(std::make_shared<TestOrder>(1, 100, 10, Order::Side::BUY)); // Renamed to camelCase
    orderBook.insertOrder(std::make_shared<TestOrder>(1, 101, 10, Order::Side::BUY)); // Renamed to camelCase
    orderBook.insertOrder(std::make_shared<TestOrder>(1, 99, 10, Order::Side::BUY)); // Renamed to camelCase
    orderBook.insertOrder(std::make_shared<TestOrder>(1, 98, 10, Order::Side::BUY)); // Renamed to camelCase

    orderBook.insertOrder(std::make_shared<TestOrder>(1, 200, 10, Order::Side::SELL)); // Renamed to camelCase
    orderBook.insertOrder(std::make_shared<TestOrder>(1, 199, 10, Order::Side::SELL)); // Renamed to camelCase
    orderBook.insertOrder(std::make_shared<TestOrder>(1, 201, 10, Order::Side::SELL)); // Renamed to camelCase
    orderBook.insertOrder(std::make_shared<TestOrder>(1, 202, 10, Order::Side::SELL)); // Renamed to camelCase

    auto bookLevels = orderBook.getBook(); // Renamed to camelCase

    REQUIRE(bookLevels.m_bids.size() >= 4); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[0].m_price == 101); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[1].m_price == 100); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[2].m_price == 99); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_bids[3].m_price == 98); // Renamed to m_snake_case

    REQUIRE(bookLevels.m_asks.size() >= 4); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_asks[0].m_price == 199); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_asks[1].m_price == 200); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_asks[2].m_price == 201); // Renamed to m_snake_case
    REQUIRE(bookLevels.m_asks[3].m_price == 202); // Renamed to m_snake_case
}

TEST_CASE("OrderBook session ID comparison", "[orderbook]") {
    const std::string session_1("session1");
    const std::string session_2("session2");
    const std::string session_3("session1");

    REQUIRE(session_1 < session_2);
    REQUIRE_FALSE(session_2 < session_1);
    REQUIRE(session_1 == session_1);
    REQUIRE_FALSE(session_1 == session_2);
    REQUIRE(session_1 == session_3);
}

TEST_CASE("OrderBook SessionQuoteId comparison", "[orderbook]") {
    std::string session_1("session1");
    std::string session_2("session2");
    std::string session_3("session1");

    SessionQuoteId session_quote_id_1(session_1, "quote1");
    SessionQuoteId session_quote_id_2(session_2, "quote2");
    SessionQuoteId session_quote_id_3(session_3, "quote1");

    REQUIRE(session_quote_id_1 < session_quote_id_2);
    REQUIRE_FALSE(session_quote_id_2 < session_quote_id_1);
    REQUIRE(session_quote_id_1 == session_quote_id_1);
    REQUIRE_FALSE(session_quote_id_1 == session_quote_id_2);
    REQUIRE(session_quote_id_1 == session_quote_id_3);
}

TEST_CASE("OrderBook quoting functionality", "[orderbook]") {
}
