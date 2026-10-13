#pragma once

#include "core/Types.hpp"

namespace map {

class Cell {
public:
    Cell() = default;
    Cell(int x, int y, core::CellType type = core::CellType::Wall)
        : pos_{x, y}, type_{type} {}

    [[nodiscard]] const core::Point& getPosition() const noexcept { return pos_; }
    [[nodiscard]] int getX() const noexcept { return pos_.x; }
    [[nodiscard]] int getY() const noexcept { return pos_.y; }

    [[nodiscard]] core::CellType getType() const noexcept { return type_; }
    void setType(core::CellType type) noexcept { type_ = type; }

    [[nodiscard]] bool isPassable() const noexcept {
        return core::isPassable(type_);
    }

    [[nodiscard]] int getEnergyCost(bool isDiagonal) const noexcept {
        return core::getStepEnergyCost(type_, isDiagonal);
    }

    // Fog of war and algorithm wave tracking
    [[nodiscard]] bool isExplored() const noexcept { return isExplored_; }
    void setExplored(bool explored) noexcept { isExplored_ = explored; }

    [[nodiscard]] bool isVisitedBySearch() const noexcept { return isVisitedBySearch_; }
    void setVisitedBySearch(bool visited) noexcept { isVisitedBySearch_ = visited; }

    [[nodiscard]] bool isPartOfPath() const noexcept { return isPartOfPath_; }
    void setPartOfPath(bool inPath) noexcept { isPartOfPath_ = inPath; }

    // Associated item
    [[nodiscard]] bool hasResource() const noexcept { return resourceId_ >= 0; }
    [[nodiscard]] int getResourceId() const noexcept { return resourceId_; }
    void setResourceId(int id) noexcept { resourceId_ = id; }

    // Walked trail flag
    [[nodiscard]] bool isWalked() const noexcept { return isWalked_; }
    void setWalked(bool walked) noexcept { isWalked_ = walked; }

    void resetVisualizationFlags() noexcept {
        isVisitedBySearch_ = false;
        isPartOfPath_ = false;
        isWalked_ = false;
    }

private:
    core::Point pos_{0, 0};
    core::CellType type_ = core::CellType::Wall;
    bool isExplored_ = false;
    bool isVisitedBySearch_ = false;
    bool isPartOfPath_ = false;
    bool isWalked_ = false;
    int resourceId_ = -1;
};

} // namespace map
