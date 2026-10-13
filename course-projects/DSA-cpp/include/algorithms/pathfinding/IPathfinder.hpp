#pragma once

#include "core/Types.hpp"
#include "map/Maze.hpp"
#include <vector>
#include <string>
#include <chrono>

namespace algorithms::pathfinding {

struct PathResult {
    std::vector<core::Point> path;
    int totalEnergyCost = 0;
    int visitedNodesCount = 0;
    std::vector<core::Point> visitedNodes;
    long long executionTimeMicroseconds = 0;
    std::string algorithmName;
    bool pathFound = false;

    [[nodiscard]] size_t getStepCount() const noexcept {
        return path.empty() ? 0 : path.size() - 1;
    }
};

class IPathfinder {
public:
    virtual ~IPathfinder() = default;

    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual PathResult findPath(
        const map::Maze& maze,
        const core::Point& start,
        const core::Point& goal
    ) = 0;
};

} // namespace algorithms::pathfinding
