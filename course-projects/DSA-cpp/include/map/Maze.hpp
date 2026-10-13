#pragma once

#include "Cell.hpp"
#include "Room.hpp"
#include "core/Item.hpp"
#include "core/Types.hpp"
#include <numeric>
#include <optional>
#include <queue>
#include <vector>

namespace map {

class Maze {
public:
  Maze(int width = 30, int height = 30) {
    resize(width, height);
    recomputeExteriorWalls();
  }

  void resize(int width, int height) {
    width_ = width;
    height_ = height;
    grid_.clear();
    grid_.resize(height_, std::vector<Cell>(width_));
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; ++x) {
        grid_[y][x] = Cell(x, y, core::CellType::Wall);
      }
    }
    rooms_.clear();
    items_.clear();
    exteriorWalls_.clear();
  }

  void recomputeExteriorWalls() const {
    exteriorWalls_.assign(height_, std::vector<bool>(width_, false));
    if (width_ <= 0 || height_ <= 0) return;

    std::queue<core::Point> q;

    // Seed BFS flood-fill from all 4 borders of the maze
    for (int x = 0; x < width_; ++x) {
      if (!grid_[0][x].isPassable() && !exteriorWalls_[0][x]) {
        exteriorWalls_[0][x] = true;
        q.push({x, 0});
      }
      if (height_ > 1 && !grid_[height_ - 1][x].isPassable() && !exteriorWalls_[height_ - 1][x]) {
        exteriorWalls_[height_ - 1][x] = true;
        q.push({x, height_ - 1});
      }
    }
    for (int y = 0; y < height_; ++y) {
      if (!grid_[y][0].isPassable() && !exteriorWalls_[y][0]) {
        exteriorWalls_[y][0] = true;
        q.push({0, y});
      }
      if (width_ > 1 && !grid_[y][width_ - 1].isPassable() && !exteriorWalls_[y][width_ - 1]) {
        exteriorWalls_[y][width_ - 1] = true;
        q.push({width_ - 1, y});
      }
    }

    constexpr int dx[4] = {1, -1, 0, 0};
    constexpr int dy[4] = {0, 0, 1, -1};

    while (!q.empty()) {
      auto curr = q.front();
      q.pop();

      for (int i = 0; i < 4; ++i) {
        int nx = curr.x + dx[i];
        int ny = curr.y + dy[i];
        if (isValid(nx, ny) && !grid_[ny][nx].isPassable() && !exteriorWalls_[ny][nx]) {
          exteriorWalls_[ny][nx] = true;
          q.push({nx, ny});
        }
      }
    }
  }

  [[nodiscard]] bool isExteriorWall(int x, int y) const {
    if (!isValid(x, y)) return true;
    if (exteriorWalls_.empty() ||
        exteriorWalls_.size() != static_cast<size_t>(height_) ||
        exteriorWalls_[0].size() != static_cast<size_t>(width_)) {
      recomputeExteriorWalls();
    }
    return exteriorWalls_[y][x];
  }

  [[nodiscard]] int getWidth() const noexcept { return width_; }
  [[nodiscard]] int getHeight() const noexcept { return height_; }

  [[nodiscard]] bool isValid(int x, int y) const noexcept {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
  }

  [[nodiscard]] bool isValid(const core::Point &p) const noexcept {
    return isValid(p.x, p.y);
  }

  [[nodiscard]] bool isPassable(int x, int y) const noexcept {
    return isValid(x, y) && grid_[y][x].isPassable();
  }

  [[nodiscard]] bool isPassable(const core::Point &p) const noexcept {
    return isPassable(p.x, p.y);
  }

  Cell &at(int x, int y) { return grid_[y][x]; }
  const Cell &at(int x, int y) const { return grid_[y][x]; }

  Cell &at(const core::Point &p) { return grid_[p.y][p.x]; }
  const Cell &at(const core::Point &p) const { return grid_[p.y][p.x]; }

  // Start & Exit points
  [[nodiscard]] const core::Point &getStartPoint() const noexcept {
    return startPoint_;
  }
  void setStartPoint(const core::Point &p) {
    startPoint_ = p;
    if (isValid(p)) {
      at(p).setType(core::CellType::Start);
    }
  }

  [[nodiscard]] const core::Point &getExitPoint() const noexcept {
    return exitPoint_;
  }
  void setExitPoint(const core::Point &p) {
    exitPoint_ = p;
    if (isValid(p)) {
      at(p).setType(core::CellType::Exit);
    }
  }

  // Generation seed
  [[nodiscard]] unsigned int getSeed() const noexcept { return seed_; }
  void setSeed(unsigned int seed) noexcept { seed_ = seed; }

  // Rooms & Items
  [[nodiscard]] const std::vector<Room> &getRooms() const noexcept {
    return rooms_;
  }
  std::vector<Room> &getRooms() noexcept { return rooms_; }

  [[nodiscard]] const std::vector<core::Item> &getItems() const noexcept {
    return items_;
  }
  std::vector<core::Item> &getItems() noexcept { return items_; }

  [[nodiscard]] std::optional<core::Item> getItemById(int id) const {
    for (const auto &item : items_) {
      if (item.id == id)
        return item;
    }
    return std::nullopt;
  }

  [[nodiscard]] int getTotalItemWeight() const noexcept {
    return std::accumulate(
        items_.begin(), items_.end(), 0,
        [](int sum, const core::Item &item) { return sum + item.weight; });
  }

  [[nodiscard]] int getTotalItemValue() const noexcept {
    return std::accumulate(
        items_.begin(), items_.end(), 0,
        [](int sum, const core::Item &item) { return sum + item.value; });
  }

  // Neighbors retrieval for pathfinding (8 directions)
  struct Neighbor {
    core::Point point;
    int energyCost;
    bool isDiagonal;
  };

  [[nodiscard]] std::vector<Neighbor>
  getNeighbors(const core::Point &current) const {
    std::vector<Neighbor> neighbors;
    neighbors.reserve(8);

    const auto &dirs = core::getEightDirections();
    for (const auto &dir : dirs) {
      core::Point next{current.x + dir.dx, current.y + dir.dy};
      if (!isValid(next) || !at(next).isPassable()) {
        continue;
      }

      // Diagonal movement restrictions:
      if (dir.isDiagonal) {
        // Disallow diagonal steps to/from Start and Exit (only orthogonal entrance/exit)
        auto currType = at(current).getType();
        auto nextType = at(next).getType();
        if (currType == core::CellType::Start || currType == core::CellType::Exit ||
            nextType == core::CellType::Start || nextType == core::CellType::Exit) {
          continue;
        }

        // 2. Strict no-corner-cutting: agent cannot clip past walls, corridor corners, or doorways.
        // Both adjacent orthogonal cells must be valid and passable (free of walls).
        bool blockedSide1 = !isValid(current.x + dir.dx, current.y) || !isPassable(current.x + dir.dx, current.y);
        bool blockedSide2 = !isValid(current.x, current.y + dir.dy) || !isPassable(current.x, current.y + dir.dy);
        if (blockedSide1 || blockedSide2) {
          continue; // Blocked: clipping a wall corner, doorway frame, or squeezing past wall
        }

        // 3. Mud corner barrier: cannot jump/cheat diagonally across mud corners,
        // nor squeeze diagonally past mud cells unless moving entirely within mud pool.
        auto side1Type = at(current.x + dir.dx, current.y).getType();
        auto side2Type = at(current.x, current.y + dir.dy).getType();
        if (side1Type == core::CellType::Mud || side2Type == core::CellType::Mud) {
          if (currType != core::CellType::Mud || nextType != core::CellType::Mud) {
            continue; // Cannot cut corner across a mud cell entering or leaving mud
          }
          if (side1Type == core::CellType::Mud && side2Type == core::CellType::Mud) {
            continue; // Cannot squeeze between two separate mud cells diagonally
          }
        }
      }

      int cost = at(next).getEnergyCost(dir.isDiagonal);
      neighbors.push_back({next, cost, dir.isDiagonal});
    }

    return neighbors;
  }

  void resetVisualizationFlags() noexcept {
    for (auto &row : grid_) {
      for (auto &cell : row) {
        cell.resetVisualizationFlags();
      }
    }
  }

private:
  int width_ = 30;
  int height_ = 30;
  std::vector<std::vector<Cell>> grid_;
  mutable std::vector<std::vector<bool>> exteriorWalls_;
  core::Point startPoint_{0, 0};
  core::Point exitPoint_{0, 0};
  std::vector<Room> rooms_;
  std::vector<core::Item> items_;
  unsigned int seed_ = 0;
};

} // namespace map
