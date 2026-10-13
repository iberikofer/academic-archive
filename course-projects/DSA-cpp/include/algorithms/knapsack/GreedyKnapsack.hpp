#pragma once

#include "IKnapsackSolver.hpp"
#include <chrono>
#include <vector>
#include <algorithm>

namespace algorithms::knapsack {

class GreedyKnapsack : public IKnapsackSolver {
public:
    [[nodiscard]] std::string getName() const override {
        return "Greedy Knapsack (Value/Weight)";
    }

    [[nodiscard]] KnapsackResult solve(
        const std::vector<core::Item>& availableItems,
        int maxCapacity
    ) override {
        auto startTime = std::chrono::high_resolution_clock::now();

        KnapsackResult result;
        result.algorithmName = getName();

        if (availableItems.empty() || maxCapacity <= 0) {
            auto endTime = std::chrono::high_resolution_clock::now();
            result.executionTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
            return result;
        }

        // Copy items to sort by density ratio (Value / Weight) descending
        std::vector<core::Item> sortedItems = availableItems;
        result.memoryBytesUsed = sortedItems.size() * sizeof(core::Item);

        std::sort(sortedItems.begin(), sortedItems.end(), [](const core::Item& a, const core::Item& b) {
            double densityA = a.getValueDensity();
            double densityB = b.getValueDensity();
            if (std::abs(densityA - densityB) > 1e-6) {
                return densityA > densityB;
            }
            return a.value > b.value; // Tie-breaker: higher absolute value
        });

        int remainingCapacity = maxCapacity;
        for (const auto& item : sortedItems) {
            if (item.weight <= remainingCapacity) {
                result.selectedItems.push_back(item);
                result.totalWeight += item.weight;
                result.totalValue += item.value;
                remainingCapacity -= item.weight;
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

        return result;
    }
};

} // namespace algorithms::knapsack
