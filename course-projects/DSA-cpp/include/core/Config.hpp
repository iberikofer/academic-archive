#pragma once

#include "Types.hpp"
#include <algorithm>

namespace core {

struct Config {
    // Map dimensions
    int mapWidth = 30;
    int mapHeight = 30;
    static constexpr int minMapDimension = 20;
    static constexpr int maxMapDimension = 100;

    // Energy management
    bool energyLimitEnabled = true;
    int initialEnergy = 600;

    // Route Mode: Point A to B (Exit) or Round Trip (Return to Start)
    RouteMode routeMode = RouteMode::PointAToPointB;

    // Backpack parameters
    bool autoBackpackCapacity = true; // Automatically ~50% of total spawned item weight
    int manualBackpackCapacity = 45;  // User slider fallback/manual override (10..200 kg)

    // Generation parameters
    unsigned int randomSeed = 1337;
    int minRoomSize = 4;
    int maxRoomSize = 12;
    float mudSpawnChance = 0.08f;     // Probability of mud/water hazard in rooms
    float extraPassageChance = 0.25f; // Chance of adding extra cyclic loops

    // UI & Visual preferences
    bool showGrid = true;
    bool showLegend = false;
    bool cameraFollowAgent = true;    // Auto-center camera when agent moves
    bool vsyncEnabled = true;         // Vertical synchronization (eliminates tearing & laddering)
    float uiScale = 1.0f;             // 0.75f to 1.50f
    float minimapScale = 1.0f;        // 0.70f to 1.30f (1.00x default)
    float stepDelaySeconds = 0.75f;   // Default 1 move per 0.75 seconds
    float speedMultiplier = 1.0f;     // x0.5 to x4.0

    // Compute dynamic recommended energy based on labyrinth dimensions
    int getRecommendedEnergy() const noexcept {
        // Generous scaling factor ensuring agent can cross the maze and return/evacuate
        return (mapWidth + mapHeight) * 12;
    }

    void clampSettings() noexcept {
        mapWidth = std::clamp(mapWidth, minMapDimension, maxMapDimension);
        mapHeight = std::clamp(mapHeight, minMapDimension, maxMapDimension);
        minRoomSize = std::clamp(minRoomSize, 4, 8);
        maxRoomSize = std::clamp(maxRoomSize, minRoomSize + 2, 16);
        manualBackpackCapacity = std::clamp(manualBackpackCapacity, 10, 200);
        speedMultiplier = std::clamp(speedMultiplier, 0.5f, 4.0f);
        uiScale = std::clamp(uiScale, 0.75f, 1.50f);
        minimapScale = std::clamp(minimapScale, 0.70f, 1.30f);
    }
};

} // namespace core
