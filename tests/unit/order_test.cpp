#include <catch2/catch_test_macros.hpp>

#include "core/order.h"
#include "core/test.h"

TEST_CASE("Order basic creation", "[order]") {
    auto order1 = TestOrder("myorder", 1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    REQUIRE(order1.orderId() == "myorder"); // Renamed to camelCase

    auto order2 = TestOrder("myorder2", 1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    REQUIRE(order2.orderId() == "myorder2"); // Renamed to camelCase
}
