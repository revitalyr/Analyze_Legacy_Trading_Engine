#include <iostream>
#include <string>

// Test to isolate Order constructor recursion
class TestOrder {
public:
    long exchangeId;
    TestOrder* next;
    
    TestOrder(long id, double price, int quantity, int side) 
        : exchangeId(id), next(nullptr) {
        std::cout << "TestOrder constructor called with id=" << id << std::endl;
        std::cout << "  exchangeId set to: " << exchangeId << std::endl;
        std::cout << "  next set to: " << (next == nullptr ? "nullptr" : "not null") << std::endl;
    }
};

class TestOrderMap {
private:
    static const int TABLE_SIZE = 1000;
    TestOrder* table[TABLE_SIZE];
    
public:
    TestOrderMap() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
        std::cout << "TestOrderMap constructor completed" << std::endl;
    }
    
    void add(TestOrder* order) {
        std::cout << "TestOrderMap::add called with order->exchangeId=" << order->exchangeId << std::endl;
        
        int bucket = order->exchangeId % TABLE_SIZE;
        std::cout << "  Calculated bucket: " << bucket << std::endl;
        
        order->next = table[bucket];
        table[bucket] = order;
        
        std::cout << "  Order added to bucket " << bucket << std::endl;
    }
    
    TestOrder* get(long exchangeId) {
        std::cout << "TestOrderMap::get called with exchangeId=" << exchangeId << std::endl;
        
        int bucket = exchangeId % TABLE_SIZE;
        std::cout << "  Looking in bucket: " << bucket << std::endl;
        
        TestOrder* order = table[bucket];
        int count = 0;
        const int MAX_ITERATIONS = 100;
        
        while (order != nullptr && count < MAX_ITERATIONS) {
            std::cout << "  Checking order " << order->exchangeId << std::endl;
            
            if (order->exchangeId == exchangeId) {
                std::cout << "  Found matching order!" << std::endl;
                return order;
            }
            order = order->next;
            count++;
            
            if (count >= MAX_ITERATIONS) {
                std::cout << "  ERROR: Too many iterations, possible infinite loop!" << std::endl;
                break;
            }
        }
        
        std::cout << "  Order not found" << std::endl;
        return nullptr;
    }
};

int main() {
    std::cout << "=== TestOrder Constructor Test ===" << std::endl;
    
    try {
        std::cout << "Creating TestOrderMap..." << std::endl;
        TestOrderMap map;
        
        std::cout << "\nCreating TestOrder..." << std::endl;
        auto o = new TestOrder(1, 100.0, 10, 0);
        
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
