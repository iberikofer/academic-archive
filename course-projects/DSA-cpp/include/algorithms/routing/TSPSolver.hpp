#pragma once

#include "RoutePlan.hpp"
#include "algorithms/pathfinding/IPathfinder.hpp"
#include "algorithms/pathfinding/AStarFinder.hpp"
#include "core/Types.hpp"
#include <vector>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <iostream>

namespace algorithms::routing {

class TSPSolver {
public:
    explicit TSPSolver(pathfinding::IPathfinder* pathfinder = nullptr)
        : defaultFinder_(), pathfinder_(pathfinder ? pathfinder : &defaultFinder_) {}

    void setPathfinder(pathfinding::IPathfinder* pathfinder) {
        pathfinder_ = pathfinder ? pathfinder : &defaultFinder_;
    }

    [[nodiscard]] RoutePlan solveRoute(
        const map::Maze& maze,
        const std::vector<core::Item>& targetItems,
        core::RouteMode routeMode
    ) {
        auto startTime = std::chrono::high_resolution_clock::now();

        RoutePlan plan;

        // If no items selected, just go directly from Start to Finish
        std::vector<Waypoint> waypoints;
        Waypoint startWp{maze.getStartPoint(), -1, "Start (Airlock)", 0, 0};
        waypoints.push_back(startWp);

        // Dynamic waypoint cap based on labyrinth dimensions (18 at 30x30, scaling gracefully up to 45 on 100x100)
        size_t maxWaypoints = static_cast<size_t>(std::clamp(18 + static_cast<int>((maze.getWidth() - 30) * 0.35f), 12, 45));
        std::vector<core::Item> itemsToRoute = targetItems;
        if (itemsToRoute.size() > maxWaypoints) {
            std::partial_sort(itemsToRoute.begin(), itemsToRoute.begin() + maxWaypoints, itemsToRoute.end(),
                [](const core::Item& a, const core::Item& b) {
                    return a.value > b.value;
                });
            itemsToRoute.resize(maxWaypoints);
        }

        for (const auto& item : itemsToRoute) {
            waypoints.push_back({item.position, item.id, item.name, item.weight, item.value});
        }

        Waypoint endWp;
        if (routeMode == core::RouteMode::PointAToPointB) {
            endWp = {maze.getExitPoint(), -1, "Exit (Evac)", 0, 0};
        } else {
            endWp = {maze.getStartPoint(), -1, "Return to Base", 0, 0};
        }
        waypoints.push_back(endWp);

        const size_t n = waypoints.size();

        if (n <= 2) {
            // Direct path from start to end
            auto pathRes = pathfinder_->findPath(maze, startWp.position, endWp.position);
            plan.visitOrder = {startWp, endWp};
            plan.fullPath = pathRes.path;
            plan.totalEnergyCost = pathRes.totalEnergyCost;
            plan.energyBefore2Opt = pathRes.totalEnergyCost;
            plan.totalSteps = static_cast<int>(pathRes.getStepCount());
            plan.routeValid = pathRes.pathFound;
            auto endTime = std::chrono::high_resolution_clock::now();
            plan.calculationTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
            return plan;
        }

        // 1. Build Distance Matrix & cache path segments
        std::vector<std::vector<int>> dist(n, std::vector<int>(n, 0));
        std::vector<std::vector<std::vector<core::Point>>> cachedPaths(n, std::vector<std::vector<core::Point>>(n));

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                auto pathRes = pathfinder_->findPath(maze, waypoints[i].position, waypoints[j].position);
                int cost = pathRes.pathFound ? pathRes.totalEnergyCost : 999999;

                dist[i][j] = cost;
                dist[j][i] = cost;

                cachedPaths[i][j] = pathRes.path;
                auto revPath = pathRes.path;
                std::reverse(revPath.begin(), revPath.end());
                cachedPaths[j][i] = std::move(revPath);
            }
        }

        // 2. Nearest Neighbor Tour Construction
        // Fixed start = index 0, Fixed end = index n - 1
        std::vector<size_t> tour;
        std::vector<bool> visited(n, false);

        tour.push_back(0);
        visited[0] = true;
        visited[n - 1] = true; // Reserve end point for final step

        size_t current = 0;
        for (size_t step = 1; step < n - 1; ++step) {
            size_t nextBest = SIZE_MAX;
            int bestCost = 9000000;
            for (size_t j = 1; j < n - 1; ++j) {
                if (!visited[j] && dist[current][j] < bestCost) {
                    bestCost = dist[current][j];
                    nextBest = j;
                }
            }
            if (nextBest != SIZE_MAX) {
                tour.push_back(nextBest);
                visited[nextBest] = true;
                current = nextBest;
            } else {
                // If unreachable from current, append first unvisited waypoint to ensure complete tour
                for (size_t j = 1; j < n - 1; ++j) {
                    if (!visited[j]) {
                        tour.push_back(j);
                        visited[j] = true;
                        current = j;
                        break;
                    }
                }
            }
        }
        tour.push_back(n - 1); // Append fixed destination

        auto computeTourCost = [&](const std::vector<size_t>& t) {
            int total = 0;
            for (size_t i = 0; i < t.size() - 1; ++i) {
                total += dist[t[i]][t[i + 1]];
            }
            return total;
        };

        int initialCost = computeTourCost(tour);
        plan.energyBefore2Opt = initialCost;

        // 3. 2-Opt Local Search Optimization
        // Keep index 0 and index n - 1 fixed; reorder intermediate items
        bool improved = true;
        int maxIterations = 50;
        int iter = 0;

        while (improved && iter++ < maxIterations) {
            improved = false;
            for (size_t i = 1; i < tour.size() - 2; ++i) {
                for (size_t k = i + 1; k < tour.size() - 1; ++k) {
                    // Guard against disconnected/infinite cost edges (>= 9000000)
                    if (dist[tour[i - 1]][tour[k]] >= 9000000 ||
                        dist[tour[i]][tour[k + 1]] >= 9000000 ||
                        dist[tour[i - 1]][tour[i]] >= 9000000 ||
                        dist[tour[k]][tour[k + 1]] >= 9000000) {
                        continue;
                    }

                    int delta = (dist[tour[i - 1]][tour[k]] + dist[tour[i]][tour[k + 1]])
                              - (dist[tour[i - 1]][tour[i]] + dist[tour[k]][tour[k + 1]]);

                    if (delta < 0) {
                        std::reverse(tour.begin() + i, tour.begin() + k + 1);
                        improved = true;
                    }
                }
            }
        }

        int finalCost = computeTourCost(tour);
        plan.totalEnergyCost = finalCost;
        plan.twoOptGainPercent = plan.calculateGain();

        // 4. Assemble Final Detailed Plan & Stitch Full Path
        for (size_t i = 0; i < tour.size(); ++i) {
            plan.visitOrder.push_back(waypoints[tour[i]]);
        }

        for (size_t i = 0; i < tour.size() - 1; ++i) {
            size_t fromIdx = tour[i];
            size_t toIdx = tour[i + 1];

            RouteSegment segment;
            segment.from = waypoints[fromIdx];
            segment.to = waypoints[toIdx];
            segment.path = cachedPaths[fromIdx][toIdx];
            segment.energyCost = dist[fromIdx][toIdx];

            plan.segments.push_back(segment);

            // Stitch into full path without duplicating joining node
            if (plan.fullPath.empty()) {
                plan.fullPath = segment.path;
            } else if (!segment.path.empty()) {
                plan.fullPath.insert(plan.fullPath.end(), segment.path.begin() + 1, segment.path.end());
            }
        }

        plan.totalSteps = plan.fullPath.empty() ? 0 : static_cast<int>(plan.fullPath.size() - 1);
        plan.routeValid = (finalCost < 999999);

        auto endTime = std::chrono::high_resolution_clock::now();
        plan.calculationTimeMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

        return plan;
    }

private:
    pathfinding::AStarFinder defaultFinder_;
    pathfinding::IPathfinder* pathfinder_;
};

} // namespace algorithms::routing
