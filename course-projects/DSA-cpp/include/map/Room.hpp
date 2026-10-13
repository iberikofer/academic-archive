#pragma once

#include "core/Types.hpp"
#include <vector>

namespace map {

struct Room {
    int id = 0;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    std::vector<int> itemIds;

    [[nodiscard]] core::Point getCenter() const noexcept {
        return { x + width / 2, y + height / 2 };
    }

    [[nodiscard]] bool contains(const core::Point& p) const noexcept {
        return p.x >= x && p.x < (x + width) && p.y >= y && p.y < (y + height);
    }

    [[nodiscard]] bool intersects(const Room& other, int margin = 1) const noexcept {
        return (x - margin <= other.x + other.width) &&
               (x + width + margin >= other.x) &&
               (y - margin <= other.y + other.height) &&
               (y + height + margin >= other.y);
    }
};

} // namespace map
