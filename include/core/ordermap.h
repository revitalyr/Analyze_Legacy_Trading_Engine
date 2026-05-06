#pragma once

#include "order.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

/**
 * @brief High-performance hash map of exchange ID -> Order
 * Uses unordered_map with shared_mutex for fast concurrent access
 */
class OrderMap {
private: // Internal state
    mutable std::shared_mutex m_mutex; // Mutex for thread-safe access to the map
    std::unordered_map<ExchangeId, std::shared_ptr<Order>> m_map; // The underlying hash map

public:
    OrderMap() {
        m_map.reserve(100000); // Reserve space for typical order count
    }

    /**
     * Add order to map
     */
    void add(std::shared_ptr<Order> order) {
        if (!order) return;

        std::unique_lock lock(m_mutex);
        m_map[order->m_exchangeId] = std::move(order);
    }
    
    /**
     * Get order by exchange ID
     */
    std::shared_ptr<Order> get(ExchangeId exchangeId) const {
        std::shared_lock lock(m_mutex);
        auto it = m_map.find(exchangeId);
        return (it != m_map.end()) ? it->second : nullptr;
    }
    
    /**
     * Remove order by exchange ID
     */
    void remove(ExchangeId exchangeId) {
        std::unique_lock lock(m_mutex);
        m_map.erase(exchangeId);
    }
    
    /**
     * Get all orders
     */
    std::vector<std::shared_ptr<const Order>> all() const {
        std::shared_lock lock(m_mutex);
        std::vector<std::shared_ptr<const Order>> orders;
        orders.reserve(m_map.size());
        
        for (const auto& [id, order] : m_map) {
            orders.push_back(order);
        }
        return orders;
    }
    
    /**
     * Get all unique instruments
     */
    std::vector<std::string> instruments() const {
        std::shared_lock lock(m_mutex); // Renamed to m_snake_case
        std::unordered_set<std::string> unique_instruments;
        
        for (const auto& [id, order] : m_map) {
            unique_instruments.insert(order->instrument());
        }
        
        return std::vector<std::string>(unique_instruments.begin(), unique_instruments.end()); // Renamed to camelCase
    }
    
    /**
     * Get order count
     */
    size_t size() const {
        std::shared_lock lock(m_mutex); // Renamed to m_snake_case
        return m_map.size(); // Renamed to m_snake_case
    }
    
    /**
     * Clear all orders
     */
    void clear() {
        std::unique_lock lock(m_mutex); // Renamed to m_snake_case
        m_map.clear(); // Renamed to m_snake_case
    }
    
    /**
     * Reserve space for expected order count
     */
    void reserve(size_t n) {
        std::unique_lock lock(m_mutex); // Renamed to m_snake_case
        m_map.reserve(n); // Renamed to m_snake_case
    }
};