#include <iostream>
#include <string>

// Test Fixed constructor isolation
class SimpleFixed {
public:
    double value;
    
    SimpleFixed(double v) : value(v) {
        std::cout << "SimpleFixed constructor called with " << v << std::endl;
    }
    
    SimpleFixed(int v) : value(static_cast<double>(v)) {
        std::cout << "SimpleFixed constructor called with int " << v << std::endl;
    }
};

class TestOrderWithFixed {
public:
    long exchangeId;
    TestOrderWithFixed* next;
    SimpleFixed price;
    SimpleFixed quantity;
    
    TestOrderWithFixed(long id, SimpleFixed p, SimpleFixed q, int side) 
        : exchangeId(id), next(nullptr), price(p), quantity(q) {
        std::cout << "TestOrderWithFixed constructor:" << std::endl;
        std::cout << "  exchangeId: " << exchangeId << std::endl;
        std::cout << "  price: " << price.value << std::endl;
        std::cout << "  quantity: " << quantity.value << std::endl;
    }
};

class TestOrderMapWithFixed {
private:
    static const int TABLE_SIZE = 1000;
    TestOrderWithFixed* table[TABLE_SIZE];
    
public:
    TestOrderMapWithFixed() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
        std::cout << "TestOrderMapWithFixed constructor completed" << std::endl;
    }
    
    void add(TestOrderWithFixed* order) {
        std::cout << "TestOrderMapWithFixed::add called" << std::endl;
        
        int bucket = order->exchangeId % TABLE_SIZE;
        order->next = table[bucket];
        table[bucket] = order;
        
        std::cout << "  Order added to bucket " << bucket << std::endl;
    }
    
    TestOrderWithFixed* get(long exchangeId) {
        std::cout << "TestOrderMapWithFixed::get called" << std::endl;
        
        int bucket = exchangeId % TABLE_SIZE;
        TestOrderWithFixed* order = table[bucket];
        
        while (order != nullptr) {
            if (order->exchangeId == exchangeId) {
                return order;
            }
            order = order->next;
        }
        return nullptr;
    }
};

int main() {
    std::cout << "=== TestOrderWithFixed Constructor Test ===" << std::endl;
    
    try {
        std::cout << "Creating TestOrderMapWithFixed..." << std::endl;
        TestOrderMapWithFixed map;
        
        std::cout << "\nCreating SimpleFixed objects..." << std::endl;
        SimpleFixed price(100.0);
        SimpleFixed quantity(10);
        
        std::cout << "\nCreating TestOrderWithFixed..." << std::endl;
        auto o = new TestOrderWithFixed(1, price, quantity, 0);
        
        std::cout << "\nFirst get (should be nullptr)..." << std::endl;
        auto result1 = map.get(1);
        std::cout << "Result1: " << (result1 == nullptr ? "nullptr" : "found") << std::endl;
        
        std::cout << "\nAdding order to map..." << std::endl;
        map.add(o);
        
        std::cout << "\nSecond get (should find order)..." << std::endl;
        auto result2 = map.get(1);
        std::cout << "Result2: " << (result2 == nullptr ? "nullptr" : "found") << std::endl;
        
        if (result2) {
            std::cout << "Retrieved order ID: " << result2->exchangeId << std::endl;
        }
        
        std::cout << "\nTest completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
