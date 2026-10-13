#pragma once

#include "IKnapsackSolver.hpp"
#include <chrono>
#include <vector>
#include <algorithm>

namespace algorithms::knapsack {

class DPKnapsack : public IKnapsackSolver {
public:
    [[nodiscard]] std::string getName() const override {
        return "0/1 Knapsack DP (Exact)";
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

        const size_t n = availableItems.size();

        // Calculate total weight of all items
        int totalItemsWeight = 0;
        for (const auto& item : availableItems) {
            totalItemsWeight += item.weight;
        }

        // If all items fit, trivial optimal solution without allocating DP table
        if (totalItemsWeight <= maxCapacity) {
            result.selectedItems = availableItems;
            result.totalWeight = totalItemsWeight;
            for (const auto& item : availableItems) {
                result.totalValue += item.value;
            }
            result.memoryBytesUsed = 0;
            auto endTime = std::chrono::high_resolution_clock::now();
            result.executionTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
            return result;
        }

        // DP capacity never needs to exceed total items weight or safe memory limit
        constexpr int MAX_SAFE_CAPACITY = 20000;
        const int cap = std::min(maxCapacity, std::min(totalItemsWeight, MAX_SAFE_CAPACITY));

        // DP Table: dp[i][w] = max value considering first i items with weight limit w
        std::vector<std::vector<int>> dp(n + 1, std::vector<int>(cap + 1, 0));

        result.memoryBytesUsed = (n + 1) * (cap + 1) * sizeof(int);

        for (size_t i = 1; i <= n; ++i) {
            const auto& item = availableItems[i - 1];
            for (int w = 0; w <= cap; ++w) {
                if (item.weight > w) {
                    dp[i][w] = dp[i - 1][w];
                } else {
                    dp[i][w] = std::max(dp[i - 1][w], dp[i - 1][w - item.weight] + item.value);
                }
            }
        }

        // Backtrack to find selected items
        int currW = cap;
        for (size_t i = n; i > 0; --i) {
            if (dp[i][currW] != dp[i - 1][currW]) {
                const auto& item = availableItems[i - 1];
                result.selectedItems.push_back(item);
                result.totalWeight += item.weight;
                result.totalValue += item.value;
                currW -= item.weight;
            }
        }

        std::reverse(result.selectedItems.begin(), result.selectedItems.end());

        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

        return result;
    }
};

} // namespace algorithms::knapsack
