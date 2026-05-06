#pragma once
#include <stdexcept>
#include <memory>
#include "order.h"

// TODO add forward_iterator support so that friend class in not needed
class OrderList {
friend class OrderBook;
private: // Internal state
    std::shared_ptr<Node> m_head = nullptr; // Head of the linked list of orders
    std::shared_ptr<Node> m_tail = nullptr; // Tail of the linked list of orders
    F m_price;
public:
    OrderList(F price) : m_price(price) {}
    const F& price() const { return m_price; }
    
    struct Iterator 
    {
        friend class OrderList;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::shared_ptr<Order>;
        using reference         = std::shared_ptr<Order>&;
        
        value_type operator*() const { 
            return current->m_order.lock(); 
        }
        
        // Prefix increment
        Iterator& operator++() { 
            current = current->m_next; 
            return *this;  
        }  
        
        bool operator== (const Iterator& other) const { 
            return current == other.current; 
        }
        
        operator bool() const { 
            return current != nullptr; 
        }
        
    private:
        Iterator(std::shared_ptr<Node> node) : current(node) {}
        std::shared_ptr<Node> current;
    };
    
    void pushBack(std::shared_ptr<Order> order) {
        if (!order) return;
        
        auto node = order->m_node;
        node->m_order = order;
        
        if (m_head == nullptr) {
            m_head = node;
            m_tail = node;
        } else {
            node->m_prev = m_tail;
            m_tail->m_next = node;
            m_tail = node;
        }
    }
    
    void remove(std::shared_ptr<Order> order) {
        if (!order) return;
        
        auto node = order->m_node;
        if (node->m_order.expired()) {
            throw std::runtime_error("node is null on removal");
        }
        
        node->m_order.reset();
        
        if (m_head == node) {
            m_head = node->m_next;
        } 
        if (m_tail == node) {
            m_tail = node->m_prev;
        }
        if (node->m_prev) {
            node->m_prev->m_next = node->m_next;
        }
        if (node->m_next) {
            node->m_next->m_prev = node->m_prev;
        }
        
        // Clear node's links
        node->m_prev = nullptr;
        node->m_next = nullptr;
    }
    
    std::shared_ptr<Order> front() const {
        return (m_head == nullptr) ? nullptr : m_head->m_order.lock();
    }
    
    Iterator begin() const { 
        return Iterator(m_head);
    }
    
    Iterator end() const { 
        return Iterator(nullptr);
    }
};