#pragma once 

#include <stdexcept>
#include <cstddef>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "orderbook.h"

#define MAX_INSTRUMENTS 1024

/** Book is a lock-free map of instrument -> OrderBook */
class BookMap {
    std::atomic<std::shared_ptr<OrderBook>> table[MAX_INSTRUMENTS];
public:
    BookMap() {
        for(int i = 0; i < MAX_INSTRUMENTS; i++) {
            table[i].store(nullptr);
        }
    }
    
    std::shared_ptr<OrderBook> getOrCreate(const std::string_view& instrument, OrderBookListener& listener) {
        auto hash = std::hash<std::string_view>{}(instrument);
        const auto start = hash % MAX_INSTRUMENTS;
        auto book = table[start].load();
        if (book != nullptr && book->instrument == instrument) return book;
        
        auto new_book = std::make_shared<OrderBook>(std::string(instrument), listener);
        auto index = start;
        while (true) {
            if (book != nullptr) {
                index = (index + 1) % MAX_INSTRUMENTS;
                if (index == start) throw std::runtime_error("no room in books map");
                book = table[index].load();
                if (book != nullptr && book->instrument == instrument) return book;
            } else {
                if (table[index].compare_exchange_weak(book, new_book)) {
                    return new_book;
                }
            }
        }
    }
    
    std::shared_ptr<OrderBook> get(const std::string_view& instrument) const {
        auto hash = std::hash<std::string_view>{}(instrument);
        const auto start = hash % MAX_INSTRUMENTS;
        auto book = table[start].load();
        if (book != nullptr && book->instrument == instrument) return book;
        
        auto index = start;
        while (true) {
            if (book != nullptr) {
                index = (index + 1) % MAX_INSTRUMENTS;
                if (index == start) return nullptr;
                book = table[index].load();
                if (book != nullptr && book->instrument == instrument) return book;
            } else {
                index = (index + 1) % MAX_INSTRUMENTS;
                if (index == start) return nullptr;
                book = table[index].load();
            }
        }
    }
    
    std::vector<std::string> instruments() const {
        std::vector<std::string> result;
        for (int i = 0; i < MAX_INSTRUMENTS; i++) {
            auto book = table[i].load();
            if (book != nullptr) {
                result.push_back(book->instrument);
            }
        }
        return result;
    }
};