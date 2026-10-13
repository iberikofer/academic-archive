#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace core {

struct Point {
    int x = 0;
    int y = 0;

    constexpr bool operator==(const Point& other) const noexcept {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Point& other) const noexcept {
        return !(*this == other);
    }

    constexpr bool operator<(const Point& other) const noexcept {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

enum class CellType {
    Wall = 0,
    Floor = 1,
    Mud = 2,
    Start = 3,
    Exit = 4
};

inline bool isPassable(CellType type) noexcept {
    return type != CellType::Wall;
}

inline std::string cellTypeToString(CellType type) {
    switch (type) {
        case CellType::Wall:  return "Wall";
        case CellType::Floor: return "Floor";
        case CellType::Mud:   return "Mud/Water";
        case CellType::Start: return "Start (Airlock)";
        case CellType::Exit:  return "Exit";
        default:              return "Unknown";
    }
}

// Energy consumption per movement type
inline int getStepEnergyCost(CellType targetType, bool isDiagonal) noexcept {
    if (!isPassable(targetType)) {
        return 999999;
    }

    if (targetType == CellType::Mud) {
        return isDiagonal ? 35 : 25; // Difficult terrain
    }

    return isDiagonal ? 14 : 10;     // Standard passable terrain (approx sqrt(2)*10)
}

struct NeighborOffset {
    int dx;
    int dy;
    bool isDiagonal;
};

// 8 directions (4 cardinal + 4 diagonal)
inline const std::vector<NeighborOffset>& getEightDirections() {
    static const std::vector<NeighborOffset> directions = {
        { 0, -1, false}, // Up
        { 0,  1, false}, // Down
        {-1,  0, false}, // Left
        { 1,  0, false}, // Right
        {-1, -1, true }, // Up-Left
        { 1, -1, true }, // Up-Right
        {-1,  1, true }, // Down-Left
        { 1,  1, true }  // Down-Right
    };
    return directions;
}

enum class RouteMode {
    PointAToPointB = 0, // Mode 1: Start at A, gather items, evacuate at Exit B
    RoundTrip = 1       // Mode 2: Start at A, gather items, return to Base A
};

} // namespace core

// Hash support for std::unordered_map and std::unordered_set
template <>
struct std::hash<core::Point> {
    std::size_t operator()(const core::Point& p) const noexcept {
        std::size_t h1 = std::hash<int>{}(p.x);
        std::size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
