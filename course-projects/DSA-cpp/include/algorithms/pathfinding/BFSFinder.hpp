#pragma once

#include "IPathfinder.hpp"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace algorithms::pathfinding {

class BFSFinder : public IPathfinder {
public:
    [[nodiscard]] std::string getName() const override {
        return "BFS (Breadth-First Search, 8-way)";
    }

    [[nodiscard]] PathResult findPath(
        const map::Maze& maze,
        const core::Point& start,
        const core::Point& goal
    ) override {
        auto startTime = std::chrono::high_resolution_clock::now();

        PathResult result;
        result.algorithmName = getName();

        if (!maze.isValid(start) || !maze.isValid(goal) ||
            !maze.isPassable(start) || !maze.isPassable(goal)) {
            auto endTime = std::chrono::high_resolution_clock::now();
            result.executionTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
            return result;
        }

        if (start == goal) {
            result.path = {start};
            result.pathFound = true;
            auto endTime = std::chrono::high_resolution_clock::now();
            result.executionTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
            return result;
        }

        std::queue<core::Point> q;
        std::unordered_set<core::Point> visited;
        std::unordered_map<core::Point, core::Point> cameFrom;

        q.push(start);
        visited.insert(start);

        bool reached = false;

        while (!q.empty()) {
            core::Point current = q.front();
            q.pop();

            result.visitedNodes.push_back(current);

            if (current == goal) {
                reached = true;
                break;
            }

            auto neighbors = maze.getNeighbors(current);
            for (const auto& nb : neighbors) {
                if (visited.find(nb.point) == visited.end()) {
                    visited.insert(nb.point);
                    cameFrom[nb.point] = current;
                    q.push(nb.point);
                }
            }
        }

        if (reached) {
            core::Point curr = goal;
            while (curr != start) {
                result.path.push_back(curr);
                curr = cameFrom[curr];
            }
            result.path.push_back(start);
            std::reverse(result.path.begin(), result.path.end());

            // Compute actual energy along the BFS hops
            result.totalEnergyCost = 0;
            for (size_t i = 1; i < result.path.size(); ++i) {
                const auto& prev = result.path[i - 1];
                const auto& next = result.path[i];
                bool isDiag = (prev.x != next.x && prev.y != next.y);
                result.totalEnergyCost += maze.at(next).getEnergyCost(isDiag);
            }

            result.pathFound = true;
        }

        result.visitedNodesCount = static_cast<int>(result.visitedNodes.size());
        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

        return result;
    }
};

} // namespace algorithms::pathfinding
