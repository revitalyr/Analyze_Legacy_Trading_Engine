#pragma once

/**
 * Global Constants for Trading Engine
 *
 * Consolidates all hardcoded constants used throughout the codebase.
 * Enables easy configuration and prevents magic numbers in code.
 */

#include <chrono>
#include <cfloat>
#include <cstddef>
#include "fixed.h" // For Price type

// ============================================================================
// BUFFER AND COLLECTION LIMITS
// ============================================================================

/** Maximum number of instrument order books that can be stored */
constexpr size_t kMaxInstruments = 1024;

/** Cache-line aligned node size for memory pools */
constexpr size_t kNodeSize = 64;

// ============================================================================
// RECURSION AND STACK PROTECTION
// ============================================================================

/** Maximum allowed recursion depth to prevent stack overflow */
constexpr int kMaxRecursionDepth = 50;

/** Maximum recursion depth before warning is issued */
constexpr int kWarningThreshold = 80;

// ============================================================================
// CIRCUIT BREAKER CONFIGURATION
// ============================================================================

/** Maximum number of failures before circuit breaker opens */
constexpr int kFailureThreshold = 10;

/** Time interval during which circuit breaker remains open */
constexpr std::chrono::seconds kCooldownPeriod{30};

/** Interval after which the recursion depth counter is reset */
constexpr std::chrono::seconds kResetInterval{60};

// ============================================================================
// DEFAULT VALUES
// ============================================================================

/** Default instrument symbol for testing */
constexpr const char* kDefaultInstrument = "SYM1";

/** Price value representing market buy (large finite value for fixed-point safety) */
constexpr Price kMarketBuyPrice = Price(1e9);

/** Price value representing market sell (large negative value for fixed-point safety) */
constexpr Price kMarketSellPrice = Price(-1e9);

// ============================================================================
// STRING INTERNER CONSTANTS
// ============================================================================

/** Invalid string ID for the string interner - indicates no string is interned */
constexpr uint32_t kInvalidStringId = 0;
