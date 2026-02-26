#include <gtest/gtest.h>

#include "core/order.h"
#include "core/test.h"

TEST(OrderMapTest, OrderMapBasic) {
    OrderMap map;

    auto o = new TestOrder(1, 100, 10, Order::BUY);
    EXPECT_EQ(map.get(1), nullptr);

    map.add(o);
    EXPECT_EQ(map.get(1), o);
    
    // Test that we can retrieve the order we just added
    auto retrieved = map.get(1);
    EXPECT_EQ(retrieved, o);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->exchangeId, 1);
}
