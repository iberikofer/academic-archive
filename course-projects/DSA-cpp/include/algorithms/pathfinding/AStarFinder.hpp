#pragma once

#include "IPathfinder.hpp"
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace algorithms::pathfinding {

class AStarFinder : public IPathfinder {
public:
    [[nodiscard]] std::string getName() const override {
        return "A* (Octile Heuristic, 8-way)";
    }

    [[nodiscard]] static int calculateOctileDistance(const core::Point& a, const core::Point& b) noexcept {
        int dx = std::abs(a.x - b.x);
        int dy = std::abs(a.y - b.y);
        // h = 10 * (dx + dy) + (14 - 20) * min(dx, dy)
        return 10 * (dx + dy) - 6 * std::min(dx, dy);
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

        struct Node {
            core::Point point;
            int fScore;
            bool operator>(const Node& other) const noexcept {
                return fScore > other.fScore;
            }
        };

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        std::unordered_map<core::Point, int> gScore;
        std::unordered_map<core::Point, core::Point> cameFrom;

        gScore[start] = 0;
        openSet.push({start, calculateOctileDistance(start, goal)});

        while (!openSet.empty()) {
            Node current = openSet.top();
            openSet.pop();

            result.visitedNodes.push_back(current.point);

            if (current.point == goal) {
                // Reconstruct path
                core::Point curr = goal;
                while (curr != start) {
                    result.path.push_back(curr);
                    curr = cameFrom[curr];
                }
                result.path.push_back(start);
                std::reverse(result.path.begin(), result.path.end());

                result.totalEnergyCost = gScore[goal];
                result.pathFound = true;
                break;
            }

            auto neighbors = maze.getNeighbors(current.point);
            for (const auto& nb : neighbors) {
                int tentativeG = gScore[current.point] + nb.energyCost;

                auto it = gScore.find(nb.point);
                if (it == gScore.end() || tentativeG < it->second) {
                    cameFrom[nb.point] = current.point;
                    gScore[nb.point] = tentativeG;
                    int f = tentativeG + calculateOctileDistance(nb.point, goal);
                    openSet.push({nb.point, f});
                }
            }
        }

        result.visitedNodesCount = static_cast<int>(result.visitedNodes.size());
        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

        return result;
    }
};

} // namespace algorithms::pathfinding
