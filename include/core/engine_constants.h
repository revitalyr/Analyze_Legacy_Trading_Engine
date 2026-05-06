#pragma once

#include <string_view>

namespace EngineConstants {

// Error Messages
constexpr std::string_view kRecursionDepthExceeded = "Recursion depth limit exceeded in insertOrder";
constexpr std::string_view kOrderPointerNull = "Order pointer is null";
constexpr std::string_view kInvalidOrderQuantity = "Invalid order quantity: ";
constexpr std::string_view kOrderAlreadyOnList = "Order is already on a list";
constexpr std::string_view kOrderNotFound = "Order ID not found";
constexpr std::string_view kOrderCannotBeNull = "Order cannot be null";
constexpr std::string_view kExchangeErrorPrefix = "[EXCHANGE ERROR] ";
constexpr std::string_view kOrderInsertionFailed = "Order insertion failed: ";
constexpr std::string_view kExceptionInInsertOrder = "Exception in insertOrder: ";
constexpr std::string_view kOrderbookErrorPrefix = "[ORDERBOOK ERROR] ";
constexpr std::string_view kLegacyNulloptResult = "Legacy nullopt result";
constexpr std::string_view kPriceLevelDoesNotExist = "price level for order does not exist";

// Test-specific constants
constexpr std::string_view kTestSessionId = "session";
constexpr std::string_view kTestOrderIdPrefix = "order_";
constexpr std::string_view kTestQuoteId = "quote1";
constexpr std::string_view kTestInstrumentAAPL = "AAPL";
constexpr std::string_view kTestInstrumentGOOG = "GOOG";
constexpr std::string_view kTestInstrumentMSFT = "MSFT";

} // namespace EngineConstants