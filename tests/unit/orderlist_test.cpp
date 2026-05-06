#include <catch2/catch_test_macros.hpp>

#include "core/orderbook.h"
#include "core/test.h"

TEST_CASE("OrderList basic operations", "[orderlist]") {
    OrderList orderList(100); // Renamed to camelCase
    REQUIRE(orderList.begin() == orderList.end()); // Renamed to camelCase

    auto order = std::make_shared<TestOrder>(1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderList.pushBack(order); // Renamed to camelCase

    REQUIRE(orderList.begin() != orderList.end()); // Renamed to camelCase
    REQUIRE(*(orderList.begin()) == order); // Renamed to camelCase
}

TEST_CASE("OrderList iterator", "[orderlist]") {
    OrderList orderList(100); // Renamed to camelCase
    REQUIRE(orderList.begin() == orderList.end()); // Renamed to camelCase

    auto order = std::make_shared<TestOrder>(1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderList.pushBack(order); // Renamed to camelCase

    REQUIRE(orderList.begin() != orderList.end()); // Renamed to camelCase
    REQUIRE(*(orderList.begin()) == order); // Renamed to camelCase

    auto order2 = std::make_shared<TestOrder>(2, 100, 10, Order::Side::BUY); // Renamed to camelCase
    orderList.pushBack(order2); // Renamed to camelCase

    REQUIRE(*(orderList.begin()) == order); // Renamed to camelCase

    auto it = orderList.begin(); // Renamed to camelCase
    ++it;
    REQUIRE(*it == order2); // Renamed to camelCase
    ++it;
    REQUIRE(it == orderList.end()); // Renamed to camelCase
}
