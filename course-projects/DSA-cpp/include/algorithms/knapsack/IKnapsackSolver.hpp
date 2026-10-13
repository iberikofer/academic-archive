#pragma once

#include "core/Item.hpp"
#include <vector>
#include <string>
#include <chrono>

namespace algorithms::knapsack {

struct KnapsackResult {
    std::vector<core::Item> selectedItems;
    int totalWeight = 0;
    int totalValue = 0;
    long long executionTimeMicroseconds = 0;
    size_t memoryBytesUsed = 0;
    std::string algorithmName;

    [[nodiscard]] double getApproximationRatio(int optimalValue) const noexcept {
        return optimalValue > 0 ? (static_cast<double>(totalValue) / optimalValue) * 100.0 : 100.0;
    }
};

class IKnapsackSolver {
public:
    virtual ~IKnapsackSolver() = default;

    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual KnapsackResult solve(
        const std::vector<core::Item>& availableItems,
        int maxCapacity
    ) = 0;
};

} // namespace algorithms::knapsack
