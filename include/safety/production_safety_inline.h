#pragma once

/**
 * Production Safety Module - Inline Implementation
 * 
 * Runtime protections for cpp_orderbook trading engine.
 * Prevents stack overflow during high-frequency trading operations.
 */

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>

class ProductionSafety {
private:
    static inline std::atomic<bool> safety_enabled{true};
    static inline std::atomic<int> failure_count{0};
    static inline std::atomic<std::chrono::steady_clock::time_point> last_failure_time{
        std::chrono::steady_clock::now()
    };
    
    static constexpr int MAX_RECURSION_DEPTH = 50;
    static constexpr std::chrono::seconds RESET_INTERVAL{1};
    static constexpr std::chrono::seconds COOLDOWN_PERIOD{30};
    static constexpr int FAILURE_THRESHOLD = 10;

public:
    // ====================================================================
    // Thread-local state (единый источник истины для глубины рекурсии)
    // ====================================================================
    struct ThreadLocalState {
        int recursion_depth = 0;
        std::chrono::steady_clock::time_point last_reset = std::chrono::steady_clock::now();
    };
    
    [[nodiscard]] static inline ThreadLocalState& threadState() noexcept {
        thread_local ThreadLocalState state;
        return state;
    }

    // ====================================================================
    // Configuration
    // ====================================================================
    static inline void enableSafety(bool enabled = true) noexcept {
        safety_enabled.store(enabled, std::memory_order_relaxed);
    }
    
    [[nodiscard]] static inline bool isTestMode() noexcept {
        return !safety_enabled.load(std::memory_order_relaxed);
    }

    // ====================================================================
    // Recursion protection
    // ====================================================================
    [[nodiscard]] static inline bool enterCriticalOperation() noexcept {
        if (isTestMode()) return true;
        
        auto& state = threadState();
        const auto now = std::chrono::steady_clock::now();
        
        // Периодический сброс для предотвращения permanent lockout
        if (now - state.last_reset > RESET_INTERVAL) {
            state.recursion_depth = 0;
            state.last_reset = now;
        }
        
        if (++state.recursion_depth > MAX_RECURSION_DEPTH) {
            state.recursion_depth = 0;  // сброс для восстановления
            return false;
        }
        return true;
    }
    
    static inline void exitCriticalOperation() noexcept {
        if (isTestMode()) return;
        
        auto& state = threadState();
        if (state.recursion_depth > 0) {
            --state.recursion_depth;
        }
    }

    // ====================================================================
    // RAII Guard
    // ====================================================================
    class StackGuard {
    public:
        StackGuard() noexcept : valid_(enterCriticalOperation()) {}
        
        ~StackGuard() noexcept {
            if (valid_) {
                exitCriticalOperation();
            }
        }
        
        // Не копируется, не перемещается
        StackGuard(const StackGuard&) = delete;
        StackGuard& operator=(const StackGuard&) = delete;
        StackGuard(StackGuard&&) = delete;
        StackGuard& operator=(StackGuard&&) = delete;
        
        [[nodiscard]] bool isValid() const noexcept { return valid_; }
        
    private:
        bool valid_;
    };

    // ====================================================================
    // Circuit breaker
    // ====================================================================
    [[nodiscard]] static inline bool circuitBreakerAllow() noexcept {
        if (!safety_enabled.load(std::memory_order_relaxed)) return true;
        
        const int failures = failure_count.load(std::memory_order_relaxed);
        if (failures > FAILURE_THRESHOLD) {
            const auto last = last_failure_time.load(std::memory_order_relaxed);
            const auto now = std::chrono::steady_clock::now();
            
            if (now - last > COOLDOWN_PERIOD) {
                failure_count.store(0, std::memory_order_relaxed);
                return true;
            }
            return false;  // Circuit open
        }
        return true;
    }
    
    static inline void recordFailure() noexcept {
        failure_count.fetch_add(1, std::memory_order_relaxed);
        last_failure_time.store(std::chrono::steady_clock::now(), std::memory_order_relaxed);
    }
    
    static inline void recordSuccess() noexcept {
        int expected = failure_count.load(std::memory_order_relaxed);
        while (expected > 0) {
            if (failure_count.compare_exchange_weak(
                    expected, expected - 1, 
                    std::memory_order_relaxed, std::memory_order_relaxed)) {
                break;
            }
        }
    }
};

// ====================================================================
// Macros (безопасные для Boost.Test)
// ====================================================================

#define CRITICAL_OPERATION_GUARD \
    ProductionSafety::StackGuard __stack_guard_local; \
    if (!__stack_guard_local.isValid()) { \
        ProductionSafety::recordFailure(); \
        return; /* graceful early return instead of throw */ \
    }

#define CIRCUIT_BREAKER_GUARD \
    if (!ProductionSafety::circuitBreakerAllow()) { \
        ProductionSafety::recordFailure(); \
        return; /* graceful early return */ \
    }