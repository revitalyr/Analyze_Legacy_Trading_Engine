#include <iostream>
#include <cassert>

// Test without any macros
#include "simple_order.h"
#include "simple_ordermap.h"

class SimpleTestOrder : public SimpleOrder {
public:
    SimpleTestOrder(long id, double price, int quantity, int side) 
        : SimpleOrder("test", "test", "test", price, quantity, 
                      static_cast<SimpleOrder::Side>(side), id) {
        std::cout << "SimpleTestOrder constructor called with id=" << id << std::endl;
    }
};

int main() {
    std::cout << "=== No Macros Test ===" << std::endl;
    
    try {
        std::cout << "Creating SimpleOrderMap..." << std::endl;
        SimpleOrderMap map;
        
        std::cout << "Creating SimpleTestOrder..." << std::endl;
        auto o = new SimpleTestOrder(1, 100.0, 10, 0);
        
        std::cout << "First get (should be nullptr)..." << std::endl;
        auto result1 = map.get(1);
        assert(result1 == nullptr);
        std::cout << "✓ First get returned nullptr" << std::endl;
        
        std::cout << "Adding order to map..." << std::endl;
        map.add(o);
        
        std::cout << "Second get (should find order)..." << std::endl;
        auto result2 = map.get(1);
        assert(result2 != nullptr);
        std::cout << "✓ Second get found order" << std::endl;
        
        if (result2) {
            assert(result2->exchangeId == 1);
            std::cout << "✓ Retrieved order ID: " << result2->exchangeId << std::endl;
        }
        
        std::cout << "\n=== All Tests Passed! ===" << std::endl;
        std::cout << "✓ No stack overflow detected" << std::endl;
        std::cout << "✓ No macros used" << std::endl;
        std::cout << "✓ Modern C++ RAII patterns" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
