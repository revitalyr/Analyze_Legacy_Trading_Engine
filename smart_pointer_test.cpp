#include "core/exchange.h"
#include "core/test.h"
#include <iostream>
#include <memory>

// Simple functional test for smart pointer functionality
int main() {
    std::cout << "Testing smart pointer refactoring..." << std::endl;
    
    try {
        // Test 1: Create Exchange
        Exchange exchange;
        std::cout << "✅ Exchange created successfully" << std::endl;
        
        // Test 2: Create orders using factory methods
        auto order1 = TestOrder::create(1, 100.0, 10, Order::BUY);
        auto order2 = TestOrder::create(2, 101.0, 5, Order::SELL);
        
        if (order1 && order2) {
            std::cout << "✅ Smart pointer orders created successfully" << std::endl;
            std::cout << "   Order1: ID=" << order1->exchangeId << ", Price=" << order1->price() << std::endl;
            std::cout << "   Order2: ID=" << order2->exchangeId << ", Price=" << order2->price() << std::endl;
        } else {
            std::cout << "❌ Failed to create orders" << std::endl;
            return 1;
        }
        
        // Test 3: Test shared_ptr copying
        auto order1_copy = order1;
        if (order1_copy && order1_copy.get() == order1.get()) {
            std::cout << "✅ Shared pointer copying works correctly" << std::endl;
        } else {
            std::cout << "❌ Shared pointer copying failed" << std::endl;
            return 1;
        }
        
        // Test 4: Test weak_ptr functionality
        std::weak_ptr<Order> weak_order = order1;
        auto shared_from_weak = weak_order.lock();
        if (shared_from_weak && shared_from_weak == order1) {
            std::cout << "✅ Weak pointer functionality works correctly" << std::endl;
        } else {
            std::cout << "❌ Weak pointer functionality failed" << std::endl;
            return 1;
        }
        
        // Test 5: Test order destruction
        order1.reset();
        auto after_reset = weak_order.lock();
        if (!after_reset) {
            std::cout << "✅ Order destruction and weak pointer invalidation works" << std::endl;
        } else {
            std::cout << "❌ Order destruction test failed" << std::endl;
            return 1;
        }
        
        std::cout << "\n🎉 All smart pointer tests passed!" << std::endl;
        std::cout << "Memory management is working correctly with smart pointers." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❌ Unknown exception occurred" << std::endl;
        return 1;
    }
}
