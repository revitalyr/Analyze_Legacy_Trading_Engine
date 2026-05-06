#include <catch2/catch_test_macros.hpp>

#include "core/order.h"
#include "core/test.h"

TEST_CASE("OrderMap basic operations", "[ordermap]") {
    OrderMap orderMap; // Renamed to camelCase

    auto order = std::make_shared<TestOrder>(1, 100, 10, Order::Side::BUY); // Renamed to camelCase
    REQUIRE(orderMap.get(1) == nullptr); // Renamed to camelCase

    orderMap.add(order); // Renamed to camelCase
    REQUIRE(orderMap.get(1) == order); // Renamed to camelCase
    
    // Test that we can retrieve the order we just added
    auto retrievedOrder = orderMap.get(1); // Renamed to camelCase
    REQUIRE(retrievedOrder == order); // Renamed to camelCase
    REQUIRE(retrievedOrder != nullptr); // Renamed to camelCase
    REQUIRE(retrievedOrder->m_exchangeId == 1); // Renamed to m_snake_case
}
