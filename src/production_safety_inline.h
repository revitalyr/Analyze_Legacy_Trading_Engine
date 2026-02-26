#pragma once

// Production safety inline functions for legacy compatibility
#include <atomic>
#include <chrono>

// Simple production safety for legacy compatibility
namespace ProductionSafety {
    inline bool enterCriticalOperation() {
        return true; // Simple implementation for now
    }
    
    inline void exitCriticalOperation() {
        // Simple implementation for now
    }
    
    inline bool isEnabled() {
        return true;
    }
}

// Legacy macros
#define PRODUCTION_CRITICAL_GUARD \
    if (!ProductionSafety::enterCriticalOperation()) { return; }

#define PRODUCTION_CIRCUIT_BREAKER \
    if (!ProductionSafety::isEnabled()) { return; }
