#include <gtest/gtest.h>

#include "core/orderbook.h"
#include "core/test.h"

TEST(OrderListTest, OrderListSimple) {
    OrderList orderlist(100);
    EXPECT_TRUE(orderlist.begin() == orderlist.end());

    auto o = new TestOrder(1, 100, 10, Order::BUY);
    orderlist.pushback(o);

    EXPECT_TRUE(orderlist.begin() != orderlist.end());
    EXPECT_EQ(*(orderlist.begin()), o);
}

TEST(OrderListTest, OrderListIterator) {
    OrderList list(100);
    EXPECT_TRUE(list.begin() == list.end());

    auto o = new TestOrder(1, 100, 10, Order::BUY);
    list.pushback(o);

    EXPECT_TRUE(list.begin() != list.end());
    EXPECT_EQ(*(list.begin()), o);

    auto o2 = new TestOrder(2, 100, 10, Order::BUY);
    list.pushback(o2);

    EXPECT_EQ(*(list.begin()), o);

    auto itr = list.begin();
    ++itr;
    EXPECT_EQ(*itr, o2);
    ++itr;
    EXPECT_TRUE(itr == list.end());
}
