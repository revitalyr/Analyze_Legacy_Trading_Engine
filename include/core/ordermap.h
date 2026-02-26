#pragma once

#include "order.h"
#include <memory>
#include <vector>
#include <atomic>

/**
 * @brief lock-free map of external order ID -> Order
 */
class OrderMap {
    static const int TABLE_SIZE = 1000000;
    std::atomic<std::shared_ptr<Order>> table[TABLE_SIZE];
public:
    void add(std::shared_ptr<Order> order) {
        if (!order) return;
        
        int bucket = order->exchangeId % TABLE_SIZE;
        std::shared_ptr<Order> expected = table[bucket].load();
        order->next = expected;
        while (!table[bucket].compare_exchange_weak(expected, order)) {
            order->next = expected;
        }
    }
    
    std::shared_ptr<Order> get(long exchangeId) const {
        int bucket = exchangeId % TABLE_SIZE;
        std::shared_ptr<Order> order = table[bucket].load();
        while (order != nullptr) {
            if (order->exchangeId == exchangeId) return order;
            order = order->next;
        }
        return nullptr;
    }
    
    std::vector<std::shared_ptr<const Order>> all() const {
        std::vector<std::shared_ptr<const Order>> orders;
        for (int i = 0; i < TABLE_SIZE; i++) {
            std::shared_ptr<Order> order = table[i].load();
            while (order != nullptr) {
                orders.push_back(order);
                order = order->next;
            }
        }
        return orders;
    }
    
    std::vector<std::string> instruments() const {
        std::vector<std::string> result;
        for (int i = 0; i < TABLE_SIZE; i++) {
            std::shared_ptr<Order> order = table[i].load();
            while (order != nullptr) {
                result.push_back(order->instrument);
                order = order->next;
            }
        }
        return result;
    }
};