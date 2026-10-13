#pragma once

#include "core/Types.hpp"
#include "core/Item.hpp"
#include <vector>
#include <string>

namespace algorithms::routing {

struct Waypoint {
    core::Point position{0, 0};
    int itemId = -1; // -1 if Start or Exit
    std::string label;
    int itemWeight = 0;
    int itemValue = 0;
};

struct RouteSegment {
    Waypoint from;
    Waypoint to;
    std::vector<core::Point> path;
    int energyCost = 0;
};

struct RoutePlan {
    std::vector<Waypoint> visitOrder;
    std::vector<RouteSegment> segments;
    std::vector<core::Point> fullPath;
    int totalEnergyCost = 0;
    int energyBefore2Opt = 0;
    int totalSteps = 0;
    double twoOptGainPercent = 0.0;
    long long calculationTimeMicroseconds = 0;
    bool routeValid = false;

    [[nodiscard]] double calculateGain() const noexcept {
        if (energyBefore2Opt > 0 && totalEnergyCost < energyBefore2Opt) {
            return (static_cast<double>(energyBefore2Opt - totalEnergyCost) / energyBefore2Opt) * 100.0;
        }
        return 0.0;
    }
};

} // namespace algorithms::routing
