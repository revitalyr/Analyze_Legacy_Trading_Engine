#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

#include "constants.h"

namespace orderbook {

class Order; // Forward declaration

/**
 * Lock-free memory pool for fixed-size objects.
 * Uses intrusive linked list for free nodes.
 */
template<typename T, size_t BlockSize = 4096>
class MemoryPool {
public:
    // Node size is determined at runtime for incomplete types
    // Uses NODE_SIZE from constants.h
    
    struct PoolNode {
        alignas(64) char object_storage[NODE_SIZE]; // Aligned storage for object
        std::atomic<PoolNode*> next{nullptr};
        
        T* object() { return reinterpret_cast<T*>(object_storage); }
    };

private:
    struct alignas(64) Block { // Represents a block of memory // Renamed to PascalCase
        char m_data[BlockSize * sizeof(PoolNode)]; // Raw memory for nodes // Renamed to m_snake_case
        Block* m_next{nullptr}; // Pointer to the next block in the chain // Renamed to m_snake_case
    };

    std::vector<std::unique_ptr<Block>> m_blocks; // Renamed to m_snake_case
    std::atomic<PoolNode*> m_freeList{nullptr}; // Renamed to m_snake_case
    std::atomic<size_t> m_allocatedCount{0}; // Renamed to m_snake_case
    std::atomic<size_t> m_capacity{0}; // Renamed to m_snake_case

public:
    MemoryPool() = default;
    ~MemoryPool() = default;

    // Disable copy/move
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    /**
     * Pre-allocate memory blocks
     */
    void reserve(size_t n) {
        size_t current = capacity_.load(std::memory_order_relaxed);
        if (n <= current) return;

        size_t blocks_needed = (n - current + BlockSize - 1) / BlockSize;
        
        for (size_t b = 0; b < blocks_needed; ++b) {
            auto block = std::make_unique<Block>(); // Renamed to camelCase
            PoolNode* nodes = reinterpret_cast<PoolNode*>(block->m_data); // Renamed to m_snake_case
            
            // Link nodes in this block
            for (size_t i = 0; i < BlockSize - 1; ++i) {
                nodes[i].next.store(&nodes[i + 1], std::memory_order_relaxed);
            }
            nodes[BlockSize - 1].next.store(nullptr, std::memory_order_relaxed);

            // Add to free list (lock-free push)
            PoolNode* block_head = &nodes[0];
            PoolNode* old_head = m_freeList.load(std::memory_order_relaxed); // Renamed to m_snake_case
            
            do {
                nodes[BlockSize - 1].next.store(old_head, std::memory_order_relaxed);
            } while (!m_freeList.compare_exchange_weak( // Renamed to m_snake_case
                old_head, block_head, // Renamed to camelCase
                std::memory_order_release,
                std::memory_order_relaxed));

            blocks_.push_back(std::move(block));
        }

        capacity_.fetch_add(blocks_needed * BlockSize, std::memory_order_relaxed);
    }

    /**
     * Allocate object from pool
     */
    T* allocate() { // Renamed to camelCase
        PoolNode* node = pop_free();
        if (!node) {
            return nullptr; // Pool exhausted
        }
        
        allocated_count_.fetch_add(1, std::memory_order_relaxed);
        return reinterpret_cast<T*>(node);
    }

    /**
     * Deallocate object back to pool
     */
    void deallocate(T* ptr) { // Renamed to camelCase
        if (!ptr) return;
        
        // Explicitly call destructor
        ptr->~T();
        
        PoolNode* node = reinterpret_cast<PoolNode*>(ptr);
        push_free(node);
        
        allocated_count_.fetch_sub(1, std::memory_order_relaxed);
    }

    /**
     * Construct object in place
     */
    template <typename... Args>
    T* construct(Args&&... args) {
        T* ptr = allocate();
        if (ptr) {
            new (ptr) T(std::forward<Args>(args)...);
        }
        return ptr;
    }

    size_t allocated_count() const {
        return m_allocatedCount.load(std::memory_order_relaxed); // Renamed to m_snake_case
    }

    size_t capacity() const {
        return m_capacity.load(std::memory_order_relaxed); // Renamed to m_snake_case
    }

private:
    PoolNode* popFree() { // Renamed to camelCase
        PoolNode* head = m_freeList.load(std::memory_order_acquire); // Renamed to m_snake_case
        
        while (head) {
            PoolNode* next = head->next.load(std::memory_order_relaxed);
            if (m_freeList.compare_exchange_weak( // Renamed to m_snake_case
                head, next,
                std::memory_order_acquire,
                std::memory_order_relaxed)) {
                return head;
            }
            // CAS failed, retry with new head
        }
        
        return nullptr;
    }

    void pushFree(PoolNode* node) { // Renamed to camelCase
        node->next.store(nullptr, std::memory_order_relaxed);
        
        PoolNode* old_head = m_freeList.load(std::memory_order_relaxed); // Renamed to m_snake_case
        
        do { // Renamed to camelCase
            node->next.store(old_head, std::memory_order_relaxed);
        } while (!m_freeList.compare_exchange_weak( // Renamed to m_snake_case
            old_head, node, // Renamed to camelCase
            std::memory_order_release,
            std::memory_order_relaxed));
    }
};

/**
 * Global order pool - singleton for the application
 */
class OrderPool {
public:
    static MemoryPool<class Order>& instance() {
        static MemoryPool<class Order> pool;
        return pool;
    }

    static void reserve(size_t n) {
        instance().reserve(n);
    }
};

} // namespace orderbook
