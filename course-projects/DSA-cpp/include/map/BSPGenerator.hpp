#pragma once

#include "Maze.hpp"
#include "core/Config.hpp"
#include <random>
#include <memory>

namespace map {

class BSPGenerator {
public:
    explicit BSPGenerator(const core::Config& config = core::Config{})
        : config_(config), rng_(config.randomSeed) {}

    void setConfig(const core::Config& config) {
        config_ = config;
        rng_.seed(config.randomSeed);
    }

    // Generate procedural maze into target Maze object
    void generate(Maze& maze);

private:
    struct BSPNode {
        int x, y, width, height;
        std::unique_ptr<BSPNode> left;
        std::unique_ptr<BSPNode> right;
        std::optional<Room> room;

        BSPNode(int x_, int y_, int w_, int h_)
            : x(x_), y(y_), width(w_), height(h_) {}

        [[nodiscard]] bool isLeaf() const noexcept {
            return !left && !right;
        }
    };

    void splitNode(BSPNode* node, int depth);
    void createRooms(BSPNode* node, Maze& maze, std::vector<Room>& rooms);
    void connectTree(BSPNode* node, Maze& maze);
    void carveCorridor(Maze& maze, const core::Point& from, const core::Point& to);
    core::Point getRoomCenterFromNode(const BSPNode* node);

    void addExtraCycles(Maze& maze, const std::vector<Room>& rooms);
    void spawnMudHazards(Maze& maze);
    void spawnResources(Maze& maze);
    void ensureConnectivity(Maze& maze);

    core::Config config_;
    std::mt19937 rng_;
};

} // namespace map
