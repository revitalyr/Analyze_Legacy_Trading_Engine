#include <gtest/gtest.h>

#include "core/order.h"
#include "core/test.h"

TEST(OrderTest, OrderBasic) {
    auto order = TestOrder("myorder", 1, 100, 10, Order::BUY);
    ASSERT_EQ(order.orderId(), "myorder");

    auto order2 = TestOrder("myorder2", 1, 100, 10, Order::BUY);
    ASSERT_EQ(order2.orderId(), "myorder2");
}
