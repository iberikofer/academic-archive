#include "map/BSPGenerator.hpp"
#include <queue>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <unordered_map>

namespace map {

void BSPGenerator::generate(Maze& maze) {
    maze.resize(config_.mapWidth, config_.mapHeight);
    maze.setSeed(config_.randomSeed);

    // Bounding partition box (with 1-cell border)
    auto root = std::make_unique<BSPNode>(1, 1, config_.mapWidth - 2, config_.mapHeight - 2);

    int maxDepth = (config_.mapWidth >= 70) ? 5 : (config_.mapWidth >= 40 ? 4 : 3);
    splitNode(root.get(), maxDepth);

    std::vector<Room> rooms;
    createRooms(root.get(), maze, rooms);

    if (rooms.empty()) {
        // Fallback: create at least one central room
        Room fallback{1, 2, 2, config_.mapWidth - 4, config_.mapHeight - 4, {}};
        for (int y = fallback.y; y < fallback.y + fallback.height; ++y) {
            for (int x = fallback.x; x < fallback.x + fallback.width; ++x) {
                maze.at(x, y).setType(core::CellType::Floor);
            }
        }
        rooms.push_back(fallback);
    }

    // Connect leaf rooms recursively through BSP tree
    connectTree(root.get(), maze);

    // Add extra passages and loops to create a cyclic maze
    addExtraCycles(maze, rooms);

    // Verify full maze connectivity via BFS
    ensureConnectivity(maze);

    // Helper to find the best outer-facing boundary cell for a given room
    auto getOuterBoundaryCell = [&](const Room& r) -> core::Point {
        int distL = r.x;
        int distT = r.y;
        int distR = (maze.getWidth() - 1) - (r.x + r.width - 1);
        int distB = (maze.getHeight() - 1) - (r.y + r.height - 1);
        int minDist = std::min({distL, distT, distR, distB});

        core::Point pt;
        if (minDist == distL) {
            // Left boundary: carve airlock 1 cell to the left if within maze bounds
            int ax = (r.x > 1) ? r.x - 1 : r.x;
            int ay = r.y + r.height / 2;
            maze.at(ax, ay).setType(core::CellType::Floor);
            pt = {ax, ay};
        } else if (minDist == distT) {
            // Top boundary: carve airlock 1 cell up
            int ax = r.x + r.width / 2;
            int ay = (r.y > 1) ? r.y - 1 : r.y;
            maze.at(ax, ay).setType(core::CellType::Floor);
            pt = {ax, ay};
        } else if (minDist == distR) {
            // Right boundary: carve airlock 1 cell right
            int ax = (r.x + r.width < maze.getWidth() - 1) ? r.x + r.width : r.x + r.width - 1;
            int ay = r.y + r.height / 2;
            maze.at(ax, ay).setType(core::CellType::Floor);
            pt = {ax, ay};
        } else {
            // Bottom boundary: carve airlock 1 cell down
            int ax = r.x + r.width / 2;
            int ay = (r.y + r.height < maze.getHeight() - 1) ? r.y + r.height : r.y + r.height - 1;
            maze.at(ax, ay).setType(core::CellType::Floor);
            pt = {ax, ay};
        }
        return pt;
    };

    // 1. Find the most peripheral room for Start (closest to maze outer perimeter)
    const Room* startRoom = &rooms.front();
    int minBorderDist = 999999;
    for (const auto& r : rooms) {
        int d = std::min({r.x, r.y,
                          (maze.getWidth() - 1) - (r.x + r.width - 1),
                          (maze.getHeight() - 1) - (r.y + r.height - 1)});
        if (d < minBorderDist) {
            minBorderDist = d;
            startRoom = &r;
        }
    }
    core::Point startPos = getOuterBoundaryCell(*startRoom);
    maze.setStartPoint(startPos);

    // 2. Find the furthest room for Exit and place on its outer boundary
    const Room* exitRoom = startRoom;
    double maxDist = -1.0;
    for (const auto& r : rooms) {
        if (&r == startRoom) continue;
        core::Point c = r.getCenter();
        double dist = std::hypot(c.x - startPos.x, c.y - startPos.y);
        if (dist > maxDist) {
            maxDist = dist;
            exitRoom = &r;
        }
    }
    core::Point exitPos = (exitRoom != startRoom) ? getOuterBoundaryCell(*exitRoom) : startPos;
    maze.setExitPoint(exitPos);

    // Store rooms inside maze before spawning resources
    maze.getRooms() = std::move(rooms);

    // Populate rooms with resources
    spawnResources(maze);

    // Add tactical terrain hazards (Mud/Water)
    spawnMudHazards(maze);

    // Recompute exterior vs interior enclosed walls
    maze.recomputeExteriorWalls();
}

void BSPGenerator::splitNode(BSPNode* node, int depth) {
    if (depth <= 0) return;

    int minSplitSize = config_.minRoomSize + 3;
    bool canSplitH = node->height >= (minSplitSize * 2);
    bool canSplitV = node->width >= (minSplitSize * 2);

    if (!canSplitH && !canSplitV) return;

    bool splitHorizontally = false;
    if (canSplitH && canSplitV) {
        if (node->width > node->height * 1.25) {
            splitHorizontally = false;
        } else if (node->height > node->width * 1.25) {
            splitHorizontally = true;
        } else {
            std::uniform_int_distribution<int> coin(0, 1);
            splitHorizontally = (coin(rng_) == 1);
        }
    } else {
        splitHorizontally = canSplitH;
    }

    if (splitHorizontally) {
        std::uniform_int_distribution<int> dist(minSplitSize, node->height - minSplitSize);
        int splitY = dist(rng_);
        node->left = std::make_unique<BSPNode>(node->x, node->y, node->width, splitY);
        node->right = std::make_unique<BSPNode>(node->x, node->y + splitY, node->width, node->height - splitY);
    } else {
        std::uniform_int_distribution<int> dist(minSplitSize, node->width - minSplitSize);
        int splitX = dist(rng_);
        node->left = std::make_unique<BSPNode>(node->x, node->y, splitX, node->height);
        node->right = std::make_unique<BSPNode>(node->x + splitX, node->y, node->width - splitX, node->height);
    }

    splitNode(node->left.get(), depth - 1);
    splitNode(node->right.get(), depth - 1);
}

void BSPGenerator::createRooms(BSPNode* node, Maze& maze, std::vector<Room>& rooms) {
    if (!node) return;

    if (node->isLeaf()) {
        int availW = node->width - 2;
        int availH = node->height - 2;
        if (availW < 2 || availH < 2) return;

        int maxW = std::clamp(availW, 2, config_.maxRoomSize);
        int maxH = std::clamp(availH, 2, config_.maxRoomSize);
        int minW = std::clamp(config_.minRoomSize, 2, maxW);
        int minH = std::clamp(config_.minRoomSize, 2, maxH);

        int roomW = (minW < maxW) ? std::uniform_int_distribution<int>(minW, maxW)(rng_) : minW;
        int roomH = (minH < maxH) ? std::uniform_int_distribution<int>(minH, maxH)(rng_) : minH;

        int minX = node->x + 1;
        int maxX = std::max(minX, node->x + node->width - roomW - 1);
        int minY = node->y + 1;
        int maxY = std::max(minY, node->y + node->height - roomH - 1);

        int rx = (minX < maxX) ? std::uniform_int_distribution<int>(minX, maxX)(rng_) : minX;
        int ry = (minY < maxY) ? std::uniform_int_distribution<int>(minY, maxY)(rng_) : minY;

        Room r{static_cast<int>(rooms.size()) + 1, rx, ry, roomW, roomH, {}};
        node->room = r;

        // Carve room floor in maze
        for (int y = ry; y < ry + roomH; ++y) {
            for (int x = rx; x < rx + roomW; ++x) {
                if (maze.isValid(x, y)) {
                    maze.at(x, y).setType(core::CellType::Floor);
                }
            }
        }

        rooms.push_back(r);
        return;
    }

    createRooms(node->left.get(), maze, rooms);
    createRooms(node->right.get(), maze, rooms);
}

core::Point BSPGenerator::getRoomCenterFromNode(const BSPNode* node) {
    if (!node) return {0, 0};
    if (node->room.has_value()) {
        return node->room->getCenter();
    }
    if (node->left) {
        return getRoomCenterFromNode(node->left.get());
    }
    if (node->right) {
        return getRoomCenterFromNode(node->right.get());
    }
    return {node->x + node->width / 2, node->y + node->height / 2};
}

void BSPGenerator::connectTree(BSPNode* node, Maze& maze) {
    if (!node || node->isLeaf()) return;

    connectTree(node->left.get(), maze);
    connectTree(node->right.get(), maze);

    core::Point p1 = getRoomCenterFromNode(node->left.get());
    core::Point p2 = getRoomCenterFromNode(node->right.get());
    carveCorridor(maze, p1, p2);
}

void BSPGenerator::carveCorridor(Maze& maze, const core::Point& from, const core::Point& to) {
    int curX = from.x;
    int curY = from.y;

    std::uniform_int_distribution<int> coin(0, 1);
    bool horizontalFirst = (coin(rng_) == 1);

    auto carvePoint = [&](int x, int y) {
        if (maze.isValid(x, y) && maze.at(x, y).getType() == core::CellType::Wall) {
            maze.at(x, y).setType(core::CellType::Floor);
        }
    };

    if (horizontalFirst) {
        int stepX = (to.x >= curX) ? 1 : -1;
        while (curX != to.x) {
            carvePoint(curX, curY);
            curX += stepX;
        }
        int stepY = (to.y >= curY) ? 1 : -1;
        while (curY != to.y) {
            carvePoint(curX, curY);
            curY += stepY;
        }
        carvePoint(curX, curY);
    } else {
        int stepY = (to.y >= curY) ? 1 : -1;
        while (curY != to.y) {
            carvePoint(curX, curY);
            curY += stepY;
        }
        int stepX = (to.x >= curX) ? 1 : -1;
        while (curX != to.x) {
            carvePoint(curX, curY);
            curX += stepX;
        }
        carvePoint(curX, curY);
    }
}

void BSPGenerator::addExtraCycles(Maze& maze, const std::vector<Room>& rooms) {
    if (rooms.size() < 2) return;

    std::uniform_real_distribution<float> prob(0.0f, 1.0f);
    for (size_t i = 0; i < rooms.size(); ++i) {
        for (size_t j = i + 1; j < rooms.size(); ++j) {
            core::Point c1 = rooms[i].getCenter();
            core::Point c2 = rooms[j].getCenter();
            double dist = std::hypot(c1.x - c2.x, c1.y - c2.y);

            // Connect nearby rooms with probability to introduce cyclic loops
            if (dist < (config_.mapWidth + config_.mapHeight) * 0.35) {
                if (prob(rng_) < config_.extraPassageChance) {
                    carveCorridor(maze, c1, c2);
                }
            }
        }
    }
}

void BSPGenerator::ensureConnectivity(Maze& maze) {
    if (maze.getRooms().empty() && !maze.isPassable(maze.getStartPoint())) return;

    core::Point start = maze.getStartPoint();
    std::vector<std::vector<bool>> visited(maze.getHeight(), std::vector<bool>(maze.getWidth(), false));
    std::queue<core::Point> q;

    if (maze.isValid(start) && maze.isPassable(start)) {
        visited[start.y][start.x] = true;
        q.push(start);
    }

    while (!q.empty()) {
        core::Point curr = q.front();
        q.pop();

        for (const auto& dir : core::getEightDirections()) {
            if (dir.isDiagonal) continue; // Orthogonal wave connectivity is sufficient
            int nx = curr.x + dir.dx;
            int ny = curr.y + dir.dy;
            if (maze.isValid(nx, ny) && maze.isPassable(nx, ny) && !visited[ny][nx]) {
                visited[ny][nx] = true;
                q.push({nx, ny});
            }
        }
    }

    // Connect any unreachable floor cell directly to start
    for (int y = 1; y < maze.getHeight() - 1; ++y) {
        for (int x = 1; x < maze.getWidth() - 1; ++x) {
            if (maze.isPassable(x, y) && !visited[y][x]) {
                carveCorridor(maze, {x, y}, start);
                visited[y][x] = true;
            }
        }
    }
}

void BSPGenerator::spawnMudHazards(Maze& maze) {
    if (config_.mudSpawnChance <= 0.001f) {
        return;
    }

    const auto startPos = maze.getStartPoint();
    const auto exitPos = maze.getExitPoint();
    const auto& rooms = maze.getRooms();

    // Collect all valid walkable floor cells
    std::vector<core::Point> allFloorCells;
    allFloorCells.reserve(maze.getWidth() * maze.getHeight() / 2);

    for (int y = 1; y < maze.getHeight() - 1; ++y) {
        for (int x = 1; x < maze.getWidth() - 1; ++x) {
            core::Point p{x, y};
            if (p == startPos || p == exitPos) {
                continue;
            }
            if (maze.at(x, y).getType() == core::CellType::Floor && !maze.at(x, y).hasResource()) {
                allFloorCells.push_back(p);
            }
        }
    }

    if (allFloorCells.empty()) {
        return;
    }

    // Target mud count clamped to prevent turning map into an unplayable swamp
    int targetMudCount = static_cast<int>(std::round(allFloorCells.size() * config_.mudSpawnChance));
    targetMudCount = std::clamp(targetMudCount, 2, static_cast<int>(allFloorCells.size() / 4));

    // Helper: count cardinal wall neighbors
    auto countCardinalWalls = [&](int x, int y) {
        int walls = 0;
        const int dx[4] = {0, 0, -1, 1};
        const int dy[4] = {-1, 1, 0, 0};
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (!maze.isValid(nx, ny) || maze.at(nx, ny).getType() == core::CellType::Wall) {
                walls++;
            }
        }
        return walls;
    };

    // Helper: check if a cell is near Start or Exit (keep safe radius of 2 tiles)
    auto isNearStartOrExit = [&](const core::Point& p) {
        if (p == startPos || p == exitPos) return true;
        int dStart = std::abs(p.x - startPos.x) + std::abs(p.y - startPos.y);
        int dExit = std::abs(p.x - exitPos.x) + std::abs(p.y - exitPos.y);
        return dStart <= 2 || dExit <= 2;
    };

    // Helper: detect useless dead-ends and peripheral empty corners where no player walks
    auto isUselessDeadCorner = [&](const core::Point& p) {
        if (isNearStartOrExit(p)) return true;
        if (maze.at(p).hasResource()) return false;

        int walls = countCardinalWalls(p.x, p.y);
        if (walls >= 3) {
            return true;
        }

        if (walls == 2) {
            bool topWall = !maze.isValid(p.x, p.y - 1) || maze.at(p.x, p.y - 1).getType() == core::CellType::Wall;
            bool bottomWall = !maze.isValid(p.x, p.y + 1) || maze.at(p.x, p.y + 1).getType() == core::CellType::Wall;
            bool leftWall = !maze.isValid(p.x - 1, p.y) || maze.at(p.x - 1, p.y).getType() == core::CellType::Wall;
            bool rightWall = !maze.isValid(p.x + 1, p.y) || maze.at(p.x + 1, p.y).getType() == core::CellType::Wall;
            bool isPerpendicularCorner = (topWall && leftWall) || (topWall && rightWall) ||
                                         (bottomWall && leftWall) || (bottomWall && rightWall);
            if (isPerpendicularCorner) {
                bool scrapNearby = false;
                for (int dy = -2; dy <= 2 && !scrapNearby; ++dy) {
                    for (int dx = -2; dx <= 2; ++dx) {
                        int nx = p.x + dx;
                        int ny = p.y + dy;
                        if (maze.isValid(nx, ny) && maze.at(nx, ny).hasResource()) {
                            scrapNearby = true;
                            break;
                        }
                    }
                }
                if (!scrapNearby) {
                    return true;
                }
            }
        }

        return false;
    };

    // Helper: detect 1-tile wide corridor bottlenecks
    auto isCorridorBottleneck = [&](const core::Point& p) {
        if (!maze.isValid(p) || maze.at(p).getType() != core::CellType::Floor) return false;
        if (isNearStartOrExit(p)) return false;

        bool topWall = !maze.isValid(p.x, p.y - 1) || maze.at(p.x, p.y - 1).getType() == core::CellType::Wall;
        bool bottomWall = !maze.isValid(p.x, p.y + 1) || maze.at(p.x, p.y + 1).getType() == core::CellType::Wall;
        bool leftWall = !maze.isValid(p.x - 1, p.y) || maze.at(p.x - 1, p.y).getType() == core::CellType::Wall;
        bool rightWall = !maze.isValid(p.x + 1, p.y) || maze.at(p.x + 1, p.y).getType() == core::CellType::Wall;

        // Strict 1-tile horizontal corridor: walls top & bottom, open left & right
        if (topWall && bottomWall && !leftWall && !rightWall) return true;
        // Strict 1-tile vertical corridor: walls left & right, open top & bottom
        if (leftWall && rightWall && !topWall && !bottomWall) return true;

        return false;
    };

    // Helper: count cardinal mud neighbors
    auto countCardinalMud = [&](const core::Point& pt) {
        int count = 0;
        const int dx[4] = {0, 0, -1, 1};
        const int dy[4] = {-1, 1, 0, 0};
        for (int i = 0; i < 4; ++i) {
            core::Point np{pt.x + dx[i], pt.y + dy[i]};
            if (maze.isValid(np) && maze.at(np).getType() == core::CellType::Mud) {
                count++;
            }
        }
        return count;
    };

    // Helper: check if a cell can become Mud while strictly enforcing:
    // 1. Max contiguous puddle size <= 2 cells anywhere on the map
    // 2. Minimum dry buffer of 4 tiles in 1-tile corridors (never multiple puddles in the same corridor)
    // 3. No mud directly under scrap or near Start/Exit
    auto canPlaceMud = [&](const core::Point& p) {
        if (!maze.isValid(p)) return false;
        if (maze.at(p).getType() != core::CellType::Floor) return false;
        if (maze.at(p).hasResource()) return false;
        if (isNearStartOrExit(p)) return false;

        // Invariant 1: Max cluster size <= 2
        int cardMud = countCardinalMud(p);
        if (cardMud >= 2) {
            return false; // Merging two mud cells would create a cluster of 3+
        }
        if (cardMud == 1) {
            // Touches exactly one mud neighbor.
            // That neighbor MUST NOT have any other mud neighbor.
            const int dx[4] = {0, 0, -1, 1};
            const int dy[4] = {-1, 1, 0, 0};
            for (int i = 0; i < 4; ++i) {
                core::Point np{p.x + dx[i], p.y + dy[i]};
                if (maze.isValid(np) && maze.at(np).getType() == core::CellType::Mud) {
                    if (countCardinalMud(np) >= 1) {
                        return false; // Neighbor is already part of a multi-cell puddle!
                    }
                    break;
                }
            }
        }

        // Invariant 2: Corridor dry buffer (at most one puddle per corridor segment)
        if (isCorridorBottleneck(p)) {
            bool leftWall = !maze.isValid(p.x - 1, p.y) || maze.at(p.x - 1, p.y).getType() == core::CellType::Wall;
            bool rightWall = !maze.isValid(p.x + 1, p.y) || maze.at(p.x + 1, p.y).getType() == core::CellType::Wall;
            bool topWall = !maze.isValid(p.x, p.y - 1) || maze.at(p.x, p.y - 1).getType() == core::CellType::Wall;
            bool bottomWall = !maze.isValid(p.x, p.y + 1) || maze.at(p.x, p.y + 1).getType() == core::CellType::Wall;

            if (leftWall && rightWall) {
                // Vertical corridor: check +/- 4 tiles vertically
                for (int dy = -4; dy <= 4; ++dy) {
                    if (dy == 0) continue;
                    int ny = p.y + dy;
                    if (maze.isValid(p.x, ny) && maze.at(p.x, ny).getType() == core::CellType::Mud) {
                        if (std::abs(dy) > 1) {
                            return false; // Existing puddle too close in this corridor!
                        }
                    }
                }
            }
            if (topWall && bottomWall) {
                // Horizontal corridor: check +/- 4 tiles horizontally
                for (int dx = -4; dx <= 4; ++dx) {
                    if (dx == 0) continue;
                    int nx = p.x + dx;
                    if (maze.isValid(nx, p.y) && maze.at(nx, p.y).getType() == core::CellType::Mud) {
                        if (std::abs(dx) > 1) {
                            return false; // Existing puddle too close in this corridor!
                        }
                    }
                }
            }
        }

        return true;
    };

    int placedMud = 0;
    std::uniform_real_distribution<float> prob01(0.0f, 1.0f);

    // Group items by room for fair room-by-room distribution
    std::unordered_map<int, std::vector<size_t>> roomItemIndices;
    const auto& items = maze.getItems();
    for (size_t i = 0; i < items.size(); ++i) {
        for (size_t ri = 0; ri < rooms.size(); ++ri) {
            if (rooms[ri].contains(items[i].position)) {
                roomItemIndices[static_cast<int>(ri)].push_back(i);
                break;
            }
        }
    }

    std::vector<int> shuffledRooms(rooms.size());
    std::iota(shuffledRooms.begin(), shuffledRooms.end(), 0);
    std::shuffle(shuffledRooms.begin(), shuffledRooms.end(), rng_);

    // -------------------------------------------------------------------------
    // Pass 1: Balanced Tactical Loot Guards (Distributed across rooms with scrap)
    // -------------------------------------------------------------------------
    int lootBudget = std::max(1, targetMudCount * 45 / 100);
    int lootPlaced = 0;

    for (int rIdx : shuffledRooms) {
        if (lootPlaced >= lootBudget || placedMud >= targetMudCount) break;

        auto it = roomItemIndices.find(rIdx);
        if (it == roomItemIndices.end() || it->second.empty()) continue;

        auto rItems = it->second;
        std::shuffle(rItems.begin(), rItems.end(), rng_);

        int maxPuddlesInThisRoom = (rItems.size() >= 4) ? 2 : 1;
        int puddlesPlacedInThisRoom = 0;

        for (size_t itemIdx : rItems) {
            if (puddlesPlacedInThisRoom >= maxPuddlesInThisRoom || lootPlaced >= lootBudget || placedMud >= targetMudCount) {
                break;
            }

            const auto& itemPos = items[itemIdx].position;
            std::vector<core::Point> candidates;
            int totalWalkable = 0;
            const int dx[4] = {0, 0, -1, 1};
            const int dy[4] = {-1, 1, 0, 0};

            for (int i = 0; i < 4; ++i) {
                core::Point np{itemPos.x + dx[i], itemPos.y + dy[i]};
                if (!maze.isValid(np)) continue;
                if (maze.at(np).isPassable()) {
                    totalWalkable++;
                }
                if (canPlaceMud(np) && !isUselessDeadCorner(np)) {
                    candidates.push_back(np);
                }
            }

            // Ensure at least 2 walkable neighbors exist so scrap is never trapped (>= 1 dry route)
            if (totalWalkable >= 2 && !candidates.empty()) {
                std::shuffle(candidates.begin(), candidates.end(), rng_);
                maze.at(candidates[0]).setType(core::CellType::Mud);
                placedMud++;
                lootPlaced++;
                puddlesPlacedInThisRoom++;

                // If 3+ walkable neighbors exist, with 30% chance form a 2-cell puddle
                if (totalWalkable >= 3 && candidates.size() > 1 && prob01(rng_) < 0.30f &&
                    placedMud < targetMudCount && lootPlaced < lootBudget) {
                    if (canPlaceMud(candidates[1])) {
                        maze.at(candidates[1]).setType(core::CellType::Mud);
                        placedMud++;
                        lootPlaced++;
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Pass 2: Distinct Inter-Room Corridor Chokepoints (At most 1 puddle per corridor)
    // -------------------------------------------------------------------------
    if (placedMud < targetMudCount) {
        std::vector<core::Point> corridorCandidates;
        for (const auto& p : allFloorCells) {
            if (maze.at(p).getType() == core::CellType::Floor && isCorridorBottleneck(p)) {
                bool insideRoom = false;
                for (const auto& r : rooms) {
                    if (r.contains(p)) {
                        insideRoom = true;
                        break;
                    }
                }
                if (!insideRoom) {
                    corridorCandidates.push_back(p);
                }
            }
        }

        std::shuffle(corridorCandidates.begin(), corridorCandidates.end(), rng_);
        int corridorBudget = std::max(1, targetMudCount * 30 / 100);
        int corridorPlaced = 0;

        for (const auto& cp : corridorCandidates) {
            if (corridorPlaced >= corridorBudget || placedMud >= targetMudCount) break;
            if (!canPlaceMud(cp)) continue;

            maze.at(cp).setType(core::CellType::Mud);
            placedMud++;
            corridorPlaced++;

            // With 30% chance, expand along corridor by exactly 1 cell (max 2-cell puddle)
            if (prob01(rng_) < 0.30f && corridorPlaced < corridorBudget && placedMud < targetMudCount) {
                const int dx[4] = {0, 0, -1, 1};
                const int dy[4] = {-1, 1, 0, 0};
                for (int i = 0; i < 4; ++i) {
                    core::Point np{cp.x + dx[i], cp.y + dy[i]};
                    if (isCorridorBottleneck(np) && canPlaceMud(np)) {
                        maze.at(np).setType(core::CellType::Mud);
                        placedMud++;
                        corridorPlaced++;
                        break;
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Pass 3: Room Transit Thoroughfares (Ensuring empty / transit rooms also have hazards)
    // -------------------------------------------------------------------------
    if (placedMud < targetMudCount) {
        for (int rIdx : shuffledRooms) {
            if (placedMud >= targetMudCount) break;

            const auto& r = rooms[rIdx];
            int roomMudCount = 0;
            for (int y = r.y; y < r.y + r.height; ++y) {
                for (int x = r.x; x < r.x + r.width; ++x) {
                    if (maze.isValid(x, y) && maze.at(x, y).getType() == core::CellType::Mud) {
                        roomMudCount++;
                    }
                }
            }

            // If room has 0 mud (e.g. left room or transit room), place an interior thoroughfare puddle
            if (roomMudCount == 0 || (r.width >= 6 && r.height >= 6 && roomMudCount <= 1)) {
                std::vector<core::Point> interiorCells;
                for (int y = r.y + 1; y < r.y + r.height - 1; ++y) {
                    for (int x = r.x + 1; x < r.x + r.width - 1; ++x) {
                        core::Point p{x, y};
                        if (canPlaceMud(p) && !isUselessDeadCorner(p)) {
                            interiorCells.push_back(p);
                        }
                    }
                }

                if (!interiorCells.empty()) {
                    std::shuffle(interiorCells.begin(), interiorCells.end(), rng_);
                    core::Point chosen = interiorCells[0];
                    maze.at(chosen).setType(core::CellType::Mud);
                    placedMud++;

                    if (prob01(rng_) < 0.25f && placedMud < targetMudCount) {
                        const int dx[4] = {0, 0, -1, 1};
                        const int dy[4] = {-1, 1, 0, 0};
                        for (int i = 0; i < 4; ++i) {
                            core::Point np{chosen.x + dx[i], chosen.y + dy[i]};
                            if (canPlaceMud(np) && !isUselessDeadCorner(np)) {
                                maze.at(np).setType(core::CellType::Mud);
                                placedMud++;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Pass 4: Fallback for any remaining budget
    // -------------------------------------------------------------------------
    if (placedMud < targetMudCount) {
        std::vector<core::Point> remainingCandidates;
        for (const auto& p : allFloorCells) {
            if (canPlaceMud(p) && !isUselessDeadCorner(p)) {
                remainingCandidates.push_back(p);
            }
        }
        std::shuffle(remainingCandidates.begin(), remainingCandidates.end(), rng_);
        for (const auto& p : remainingCandidates) {
            if (placedMud >= targetMudCount) break;
            if (canPlaceMud(p)) {
                maze.at(p).setType(core::CellType::Mud);
                placedMud++;
            }
        }
    }

    // -------------------------------------------------------------------------
    // Safety Invariant: Absolutely no scrap item resides on mud
    // -------------------------------------------------------------------------
    for (const auto& item : maze.getItems()) {
        if (maze.isValid(item.position) && maze.at(item.position).getType() == core::CellType::Mud) {
            maze.at(item.position).setType(core::CellType::Floor);
        }
    }

    // Safety Invariant: Start and Exit and immediate 4-neighbors have no mud
    auto clearMudAround = [&](const core::Point& pt) {
        if (!maze.isValid(pt)) return;
        const int dx[5] = {0, 0, 0, -1, 1};
        const int dy[5] = {0, -1, 1, 0, 0};
        for (int i = 0; i < 5; ++i) {
            core::Point p{pt.x + dx[i], pt.y + dy[i]};
            if (maze.isValid(p) && maze.at(p).getType() == core::CellType::Mud) {
                maze.at(p).setType(core::CellType::Floor);
            }
        }
    };
    clearMudAround(maze.getStartPoint());
    clearMudAround(maze.getExitPoint());
}

void BSPGenerator::spawnResources(Maze& maze) {
    auto& items = maze.getItems();
    items.clear();

    const auto& templates = core::getLethalScrapTemplates();
    if (templates.empty()) return;

    // Smart scaling: at 30x30 default is 30 items, scaling gracefully with map dimensions (18-20 for 20x20, 30 for 30x30, 43 for 40x40, ~70 for 62x62, ~120 for 100x100)
    int targetItemCount = std::max(20, static_cast<int>(30.0f + (config_.mapWidth - 30) * 1.30f));

    const auto& rooms = maze.getRooms();
    std::vector<int> roomItemCounts(rooms.size(), 0);

    auto getRoomIndex = [&](const core::Point& p) -> int {
        for (size_t i = 0; i < rooms.size(); ++i) {
            if (rooms[i].contains(p)) return static_cast<int>(i);
        }
        return -1; // Corridor / hallway cell
    };

    auto getMaxItemsForRoom = [&](int rIdx) -> int {
        if (rIdx < 0 || rIdx >= static_cast<int>(rooms.size())) return 0;
        const auto& r = rooms[rIdx];
        int area = r.width * r.height;
        if (area < 25) return 1;
        if (area < 45) return 2;
        if (area < 75) return 3;
        if (area < 120) return 4;
        return 5;
    };

    const auto& startPos = maze.getStartPoint();
    const auto& exitPos = maze.getExitPoint();

    // Collect all valid Floor cells with safety distance from Start and Exit
    std::vector<core::Point> candidatePositions;
    candidatePositions.reserve(maze.getWidth() * maze.getHeight() / 2);

    for (int y = 0; y < maze.getHeight(); ++y) {
        for (int x = 0; x < maze.getWidth(); ++x) {
            core::Point pos{x, y};
            if (pos == startPos || pos == exitPos) continue;
            const auto& cell = maze.at(pos);
            if (cell.getType() == core::CellType::Floor) {
                // Safety buffer: at least 3 tiles from entrance, 2 from exit
                if (std::abs(pos.x - startPos.x) + std::abs(pos.y - startPos.y) >= 3 &&
                    std::abs(pos.x - exitPos.x) + std::abs(pos.y - exitPos.y) >= 2) {
                    candidatePositions.push_back(pos);
                }
            }
        }
    }

    if (candidatePositions.empty()) return;

    // Shuffle candidate positions for randomized distribution
    std::shuffle(candidatePositions.begin(), candidatePositions.end(), rng_);

    std::vector<core::Point> chosenPositions;
    chosenPositions.reserve(targetItemCount);

    std::vector<bool> chosenMask(maze.getWidth() * maze.getHeight(), false);
    auto posKey = [&](const core::Point& p) { return p.y * maze.getWidth() + p.x; };

    auto canPlaceAt = [&](const core::Point& p, float minItemDist, float minCorridorDist, bool checkRoomCap) -> bool {
        int rIdx = getRoomIndex(p);
        if (checkRoomCap && rIdx >= 0) {
            if (roomItemCounts[rIdx] >= getMaxItemsForRoom(rIdx)) {
                return false;
            }
        }

        float reqDist = (rIdx < 0) ? minCorridorDist : minItemDist;
        for (const auto& existing : chosenPositions) {
            int dx = std::abs(p.x - existing.x);
            int dy = std::abs(p.y - existing.y);

            // HARD ANTI-CLUMPING INVARIANTS:
            // 1. Strictly forbid directly adjacent (dx+dy <= 1) and diagonal neighbor (max(dx, dy) < 2)
            if (std::max(dx, dy) < 2) return false;
            // 2. Strictly forbid 2-tile linear or knight moves closer than Manhattan 3
            if (dx + dy < 3) return false;

            float dist = std::hypot(static_cast<float>(dx), static_cast<float>(dy));
            if (dist < reqDist) return false;
        }
        return true;
    };

    auto tryPlacementPass = [&](float minItemDist, float minCorridorDist, bool checkRoomCap) {
        for (const auto& pos : candidatePositions) {
            if (static_cast<int>(chosenPositions.size()) >= targetItemCount) break;
            if (chosenMask[posKey(pos)]) continue;

            if (canPlaceAt(pos, minItemDist, minCorridorDist, checkRoomCap)) {
                chosenPositions.push_back(pos);
                chosenMask[posKey(pos)] = true;
                int rIdx = getRoomIndex(pos);
                if (rIdx >= 0) {
                    roomItemCounts[rIdx]++;
                }
            }
        }
    };

    // Pass 1: Generous tactical distribution (wide spacing in rooms and corridors)
    tryPlacementPass(4.0f, 5.0f, true);

    // Pass 2: Balanced distribution
    if (static_cast<int>(chosenPositions.size()) < targetItemCount) {
        tryPlacementPass(3.0f, 4.0f, true);
    }

    // Pass 3: Moderate distribution, slightly relaxing room caps for remaining items
    if (static_cast<int>(chosenPositions.size()) < targetItemCount) {
        tryPlacementPass(2.8f, 3.5f, false);
    }

    // Pass 4: Fallback minimum safe distance (still strictly enforcing max(dx, dy) >= 2 and dx+dy >= 3)
    if (static_cast<int>(chosenPositions.size()) < targetItemCount) {
        tryPlacementPass(2.2f, 3.0f, false);
    }

    int toSpawn = static_cast<int>(chosenPositions.size());
    items.reserve(toSpawn);

    std::vector<int> lightIndices, medIndices, heavyIndices;
    for (size_t idx = 0; idx < templates.size(); ++idx) {
        if (templates[idx].category == core::ItemCategory::LightScrap) {
            lightIndices.push_back(static_cast<int>(idx));
        } else if (templates[idx].category == core::ItemCategory::MediumScrap) {
            medIndices.push_back(static_cast<int>(idx));
        } else {
            heavyIndices.push_back(static_cast<int>(idx));
        }
    }

    std::uniform_real_distribution<float> catDist(0.0f, 1.0f);
    std::uniform_int_distribution<size_t> lightIdxDist(0, lightIndices.empty() ? 0 : lightIndices.size() - 1);
    std::uniform_int_distribution<size_t> medIdxDist(0, medIndices.empty() ? 0 : medIndices.size() - 1);
    std::uniform_int_distribution<size_t> heavyIdxDist(0, heavyIndices.empty() ? 0 : heavyIndices.size() - 1);
    std::uniform_int_distribution<int> weightDeltaDist(-1, 1);
    std::uniform_int_distribution<int> valueDeltaDist(-5, 5);

    for (int i = 0; i < toSpawn; ++i) {
        core::Point pos = chosenPositions[i];
        Cell& cell = maze.at(pos);

        float roll = catDist(rng_);
        int tmplIdx = 0;
        if (roll < 0.45f && !lightIndices.empty()) {
            // 45% Light scrap (cheap, light, high frequency)
            tmplIdx = lightIndices[lightIdxDist(rng_)];
        } else if (roll < 0.80f && !medIndices.empty()) {
            // 35% Medium scrap (balanced)
            tmplIdx = medIndices[medIdxDist(rng_)];
        } else if (!heavyIndices.empty()) {
            // 20% Heavy scrap (heavy, very valuable, rarer)
            tmplIdx = heavyIndices[heavyIdxDist(rng_)];
        } else {
            tmplIdx = 0;
        }

        const auto& tmpl = templates[tmplIdx];

        core::Item item;
        item.id = i + 1;
        item.position = pos;
        item.name = tmpl.name;
        item.category = tmpl.category;

        // Apply subtle realistic variance to weight and credits
        int finalWeight = std::max(1, tmpl.baseWeight + weightDeltaDist(rng_));
        int finalValue = std::max(15, tmpl.baseValue + valueDeltaDist(rng_));

        // Clamp to category limits
        if (tmpl.category == core::ItemCategory::LightScrap) {
            finalWeight = std::clamp(finalWeight, 1, 3);
        } else if (tmpl.category == core::ItemCategory::MediumScrap) {
            finalWeight = std::clamp(finalWeight, 4, 8);
        } else {
            finalWeight = std::clamp(finalWeight, 10, 18);
        }

        item.weight = finalWeight;
        item.value = finalValue;

        cell.setResourceId(item.id);
        items.push_back(item);
    }
}

} // namespace map
