#include <iostream>

// Minimal test to isolate stack overflow
struct MinimalOrder {
    long exchangeId;
    MinimalOrder* next;
    
    MinimalOrder(long id) : exchangeId(id), next(nullptr) {
        std::cout << "Creating order " << id << std::endl;
    }
};

class MinimalOrderMap {
private:
    static const int TABLE_SIZE = 1000;
    MinimalOrder* table[TABLE_SIZE];
    
public:
    MinimalOrderMap() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }
    
    void add(MinimalOrder* order) {
        if (!order) return;
        
        int bucket = order->exchangeId % TABLE_SIZE;
        order->next = table[bucket];
        table[bucket] = order;
        
        std::cout << "Added order " << order->exchangeId << " to bucket " << bucket << std::endl;
    }
    
    MinimalOrder* get(long exchangeId) {
        int bucket = exchangeId % TABLE_SIZE;
        MinimalOrder* order = table[bucket];
        
        std::cout << "Looking for order " << exchangeId << " in bucket " << bucket << std::endl;
        
        while (order != nullptr) {
            if (order->exchangeId == exchangeId) {
                std::cout << "Found order " << exchangeId << std::endl;
                return order;
            }
            order = order->next;
        }
        
        std::cout << "Order " << exchangeId << " not found" << std::endl;
        return nullptr;
    }
};

int main() {
    std::cout << "Starting minimal OrderMap test..." << std::endl;
    
    try {
        MinimalOrderMap map;
        std::cout << "Created OrderMap" << std::endl;
        
        auto o = new MinimalOrder(1);
        std::cout << "Created order" << std::endl;
        
        auto result1 = map.get(1);
        std::cout << "First get: " << (result1 == nullptr ? "nullptr" : "found") << std::endl;
        
        map.add(o);
        std::cout << "Added order to map" << std::endl;
        
        auto result2 = map.get(1);
        std::cout << "Second get: " << (result2 == nullptr ? "nullptr" : "found") << std::endl;
        
        if (result2) {
            std::cout << "Retrieved order ID: " << result2->exchangeId << std::endl;
        }
        
        std::cout << "Test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
