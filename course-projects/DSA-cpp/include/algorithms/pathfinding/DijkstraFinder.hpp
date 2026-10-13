#pragma once

#include "IPathfinder.hpp"
#include <queue>
#include <unordered_map>
#include <algorithm>

namespace algorithms::pathfinding {

class DijkstraFinder : public IPathfinder {
public:
    [[nodiscard]] std::string getName() const override {
        return "Dijkstra (Uniform Cost, 8-way)";
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
            int dist;
            bool operator>(const Node& other) const noexcept {
                return dist > other.dist;
            }
        };

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        std::unordered_map<core::Point, int> distMap;
        std::unordered_map<core::Point, core::Point> cameFrom;

        distMap[start] = 0;
        pq.push({start, 0});

        while (!pq.empty()) {
            Node current = pq.top();
            pq.pop();

            // Skip if outdated entry
            if (current.dist > distMap[current.point]) {
                continue;
            }

            result.visitedNodes.push_back(current.point);

            if (current.point == goal) {
                core::Point curr = goal;
                while (curr != start) {
                    result.path.push_back(curr);
                    curr = cameFrom[curr];
                }
                result.path.push_back(start);
                std::reverse(result.path.begin(), result.path.end());

                result.totalEnergyCost = distMap[goal];
                result.pathFound = true;
                break;
            }

            auto neighbors = maze.getNeighbors(current.point);
            for (const auto& nb : neighbors) {
                int newDist = current.dist + nb.energyCost;

                auto it = distMap.find(nb.point);
                if (it == distMap.end() || newDist < it->second) {
                    distMap[nb.point] = newDist;
                    cameFrom[nb.point] = current.point;
                    pq.push({nb.point, newDist});
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
