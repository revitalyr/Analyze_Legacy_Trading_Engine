#define BOOST_TEST_MODULE ordermap_simple
#include <boost/test/included/unit_test.hpp>

#include "order.h"
#include "test.h"

BOOST_AUTO_TEST_CASE( ordermap_minimal ) {
    OrderMap map;
    
    // Test single order to avoid any recursion
    auto o = new TestOrder(1, 100, 10, Order::BUY);
    
    // Verify initial state
    BOOST_TEST(map.get(1)==nullptr);
    
    // Add order
    map.add(o);
    
    // Verify order was added
    BOOST_TEST(map.get(1)==o);
    BOOST_TEST(map.get(1)->exchangeId == 1);
    
    // Test that get returns null for non-existent order
    BOOST_TEST(map.get(999)==nullptr);
}
