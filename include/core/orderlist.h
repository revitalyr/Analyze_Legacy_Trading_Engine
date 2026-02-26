#pragma once
#include <stdexcept>
#include <memory>
#include "order.h"

// TODO add forward_iterator support so that friend class in not needed
class OrderList {
friend class OrderBook;
private:
    std::shared_ptr<Node> head = nullptr;
    std::shared_ptr<Node> tail = nullptr;
    F _price;
public:
    OrderList(F price) : _price(price) {}
    const F& price() const { return _price; }
    
    struct Iterator 
    {
        friend class OrderList;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::shared_ptr<Order>;
        using reference         = std::shared_ptr<Order>&;
        
        value_type operator*() const { 
            return current->order.lock(); 
        }
        
        // Prefix increment
        Iterator& operator++() { 
            current = current->next; 
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
    
    void pushback(std::shared_ptr<Order> order) {
        if (!order) return;
        
        auto node = order->node;
        node->order = order;
        
        if (head == nullptr) {
            head = node;
            tail = node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
    }
    
    void remove(std::shared_ptr<Order> order) {
        if (!order) return;
        
        auto node = order->node;
        if (node->order.expired()) {
            throw std::runtime_error("node is null on removal");
        }
        
        node->order.reset();
        
        if (head == node) {
            head = node->next;
        } 
        if (tail == node) {
            tail = node->prev;
        }
        if (node->prev) {
            node->prev->next = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        }
        
        // Clear node's links
        node->prev = nullptr;
        node->next = nullptr;
    }
    
    std::shared_ptr<Order> front() const {
        return (head == nullptr) ? nullptr : head->order.lock();
    }
    
    Iterator begin() const { 
        return Iterator(head); 
    }
    
    Iterator end() const { 
        return Iterator(nullptr); 
    }
};