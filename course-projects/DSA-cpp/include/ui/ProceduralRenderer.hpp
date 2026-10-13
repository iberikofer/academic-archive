#pragma once

#include "algorithms/routing/RoutePlan.hpp"
#include "map/Maze.hpp"
#include "simulation/Agent.hpp"
#include "simulation/SimulationEngine.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <vector>

namespace ui {

struct PickupEffect {
  sf::Vector2f worldPos;
  core::ItemCategory category;
  int value = 0;
  int weight = 0;
  float elapsed = 0.0f;
  float duration = 1.0f;
};

struct CellPaintEffect {
  int x = 0;
  int y = 0;
  float elapsed = 0.0f;
  float duration = 0.32f;
};

enum class GenerationTransitionPhase {
  None,
  FadeOut,
  SonarScan,
  PostSonarPause,
  RouteTracing,
  PostRoutePause,
  CameraZoomingToAgent
};

struct GenerationTransition {
  GenerationTransitionPhase phase = GenerationTransitionPhase::None;
  float timer = 0.0f;
  float fadeOutDuration = 0.35f;
  float sonarDuration = 3.00f;
  float postSonarPauseDuration = 1.00f;
  float routeTracingDuration = 4.20f;
  float postRoutePauseDuration = 0.50f;
  float cameraZoomDuration = 0.50f;

  sf::Vector2f scanOrigin{0.0f, 0.0f};
  float maxRadius = 1200.0f;

  // Pending seed data for regeneration
  bool hasPendingRegen = false;
  bool pendingUseSpecifiedSeed = false;
  unsigned int pendingSpecifiedSeedValue = 0;
  bool isRecalculatingRoute = false;

  void startFadeOut(bool useSpecifiedSeed, unsigned int seedValue) {
    isRecalculatingRoute = false;
    routeTracingDuration = 4.20f;
    phase = GenerationTransitionPhase::FadeOut;
    timer = 0.0f;
    hasPendingRegen = true;
    pendingUseSpecifiedSeed = useSpecifiedSeed;
    pendingSpecifiedSeedValue = seedValue;
  }

  void startStartupScan(sf::Vector2f origin, float radius) {
    isRecalculatingRoute = false;
    routeTracingDuration = 4.20f;
    phase = GenerationTransitionPhase::SonarScan;
    timer = 0.0f;
    scanOrigin = origin;
    maxRadius = radius;
    hasPendingRegen = false;
  }

  void startRouteTracingOnly(float duration = 2.80f) {
    phase = GenerationTransitionPhase::RouteTracing;
    timer = 0.0f;
    routeTracingDuration = duration;
    isRecalculatingRoute = true;
  }

  [[nodiscard]] bool isTransitionActive() const noexcept {
    if (isRecalculatingRoute)
      return false;
    return phase != GenerationTransitionPhase::None &&
           phase != GenerationTransitionPhase::CameraZoomingToAgent;
  }

  [[nodiscard]] bool isFadingOut() const noexcept {
    return phase == GenerationTransitionPhase::FadeOut;
  }

  [[nodiscard]] bool isSonarScanning() const noexcept {
    return phase == GenerationTransitionPhase::SonarScan;
  }

  [[nodiscard]] bool isPostSonarPause() const noexcept {
    return phase == GenerationTransitionPhase::PostSonarPause;
  }

  [[nodiscard]] bool isRouteTracing() const noexcept {
    return phase == GenerationTransitionPhase::RouteTracing;
  }

  [[nodiscard]] bool isPostRoutePause() const noexcept {
    return phase == GenerationTransitionPhase::PostRoutePause;
  }

  [[nodiscard]] bool isCameraZooming() const noexcept {
    return phase == GenerationTransitionPhase::CameraZoomingToAgent;
  }

  [[nodiscard]] float getFadeAlpha() const noexcept {
    if (phase != GenerationTransitionPhase::FadeOut)
      return 0.0f;
    return std::clamp(timer / fadeOutDuration, 0.0f, 1.0f);
  }

  [[nodiscard]] float getSonarProgress() const noexcept {
    if (phase == GenerationTransitionPhase::SonarScan) {
      return std::clamp(timer / sonarDuration, 0.0f, 1.0f);
    }
    if (phase == GenerationTransitionPhase::FadeOut)
      return 0.0f;
    return 1.0f;
  }

  [[nodiscard]] float getCurrentScanRadius() const noexcept {
    if (phase == GenerationTransitionPhase::SonarScan) {
      float t = getSonarProgress();
      // Smooth ease in-out curve so waves smoothly accelerate from center
      float ease = t * t * (3.0f - 2.0f * t);
      return maxRadius * ease;
    }
    if (phase == GenerationTransitionPhase::FadeOut)
      return -1.0f;
    return maxRadius * 1.5f; // Fully revealed for subsequent phases
  }

  [[nodiscard]] float getRouteProgress() const noexcept {
    if (phase == GenerationTransitionPhase::RouteTracing) {
      return std::clamp(timer / routeTracingDuration, 0.0f, 1.0f);
    }
    if (phase == GenerationTransitionPhase::PostRoutePause ||
        phase == GenerationTransitionPhase::CameraZoomingToAgent ||
        phase == GenerationTransitionPhase::None) {
      return 1.0f;
    }
    return 0.0f;
  }
};

class ProceduralRenderer {
public:
  static constexpr float TILE_SIZE = 36.0f;

  void resetAnimation() {
    visualAgentInitialized_ = false;
    pickupEffects_.clear();
    lastCollectedIds_.clear();
    paintEffects_.clear();
    visitedHistorySet_.clear();
    routeCompletionTimer_ = 0.0f;
    wasRouteCompleted_ = false;
    catalogHoverAlpha_ = 0.0f;
    lastActiveCatalogItemName_.clear();
    lastActiveCatalogCategory_ = -1;
  }

  [[nodiscard]] sf::Vector2f getVisualAgentPos() const noexcept {
    return visualAgentPos_;
  }

  void update(float dt, const simulation::SimulationEngine &engine,
              const map::Maze &maze) {
    lastDt_ = std::clamp(dt, 0.001f, 0.1f);
    animTime_ += dt;
    if (routeCompletionTimer_ > 0.0f) {
      routeCompletionTimer_ = std::max(0.0f, routeCompletionTimer_ - dt);
    }
    const auto &agent = engine.getAgent();

    // 1. Smooth agent movement interpolation
    sf::Vector2f targetPos{agent.getPosition().x * TILE_SIZE + TILE_SIZE * 0.5f,
                           agent.getPosition().y * TILE_SIZE +
                               TILE_SIZE * 0.5f};

    if (!visualAgentInitialized_) {
      visualAgentPos_ = targetPos;
      visualAgentInitialized_ = true;
    } else {
      float dist = std::hypot(targetPos.x - visualAgentPos_.x,
                              targetPos.y - visualAgentPos_.y);
      if (dist > TILE_SIZE * 3.5f) {
        // Instant snap on long jumps / resets
        visualAgentPos_ = targetPos;
      } else if (dist > 0.05f) {
        float speedMul = engine.getSpeedMultiplier();
        float lerpRate = 14.0f * std::clamp(speedMul, 0.8f, 5.0f);
        float factor = 1.0f - std::exp(-lerpRate * dt);
        visualAgentPos_ += (targetPos - visualAgentPos_) * factor;
      } else {
        visualAgentPos_ = targetPos;
      }
    }

    // 2. Track item pickup events
    const auto &currentCollected = engine.getCollectedItemIds();
    for (int id : currentCollected) {
      if (std::find(lastCollectedIds_.begin(), lastCollectedIds_.end(), id) ==
          lastCollectedIds_.end()) {
        // Item newly collected! Spawn visual pickup effect
        auto itemOpt = maze.getItemById(id);
        if (itemOpt.has_value()) {
          sf::Vector2f itemWorldPos{
              itemOpt->position.x * TILE_SIZE + TILE_SIZE * 0.5f,
              itemOpt->position.y * TILE_SIZE + TILE_SIZE * 0.5f};
          pickupEffects_.push_back({itemWorldPos, itemOpt->category,
                                    itemOpt->value, itemOpt->weight, 0.0f,
                                    1.0f});
        }
      }
    }
    lastCollectedIds_ = currentCollected;

    // 3. Advance active pickup effects
    for (auto &effect : pickupEffects_) {
      effect.elapsed += dt;
    }
    pickupEffects_.erase(std::remove_if(pickupEffects_.begin(),
                                        pickupEffects_.end(),
                                        [](const PickupEffect &e) {
                                          return e.elapsed >= e.duration;
                                        }),
                         pickupEffects_.end());

    // 4. Track square coloring animation for freshly stepped-on cells
    core::Point agentPos = engine.getAgent().getPosition();
    int cellKey = agentPos.y * 10000 + agentPos.x;

    if (maze.isValid(agentPos) && maze.at(agentPos).isWalked() &&
        maze.at(agentPos).getType() != core::CellType::Start &&
        maze.at(agentPos).getType() != core::CellType::Exit) {
      if (visitedHistorySet_.find(cellKey) == visitedHistorySet_.end()) {
        visitedHistorySet_.insert(cellKey);
        float speedMul = std::max(0.6f, engine.getSpeedMultiplier());
        float fillDur = std::clamp(0.35f / speedMul, 0.08f, 0.45f);
        paintEffects_.push_back({agentPos.x, agentPos.y, 0.0f, fillDur});
      }
    }

    // Clean up history and active paint effects if steps were undone (e.g. step
    // backward)
    for (auto it = visitedHistorySet_.begin();
         it != visitedHistorySet_.end();) {
      int k = *it;
      int cx = k % 10000;
      int cy = k / 10000;
      if (maze.isValid(cx, cy) && !maze.at(cx, cy).isWalked()) {
        it = visitedHistorySet_.erase(it);
      } else {
        ++it;
      }
    }
    paintEffects_.erase(std::remove_if(paintEffects_.begin(),
                                       paintEffects_.end(),
                                       [&maze](const CellPaintEffect &e) {
                                         return maze.isValid(e.x, e.y) &&
                                                !maze.at(e.x, e.y).isWalked();
                                       }),
                        paintEffects_.end());

    // Advance active paint effects
    for (auto &effect : paintEffects_) {
      effect.elapsed += dt;
    }
    paintEffects_.erase(std::remove_if(paintEffects_.begin(),
                                       paintEffects_.end(),
                                       [](const CellPaintEffect &e) {
                                         return e.elapsed >= e.duration;
                                       }),
                        paintEffects_.end());
  }

  static constexpr int SCAN_PAD_TILES = 16;

  void updateCachedDistances(const map::Maze &maze, sf::Vector2f scanOrigin) {
    int w = maze.getWidth();
    int h = maze.getHeight();
    int totalW = w + 2 * SCAN_PAD_TILES;
    int totalH = h + 2 * SCAN_PAD_TILES;
    if (cachedScanOrigin_ != scanOrigin || cachedMazeW_ != w ||
        cachedMazeH_ != h ||
        cachedDistances_.size() != static_cast<size_t>(totalW * totalH)) {
      cachedScanOrigin_ = scanOrigin;
      cachedMazeW_ = w;
      cachedMazeH_ = h;
      cachedDistances_.resize(totalW * totalH);
      for (int ly = 0; ly < totalH; ++ly) {
        int y = ly - SCAN_PAD_TILES;
        float cy = (y + 0.5f) * TILE_SIZE;
        float dy = cy - scanOrigin.y;
        for (int lx = 0; lx < totalW; ++lx) {
          int x = lx - SCAN_PAD_TILES;
          float cx = (x + 0.5f) * TILE_SIZE;
          float dx = cx - scanOrigin.x;
          cachedDistances_[ly * totalW + lx] = std::sqrt(dx * dx + dy * dy);
        }
      }
    }
  }

  void renderMaze(sf::RenderTarget &target, const map::Maze &maze,
                  bool showGrid, float scanRadius = -1.0f,
                  sf::Vector2f scanOrigin = {0.0f, 0.0f},
                  bool isExitDisabled = false) {
    if (scanRadius >= 0.0f) {
      updateCachedDistances(maze, scanOrigin);
    }

    int w = maze.getWidth();
    int h = maze.getHeight();
    int totalW = w + 2 * SCAN_PAD_TILES;
    int totalH = h + 2 * SCAN_PAD_TILES;

    // High-performance batched vertex arrays for radar ping wave highlights
    // across maze and outer margins (zero-allocation cached capacity)
    auto &pingQuads = pingQuads_;
    auto &pingBorders = pingBorders_;
    if (scanRadius >= 0.0f) {
      size_t neededQuads = static_cast<size_t>(totalW * totalH * 6);
      size_t neededBorders = static_cast<size_t>(totalW * totalH * 8);
      if (pingQuads.getVertexCount() < neededQuads) {
        pingQuads.resize(neededQuads);
      }
      if (pingBorders.getVertexCount() < neededBorders) {
        pingBorders.resize(neededBorders);
      }
    }
    size_t pingQuadCount = 0;
    size_t pingBorderCount = 0;

    // 1. Batched base terrain quads (Geometry Batching: 1 draw call for all base tiles!)
    auto &baseQuads = baseTileQuads_;
    size_t neededBase = static_cast<size_t>(w * h * 6);
    if (baseQuads.getVertexCount() < neededBase) {
      baseQuads.resize(neededBase);
    }
    size_t baseQuadCount = 0;

    for (int y = 0; y < maze.getHeight(); ++y) {
      for (int x = 0; x < maze.getWidth(); ++x) {
        if (scanRadius >= 0.0f) {
          int lx = x + SCAN_PAD_TILES;
          int ly = y + SCAN_PAD_TILES;
          float dist = cachedDistances_[ly * totalW + lx];
          if (dist > scanRadius + 14.0f) {
            continue;
          }
        }

        const auto &cell = maze.at(x, y);
        sf::Color col;
        switch (cell.getType()) {
        case core::CellType::Wall:
          col = sf::Color(22, 27, 36);
          break;
        case core::CellType::Floor:
          col = sf::Color(44, 52, 64);
          break;
        case core::CellType::Mud:
          col = sf::Color(58, 44, 28);
          break;
        case core::CellType::Start:
          col = sf::Color(180, 95, 12);
          break;
        case core::CellType::Exit:
          col = isExitDisabled ? sf::Color(36, 40, 48) : sf::Color(24, 110, 60);
          break;
        }

        float px = x * TILE_SIZE;
        float py = y * TILE_SIZE;
        float pxNext = px + TILE_SIZE;
        float pyNext = py + TILE_SIZE;

        baseQuads[baseQuadCount + 0] = {{px, py}, col};
        baseQuads[baseQuadCount + 1] = {{pxNext, py}, col};
        baseQuads[baseQuadCount + 2] = {{pxNext, pyNext}, col};
        baseQuads[baseQuadCount + 3] = {{px, py}, col};
        baseQuads[baseQuadCount + 4] = {{pxNext, pyNext}, col};
        baseQuads[baseQuadCount + 5] = {{px, pyNext}, col};
        baseQuadCount += 6;
      }
    }

    if (baseQuadCount > 0) {
      baseQuads.resize(baseQuadCount);
      target.draw(baseQuads);
    }

    // 2. Special animated cells (sludge puddles/bubbles, airlock hatches, walked trails, search highlights)
    for (int y = 0; y < maze.getHeight(); ++y) {
      for (int x = 0; x < maze.getWidth(); ++x) {
        const auto &cell = maze.at(x, y);
        auto cType = cell.getType();
        bool isSpecial = (cType == core::CellType::Mud ||
                          cType == core::CellType::Start ||
                          cType == core::CellType::Exit ||
                          cell.isWalked() ||
                          cell.isVisitedBySearch());
        if (!isSpecial) {
          continue; // Plain wall and floor cells skipped instantly!
        }

        float px = x * TILE_SIZE;
        float py = y * TILE_SIZE;

        if (scanRadius >= 0.0f) {
          int lx = x + SCAN_PAD_TILES;
          int ly = y + SCAN_PAD_TILES;
          float dist = cachedDistances_[ly * totalW + lx];
          if (dist > scanRadius + 14.0f) {
            continue;
          }
        }

        switch (cell.getType()) {
        case core::CellType::Wall:
        case core::CellType::Floor:
          break;

        case core::CellType::Mud: {

          // Darker inner sludge puddle
          sf::RectangleShape puddle({TILE_SIZE - 6.0f, TILE_SIZE - 6.0f});
          puddle.setPosition({px + 3.0f, py + 3.0f});
          puddle.setFillColor(sf::Color(40, 29, 17, 230));
          target.draw(puddle);

          // 1. Animated surface ripple (pulsing square) - slow viscous sludge
          // expansion
          float ripPhase = std::fmod(
              animTime_ * 0.32f + (x * 7.0f + y * 13.0f) * 0.2f, 1.0f);
          if (ripPhase < 0.0f)
            ripPhase += 1.0f;
          float ripSize = 4.0f + ripPhase * 16.0f;
          sf::RectangleShape rippleSquare({ripSize, ripSize});
          rippleSquare.setOrigin({ripSize * 0.5f, ripSize * 0.5f});
          rippleSquare.setPosition(
              {px + TILE_SIZE * 0.5f, py + TILE_SIZE * 0.5f});
          rippleSquare.setFillColor(sf::Color::Transparent);
          rippleSquare.setOutlineThickness(1.0f);
          rippleSquare.setOutlineColor(sf::Color(
              88, 68, 44, static_cast<uint8_t>((1.0f - ripPhase) * 140.0f)));
          target.draw(rippleSquare);

          // 2. Multiple square sludge bubbles that spawn and pop dynamically
          // (slow, heavy bubbling)
          struct SludgeBubble {
            float relX, relY;
            float speed;
            float phaseOffset;
            float maxSize;
          };

          auto mudHash = [](int cx, int cy, int s) -> float {
            uint32_t n = static_cast<uint32_t>(cx * 73856093 ^ cy * 19349663 ^
                                               s * 83492791);
            n = (n ^ (n >> 13)) * 1274126177;
            return static_cast<float>((n ^ (n >> 16)) & 0xFFFF) / 65535.0f;
          };

          constexpr float MARGIN_MIN = 6.5f;
          constexpr float MARGIN_MAX = 29.5f;
          constexpr float SPAN = MARGIN_MAX - MARGIN_MIN;
          constexpr float MIN_DIST_SQ = 9.0f * 9.0f;

          // Bubble 0: randomized position within safe margins
          sf::Vector2f b0{MARGIN_MIN + mudHash(x, y, 101) * SPAN,
                          MARGIN_MIN + mudHash(x, y, 102) * SPAN};

          // Bubble 1: randomized position, rejecting candidates overlapping
          // with b0
          sf::Vector2f b1{0.0f, 0.0f};
          for (int i = 0; i < 16; ++i) {
            float cx = MARGIN_MIN + mudHash(x, y, 200 + i * 2) * SPAN;
            float cy = MARGIN_MIN + mudHash(x, y, 201 + i * 2) * SPAN;
            float dx = cx - b0.x;
            float dy = cy - b0.y;
            if (dx * dx + dy * dy >= MIN_DIST_SQ) {
              b1 = {cx, cy};
              break;
            }
          }
          if (b1.x == 0.0f && b1.y == 0.0f) {
            float fallbackX = (b0.x > 18.0f) ? (b0.x - 11.0f) : (b0.x + 11.0f);
            float fallbackY = (b0.y > 18.0f) ? (b0.y - 11.0f) : (b0.y + 11.0f);
            b1 = {std::clamp(fallbackX, MARGIN_MIN, MARGIN_MAX),
                  std::clamp(fallbackY, MARGIN_MIN, MARGIN_MAX)};
          }

          // Bubble 2: randomized position, rejecting candidates overlapping
          // with b0 or b1
          sf::Vector2f b2{0.0f, 0.0f};
          for (int i = 0; i < 32; ++i) {
            float cx = MARGIN_MIN + mudHash(x, y, 300 + i * 2) * SPAN;
            float cy = MARGIN_MIN + mudHash(x, y, 301 + i * 2) * SPAN;
            float d0x = cx - b0.x, d0y = cy - b0.y;
            float d1x = cx - b1.x, d1y = cy - b1.y;
            if (d0x * d0x + d0y * d0y >= MIN_DIST_SQ &&
                d1x * d1x + d1y * d1y >= MIN_DIST_SQ) {
              b2 = {cx, cy};
              break;
            }
          }
          if (b2.x == 0.0f && b2.y == 0.0f) {
            float candX[4] = {MARGIN_MIN + 3.0f, MARGIN_MAX - 3.0f,
                              MARGIN_MIN + 3.0f, MARGIN_MAX - 3.0f};
            float candY[4] = {MARGIN_MIN + 3.0f, MARGIN_MIN + 3.0f,
                              MARGIN_MAX - 3.0f, MARGIN_MAX - 3.0f};
            float bestDist = -1.0f;
            for (int k = 0; k < 4; ++k) {
              float d0 = std::hypot(candX[k] - b0.x, candY[k] - b0.y);
              float d1 = std::hypot(candX[k] - b1.x, candY[k] - b1.y);
              float minD = std::min(d0, d1);
              if (minD > bestDist) {
                bestDist = minD;
                b2 = {candX[k], candY[k]};
              }
            }
          }

          const SludgeBubble bubbles[3] = {
              {b0.x, b0.y, 0.38f + mudHash(x, y, 401) * 0.22f,
               mudHash(x, y, 501) * 6.28f, 3.0f + mudHash(x, y, 601) * 1.2f},
              {b1.x, b1.y, 0.38f + mudHash(x, y, 402) * 0.22f,
               mudHash(x, y, 502) * 6.28f, 2.9f + mudHash(x, y, 602) * 1.1f},
              {b2.x, b2.y, 0.38f + mudHash(x, y, 403) * 0.22f,
               mudHash(x, y, 503) * 6.28f, 3.2f + mudHash(x, y, 603) * 1.2f}};

          for (const auto &b : bubbles) {
            float cycle = std::fmod(animTime_ * b.speed + b.phaseOffset, 1.0f);
            if (cycle < 0.0f)
              cycle += 1.0f;

            // Bubble is active for 70% of the cycle (spawns, grows, pops), then
            // hidden for 30%
            if (cycle < 0.70f) {
              float lifeNorm = cycle / 0.70f;
              float growNorm = std::sin(lifeNorm * 3.14159265f);

              float curSize = 1.2f + growNorm * (b.maxSize - 1.2f);
              uint8_t alpha = static_cast<uint8_t>(
                  std::clamp(growNorm * 220.0f, 0.0f, 220.0f));

              sf::RectangleShape sqBubble({curSize, curSize});
              sqBubble.setOrigin({curSize * 0.5f, curSize * 0.5f});
              sqBubble.setPosition({px + b.relX, py + b.relY});
              sqBubble.setFillColor(sf::Color(112, 86, 54, alpha));
              sqBubble.setOutlineThickness(0.8f);
              sqBubble.setOutlineColor(
                  sf::Color(152, 122, 80, static_cast<uint8_t>(alpha * 0.95f)));
              target.draw(sqBubble);
            }
          }

          // Hazard corner warning rivets
          sf::RectangleShape rivet({2.5f, 2.5f});
          rivet.setFillColor(sf::Color(120, 95, 55, 180));
          rivet.setPosition({px + 1.0f, py + 1.0f});
          target.draw(rivet);
          rivet.setPosition({px + TILE_SIZE - 3.5f, py + 1.0f});
          target.draw(rivet);
          rivet.setPosition({px + 1.0f, py + TILE_SIZE - 3.5f});
          target.draw(rivet);
          rivet.setPosition({px + TILE_SIZE - 3.5f, py + TILE_SIZE - 3.5f});
          target.draw(rivet);
          break;
        }

        case core::CellType::Start: {
          // Industrial Airlock Entry with Hazard Striping & Pulsing Beacon
          // Inner reinforced airlock hatch
          sf::RectangleShape hatch({TILE_SIZE - 6.0f, TILE_SIZE - 6.0f});
          hatch.setPosition({px + 3.0f, py + 3.0f});
          hatch.setFillColor(sf::Color(240, 135, 18));
          hatch.setOutlineThickness(1.5f);
          hatch.setOutlineColor(sf::Color(255, 210, 90));
          target.draw(hatch);

          // Inward contracting entrance beacon rings (strictly shrinking into
          // the airlock)
          float inPhase = std::fmod(animTime_ * 1.35f, 1.0f);
          for (float offset : {0.0f, 0.5f}) {
            float t =
                std::fmod(inPhase + offset, 1.0f); // 0.0 (outer) -> 1.0 (inner)
            // Strictly shrinking: from 11.2f at t=0 down to 2.2f at t=1
            float sRingR = 11.2f * (1.0f - t) + 2.2f * t;
            float alphaNorm =
                std::sin(t * 3.14159265f); // Smooth fade in and out
            uint8_t alpha = static_cast<uint8_t>(
                std::clamp(alphaNorm * 220.0f, 0.0f, 255.0f));

            sf::CircleShape sRing(sRingR);
            sRing.setOrigin({sRingR, sRingR});
            sRing.setPosition({px + TILE_SIZE * 0.5f, py + TILE_SIZE * 0.5f});
            sRing.setFillColor(sf::Color::Transparent);
            sRing.setOutlineThickness(1.5f);
            sRing.setOutlineColor(sf::Color(255, 235, 120, alpha));
            target.draw(sRing);
          }

          // 4 corner amber beacon LEDs
          float cornerPulse = 0.5f + 0.5f * std::sin(animTime_ * 5.0f);
          uint8_t ledAlpha = static_cast<uint8_t>(160 + 95 * cornerPulse);
          sf::RectangleShape led({3.0f, 3.0f});
          led.setFillColor(sf::Color(255, 220, 60, ledAlpha));
          led.setPosition({px + 2.0f, py + 2.0f});
          target.draw(led);
          led.setPosition({px + TILE_SIZE - 5.0f, py + 2.0f});
          target.draw(led);
          led.setPosition({px + 2.0f, py + TILE_SIZE - 5.0f});
          target.draw(led);
          led.setPosition({px + TILE_SIZE - 5.0f, py + TILE_SIZE - 5.0f});
          target.draw(led);
          break;
        }

        case core::CellType::Exit: {
          if (isExitDisabled) {
            // Decommissioned / Inaccessible Airlock Platform (Evacuation only
            // via Start/Base A)
            // Inner heavy steel hatch pad with muted warning border
            sf::RectangleShape pad({TILE_SIZE - 6.0f, TILE_SIZE - 6.0f});
            pad.setPosition({px + 3.0f, py + 3.0f});
            pad.setFillColor(sf::Color(52, 56, 66));
            pad.setOutlineThickness(1.5f);
            pad.setOutlineColor(sf::Color(80, 85, 96));
            target.draw(pad);

            // Crossed-out warning hazard bars 'X' (enlarged & bold)
            float crossLen = 22.0f;
            float barThick = 4.0f;
            sf::Vector2f center{px + TILE_SIZE * 0.5f, py + TILE_SIZE * 0.5f};

            for (float rot : {45.0f, -45.0f}) {
              sf::RectangleShape bar({crossLen, barThick});
              bar.setOrigin({crossLen * 0.5f, barThick * 0.5f});
              bar.setPosition(center);
              bar.setRotation(sf::degrees(rot));
              bar.setFillColor(sf::Color(225, 55, 55, 240));
              target.draw(bar);
            }

            // 4 corner offline/inactive indicators
            sf::RectangleShape dLed({3.0f, 3.0f});
            dLed.setFillColor(sf::Color(75, 42, 42, 170));
            dLed.setPosition({px + 2.0f, py + 2.0f});
            target.draw(dLed);
            dLed.setPosition({px + TILE_SIZE - 5.0f, py + 2.0f});
            target.draw(dLed);
            dLed.setPosition({px + 2.0f, py + TILE_SIZE - 5.0f});
            target.draw(dLed);
            dLed.setPosition({px + TILE_SIZE - 5.0f, py + TILE_SIZE - 5.0f});
            target.draw(dLed);
            break;
          }

          // Emergency Extraction Airlock Platform with Holographic Field
          // Inner landing pad
          sf::RectangleShape pad({TILE_SIZE - 6.0f, TILE_SIZE - 6.0f});
          pad.setPosition({px + 3.0f, py + 3.0f});
          pad.setFillColor(sf::Color(40, 185, 100));
          pad.setOutlineThickness(1.5f);
          pad.setOutlineColor(sf::Color(140, 255, 185));
          target.draw(pad);

          // Outward expanding extraction radar rings (strictly growing out from
          // center)
          float outPhase = std::fmod(animTime_ * 1.35f, 1.0f);
          for (float offset : {0.0f, 0.5f}) {
            float t = std::fmod(outPhase + offset,
                                1.0f); // 0.0 (inner) -> 1.0 (outer)
            // Strictly expanding: from 2.2f at t=0 up to 11.5f at t=1
            float eRingR = 2.2f * (1.0f - t) + 11.5f * t;
            float alphaNorm =
                std::sin(t * 3.14159265f); // Smooth fade in and out
            uint8_t alpha = static_cast<uint8_t>(
                std::clamp(alphaNorm * 220.0f, 0.0f, 255.0f));

            sf::CircleShape eRing(eRingR);
            eRing.setOrigin({eRingR, eRingR});
            eRing.setPosition({px + TILE_SIZE * 0.5f, py + TILE_SIZE * 0.5f});
            eRing.setFillColor(sf::Color::Transparent);
            eRing.setOutlineThickness(1.5f);
            eRing.setOutlineColor(sf::Color(140, 255, 190, alpha));
            target.draw(eRing);
          }

          // 4 corner emerald beacon LEDs
          float cornerExitPulse = 0.5f + 0.5f * std::sin(animTime_ * 5.0f);
          uint8_t eLedAlpha = static_cast<uint8_t>(170 + 85 * cornerExitPulse);
          sf::RectangleShape eLed({3.0f, 3.0f});
          eLed.setFillColor(sf::Color(80, 255, 160, eLedAlpha));
          eLed.setPosition({px + 2.0f, py + 2.0f});
          target.draw(eLed);
          eLed.setPosition({px + TILE_SIZE - 5.0f, py + 2.0f});
          target.draw(eLed);
          eLed.setPosition({px + 2.0f, py + TILE_SIZE - 5.0f});
          target.draw(eLed);
          eLed.setPosition({px + TILE_SIZE - 5.0f, py + TILE_SIZE - 5.0f});
          target.draw(eLed);
          break;
        }
        }

        // Walked trail: animated square fill for freshly stepped-on cells,
        // static for settled cells
        if (cell.isWalked() && cell.getType() != core::CellType::Start &&
            cell.getType() != core::CellType::Exit) {
          bool isMud = (cell.getType() == core::CellType::Mud);
          auto pIt = std::find_if(paintEffects_.begin(), paintEffects_.end(),
                                  [x, y](const CellPaintEffect &e) {
                                    return e.x == x && e.y == y;
                                  });

          if (pIt != paintEffects_.end()) {
            float t = std::clamp(pIt->elapsed / pIt->duration, 0.0f, 1.0f);
            float ease = 1.0f - std::pow(1.0f - t, 3.0f); // Ease-out cubic
            float curSize = TILE_SIZE * ease;
            float halfSize = curSize * 0.5f;

            // 1. Expanding square fill from center (translucent cyan)
            sf::Color fillColor =
                isMud ? sf::Color(0, 180, 215, static_cast<uint8_t>(35 * t))
                      : sf::Color(10, 48, 65, static_cast<uint8_t>(220 * t));
            sf::Color borderColor =
                isMud
                    ? sf::Color(0, 220, 235, static_cast<uint8_t>(25 + 45 * t))
                    : sf::Color(0, 230, 245,
                                static_cast<uint8_t>(100 + 155 * t));

            sf::RectangleShape growingOverlay({curSize, curSize});
            growingOverlay.setOrigin({halfSize, halfSize});
            growingOverlay.setPosition(
                {px + TILE_SIZE * 0.5f, py + TILE_SIZE * 0.5f});
            growingOverlay.setFillColor(fillColor);
            growingOverlay.setOutlineThickness(isMud ? -1.0f : -1.5f);
            growingOverlay.setOutlineColor(borderColor);
            target.draw(growingOverlay);

            // 2. High-energy tactical flash edge expanding with the square
            if (t < 0.85f) {
              float flashT = 1.0f - (t / 0.85f);
              sf::Color flashFill =
                  isMud ? sf::Color(0, 200, 230,
                                    static_cast<uint8_t>(flashT * 25.0f))
                        : sf::Color(0, 235, 255,
                                    static_cast<uint8_t>(flashT * 90.0f));
              sf::Color flashBorder =
                  isMud ? sf::Color(140, 255, 250,
                                    static_cast<uint8_t>(flashT * 60.0f))
                        : sf::Color(140, 255, 250,
                                    static_cast<uint8_t>(flashT * 220.0f));

              sf::RectangleShape flashBox({curSize, curSize});
              flashBox.setOrigin({halfSize, halfSize});
              flashBox.setPosition(
                  {px + TILE_SIZE * 0.5f, py + TILE_SIZE * 0.5f});
              flashBox.setFillColor(flashFill);
              flashBox.setOutlineThickness(isMud ? 0.8f : 1.2f);
              flashBox.setOutlineColor(flashBorder);
              target.draw(flashBox);
            }
          } else {
            // Settled stepped-on cell
            sf::Color settledFill =
                isMud ? sf::Color(0, 180, 215, 35)
                      : // Delicate cyan tint for walked mud so sludge and
                        // bubbles stay visible
                    sf::Color(10, 48, 65, 220); // Deep dark cyan
            sf::Color settledBorder =
                isMud ? sf::Color(0, 220, 235, 65)
                      : // Soft semi-transparent cyan border for mud
                    sf::Color(
                        0, 230, 245,
                        230); // Bright light cyan border for standard tiles

            sf::RectangleShape walkedOverlay({TILE_SIZE, TILE_SIZE});
            walkedOverlay.setPosition({px, py});
            walkedOverlay.setFillColor(settledFill);
            walkedOverlay.setOutlineThickness(isMud ? -1.0f : -1.5f);
            walkedOverlay.setOutlineColor(settledBorder);
            target.draw(walkedOverlay);
          }
        }

        // Highlight cell if evaluated in search wave
        if (cell.isVisitedBySearch()) {
          sf::RectangleShape waveHighlight({TILE_SIZE, TILE_SIZE});
          waveHighlight.setPosition({px, py});
          waveHighlight.setFillColor(sf::Color(100, 149, 237, 75));
          target.draw(waveHighlight);
        }
      }
    }

    // Radar wave ping highlights across both the maze and outer margins (smooth
    // fade to outer edges) - Bounding Box Culling optimization
    if (scanRadius >= 0.0f) {
      float maxR = scanRadius + 14.0f;
      int minTileX = std::max(-SCAN_PAD_TILES, static_cast<int>(std::floor((scanOrigin.x - maxR) / TILE_SIZE)));
      int maxTileX = std::min(maze.getWidth() + SCAN_PAD_TILES - 1, static_cast<int>(std::ceil((scanOrigin.x + maxR) / TILE_SIZE)));
      int minTileY = std::max(-SCAN_PAD_TILES, static_cast<int>(std::floor((scanOrigin.y - maxR) / TILE_SIZE)));
      int maxTileY = std::min(maze.getHeight() + SCAN_PAD_TILES - 1, static_cast<int>(std::ceil((scanOrigin.y + maxR) / TILE_SIZE)));

      for (int y = minTileY; y <= maxTileY; ++y) {
        for (int x = minTileX; x <= maxTileX; ++x) {
          int lx = x + SCAN_PAD_TILES;
          int ly = y + SCAN_PAD_TILES;
          float dist = cachedDistances_[ly * totalW + lx];
          float waveDist = scanRadius - dist;
          constexpr float WAVE_WIDTH = 70.0f;
          if (waveDist >= -14.0f && waveDist < WAVE_WIDTH) {
            float edgeFade = 1.0f;
            bool isInside = (x >= 0 && x < maze.getWidth() && y >= 0 &&
                             y < maze.getHeight());
            if (!isInside) {
              float dX =
                  (x < 0) ? static_cast<float>(-x)
                          : (x >= maze.getWidth()
                                 ? static_cast<float>(x - maze.getWidth() + 1)
                                 : 0.0f);
              float dY =
                  (y < 0) ? static_cast<float>(-y)
                          : (y >= maze.getHeight()
                                 ? static_cast<float>(y - maze.getHeight() + 1)
                                 : 0.0f);
              float outDist = std::max(dX, dY);
              float fadeNorm = std::clamp(
                  1.0f - (outDist / static_cast<float>(SCAN_PAD_TILES)), 0.0f,
                  1.0f);
              edgeFade = fadeNorm * fadeNorm * (3.0f - 2.0f * fadeNorm);
            }

            if (edgeFade > 0.01f) {
              float t = (waveDist < 0.0f) ? ((waveDist + 14.0f) / 14.0f)
                                          : (1.0f - (waveDist / WAVE_WIDTH));
              float intensity = t * t * (3.0f - 2.0f * t) * edgeFade;

              uint8_t fillAlpha = static_cast<uint8_t>(intensity * 75.0f);
              uint8_t borderAlpha = static_cast<uint8_t>(intensity * 220.0f);

              if (fillAlpha > 0 || borderAlpha > 0) {
                float px = x * TILE_SIZE;
                float py = y * TILE_SIZE;
                float x2 = px + TILE_SIZE;
                float y2 = py + TILE_SIZE;
                sf::Color fillCol(0, 230, 255, fillAlpha);

                pingQuads[pingQuadCount + 0] = {{px, py}, fillCol};
                pingQuads[pingQuadCount + 1] = {{x2, py}, fillCol};
                pingQuads[pingQuadCount + 2] = {{x2, y2}, fillCol};
                pingQuads[pingQuadCount + 3] = {{px, py}, fillCol};
                pingQuads[pingQuadCount + 4] = {{x2, y2}, fillCol};
                pingQuads[pingQuadCount + 5] = {{px, y2}, fillCol};
                pingQuadCount += 6;

                sf::Color borderCol(0, 255, 255, borderAlpha);
                pingBorders[pingBorderCount + 0] = {{px, py}, borderCol};
                pingBorders[pingBorderCount + 1] = {{x2, py}, borderCol};
                pingBorders[pingBorderCount + 2] = {{x2, py}, borderCol};
                pingBorders[pingBorderCount + 3] = {{x2, y2}, borderCol};
                pingBorders[pingBorderCount + 4] = {{x2, y2}, borderCol};
                pingBorders[pingBorderCount + 5] = {{px, y2}, borderCol};
                pingBorders[pingBorderCount + 6] = {{px, y2}, borderCol};
                pingBorders[pingBorderCount + 7] = {{px, py}, borderCol};
                pingBorderCount += 8;
              }
            }
          }
        }
      }
    }

    // Draw batched high-energy radar ping waves in single draw calls
    if (pingQuadCount > 0) {
      pingQuads.resize(pingQuadCount);
      target.draw(pingQuads);
    }
    if (pingBorderCount > 0) {
      pingBorders.resize(pingBorderCount);
      target.draw(pingBorders);
    }

    // Dedicated grid overlay: rendered strictly inside walkable corridor and
    // room floor tiles of the maze - Zero-Allocation buffer
    if (showGrid) {
      auto &gridLines = gridLines_;
      size_t maxGridVerts = static_cast<size_t>(maze.getWidth() * maze.getHeight() * 8);
      if (gridLines.getVertexCount() < maxGridVerts) {
        gridLines.resize(maxGridVerts);
      }
      size_t gridVertCount = 0;
      sf::Color gridColor(60, 76, 96, 140);

      for (int y = 0; y < maze.getHeight(); ++y) {
        for (int x = 0; x < maze.getWidth(); ++x) {
          if (!maze.isPassable(x, y)) {
            continue;
          }

          int lx = x + SCAN_PAD_TILES;
          int ly = y + SCAN_PAD_TILES;
          if (scanRadius >= 0.0f) {
            if (cachedDistances_[ly * totalW + lx] > scanRadius) {
              continue;
            }
          }

          float px = x * TILE_SIZE;
          float py = y * TILE_SIZE;
          float pxNext = px + TILE_SIZE;
          float pyNext = py + TILE_SIZE;

          // Top horizontal edge
          gridLines[gridVertCount + 0] = {{px, py}, gridColor};
          gridLines[gridVertCount + 1] = {{pxNext, py}, gridColor};

          // Left vertical edge
          gridLines[gridVertCount + 2] = {{px, py}, gridColor};
          gridLines[gridVertCount + 3] = {{px, pyNext}, gridColor};
          gridVertCount += 4;

          // Right vertical edge if at maze border or neighbor tile is not passable
          if (x + 1 >= maze.getWidth() || !maze.isPassable(x + 1, y)) {
            gridLines[gridVertCount + 0] = {{pxNext, py}, gridColor};
            gridLines[gridVertCount + 1] = {{pxNext, pyNext}, gridColor};
            gridVertCount += 2;
          }

          // Bottom horizontal edge if at maze border or neighbor tile is not passable
          if (y + 1 >= maze.getHeight() || !maze.isPassable(x, y + 1)) {
            gridLines[gridVertCount + 0] = {{px, pyNext}, gridColor};
            gridLines[gridVertCount + 1] = {{pxNext, pyNext}, gridColor};
            gridVertCount += 2;
          }
        }
      }

      if (gridVertCount > 0) {
        gridLines.resize(gridVertCount);
        target.draw(gridLines);
      }
    }
  }

  void renderRoute(sf::RenderTarget &target,
                   const algorithms::routing::RoutePlan &plan,
                   float routeProgress = 1.0f) {
    if (plan.fullPath.size() < 2 || routeProgress <= 0.001f)
      return;

    // Calculate segment distances
    std::vector<float> segLens;
    segLens.reserve(plan.fullPath.size() - 1);
    float totalDist = 0.0f;
    for (size_t i = 0; i + 1 < plan.fullPath.size(); ++i) {
      float x1 = plan.fullPath[i].x * TILE_SIZE + TILE_SIZE * 0.5f;
      float y1 = plan.fullPath[i].y * TILE_SIZE + TILE_SIZE * 0.5f;
      float x2 = plan.fullPath[i + 1].x * TILE_SIZE + TILE_SIZE * 0.5f;
      float y2 = plan.fullPath[i + 1].y * TILE_SIZE + TILE_SIZE * 0.5f;
      float d = std::hypot(x2 - x1, y2 - y1);
      segLens.push_back(d);
      totalDist += d;
    }
    if (totalDist <= 0.001f)
      return;

    float targetDist = totalDist * std::clamp(routeProgress, 0.0f, 1.0f);
    float accDist = 0.0f;

    std::vector<sf::Vector2f> activePts;
    activePts.reserve(plan.fullPath.size());
    activePts.push_back({plan.fullPath[0].x * TILE_SIZE + TILE_SIZE * 0.5f,
                         plan.fullPath[0].y * TILE_SIZE + TILE_SIZE * 0.5f});

    sf::Vector2f headPoint = activePts.front();
    for (size_t i = 0; i < segLens.size(); ++i) {
      float p1x = plan.fullPath[i].x * TILE_SIZE + TILE_SIZE * 0.5f;
      float p1y = plan.fullPath[i].y * TILE_SIZE + TILE_SIZE * 0.5f;
      float p2x = plan.fullPath[i + 1].x * TILE_SIZE + TILE_SIZE * 0.5f;
      float p2y = plan.fullPath[i + 1].y * TILE_SIZE + TILE_SIZE * 0.5f;

      if (accDist + segLens[i] <= targetDist) {
        activePts.push_back({p2x, p2y});
        accDist += segLens[i];
        headPoint = {p2x, p2y};
      } else {
        float remain = targetDist - accDist;
        float frac = (segLens[i] > 0.001f) ? (remain / segLens[i]) : 0.0f;
        headPoint = {p1x + (p2x - p1x) * frac, p1y + (p2y - p1y) * frac};
        activePts.push_back(headPoint);
        break;
      }
    }

            if (activePts.size() < 2) return;

        // Filter out any consecutive duplicate points
        std::vector<sf::Vector2f> cleanPts;
        cleanPts.reserve(activePts.size());
        for (const auto& p : activePts) {
            if (cleanPts.empty() || std::hypot(p.x - cleanPts.back().x, p.y - cleanPts.back().y) > 0.5f) {
                cleanPts.push_back(p);
            }
        }
        if (cleanPts.size() >= 2 && std::hypot(cleanPts.back().x - activePts.back().x,
                                              cleanPts.back().y - activePts.back().y) > 0.001f) {
            cleanPts.push_back(activePts.back());
        }

        // Merge strictly consecutive straight collinear points (dot > 0.999f) without skipping dead ends or reversals!
        std::vector<sf::Vector2f> waypoints;
        waypoints.reserve(cleanPts.size());
        if (cleanPts.size() >= 2) {
            waypoints.push_back(cleanPts[0]);
            for (size_t i = 1; i + 1 < cleanPts.size(); ++i) {
                sf::Vector2f dIn = cleanPts[i] - cleanPts[i - 1];
                sf::Vector2f dOut = cleanPts[i + 1] - cleanPts[i];
                float lenIn = std::hypot(dIn.x, dIn.y);
                float lenOut = std::hypot(dOut.x, dOut.y);
                if (lenIn < 0.001f || lenOut < 0.001f) continue;
                dIn /= lenIn;
                dOut /= lenOut;

                float dot = dIn.x * dOut.x + dIn.y * dOut.y;
                float cross = dIn.x * dOut.y - dIn.y * dOut.x;

                // Only skip cleanPts[i] if it is continuing in the EXACT SAME forward direction:
                if (dot > 0.999f && std::abs(cross) < 0.01f) {
                    continue; // Pure straight collinear continuation
                }

                // If dot <= 0.999f (turn left, turn right, or turn 180 degrees backward):
                // cleanPts[i] IS A REAL TURN OR DEAD END / SCRAP! Keep it!
                waypoints.push_back(cleanPts[i]);
            }
            waypoints.push_back(cleanPts.back());
        }

        if (waypoints.size() < 2) return;

        // Draw seamless constant-width polyline with smooth circular outer fillets,
        // miter inner corners, and round end-caps (strictly uniform width, zero overlap artifacts)
        auto drawSeamlessPolyline = [&](const std::vector<sf::Vector2f>& pts,
                                        float thickness, const sf::Color& color) {
            if (pts.size() < 2) return;
            float W = thickness * 0.5f;

            sf::VertexArray va(sf::PrimitiveType::Triangles);

            auto addTri = [&](sf::Vector2f pA, sf::Vector2f pB, sf::Vector2f pC) {
                va.append({pA, color});
                va.append({pB, color});
                va.append({pC, color});
            };

            auto addQuad = [&](sf::Vector2f pA, sf::Vector2f pB, sf::Vector2f pC, sf::Vector2f pD) {
                addTri(pA, pB, pC);
                addTri(pA, pC, pD);
            };

            size_t numSegs = pts.size() - 1;
            std::vector<sf::Vector2f> dirs(numSegs);
            std::vector<sf::Vector2f> unitNorms(numSegs);
            std::vector<sf::Vector2f> norms(numSegs);
            std::vector<float> lens(numSegs);

            for (size_t i = 0; i < numSegs; ++i) {
                sf::Vector2f d = pts[i + 1] - pts[i];
                float len = std::hypot(d.x, d.y);
                lens[i] = len;
                if (len < 0.0001f) {
                    dirs[i] = {1.0f, 0.0f};
                } else {
                    dirs[i] = d / len;
                }
                unitNorms[i] = {-dirs[i].y, dirs[i].x};
                norms[i] = unitNorms[i] * W;
            }

            // Semicircular rounded cap at terminal endpoint without segment overlap
            auto addRoundCap = [&](sf::Vector2f center, sf::Vector2f forward, sf::Vector2f norm) {
                constexpr int CAP_STEPS = 8;
                sf::Vector2f prev = norm;
                for (int k = 1; k <= CAP_STEPS; ++k) {
                    float t = static_cast<float>(k) / CAP_STEPS;
                    sf::Vector2f cur = (1.0f - t) * norm + t * forward;
                    float len = std::hypot(cur.x, cur.y);
                    if (len > 0.0001f) cur = (cur / len) * W;
                    addTri(center, center + prev, center + cur);
                    prev = cur;
                }
                for (int k = 1; k <= CAP_STEPS; ++k) {
                    float t = static_cast<float>(k) / CAP_STEPS;
                    sf::Vector2f cur = (1.0f - t) * forward - t * norm;
                    float len = std::hypot(cur.x, cur.y);
                    if (len > 0.0001f) cur = (cur / len) * W;
                    addTri(center, center + prev, center + cur);
                    prev = cur;
                }
            };

            // 1. Terminal start round cap at pts[0]
            addRoundCap(pts[0], -dirs[0] * W, norms[0]);

            // Precompute corner cut parameters for every joint between segments
            struct CornerCut {
                bool hasTurn{false};
                bool isReversal{false};
                float side{0.0f}; // > 0 left turn, < 0 right turn
                sf::Vector2f cInner{};
                sf::Vector2f vInOuter{};  // unit vector from joint to outer incoming edge
                sf::Vector2f vOutOuter{}; // unit vector from joint to outer outgoing edge
            };

            std::vector<CornerCut> corners(numSegs > 1 ? (numSegs - 1) : 0);
            for (size_t i = 0; i + 1 < numSegs; ++i) {
                float cross = dirs[i].x * dirs[i + 1].y - dirs[i].y * dirs[i + 1].x;
                float dot = dirs[i].x * dirs[i + 1].x + dirs[i].y * dirs[i + 1].y;

                if (dot > 0.999f && std::abs(cross) < 0.01f) {
                    corners[i].hasTurn = false;
                    corners[i].isReversal = false;
                    continue;
                }

                if (dot < -0.85f) {
                    // 180 degree U-turn / reversal at dead-end target
                    corners[i].hasTurn = false;
                    corners[i].isReversal = true;
                    continue;
                }

                corners[i].hasTurn = true;
                corners[i].isReversal = false;
                float s = (cross > 0.0f) ? 1.0f : -1.0f;
                corners[i].side = s;

                // Miter bisector vector
                sf::Vector2f bisector = unitNorms[i] + unitNorms[i + 1];
                float bLen = std::hypot(bisector.x, bisector.y);
                if (bLen > 0.0001f) bisector /= bLen;

                float denom = 1.0f + (unitNorms[i].x * unitNorms[i + 1].x + unitNorms[i].y * unitNorms[i + 1].y);
                float miterLen = (denom > 0.001f) ? (W / std::sqrt(denom * 0.5f)) : (2.5f * W);
                // Clamp miter to avoid spikes on sharp angles or short segments
                float maxLen = std::min(lens[i], lens[i + 1]) * 0.45f;
                miterLen = std::clamp(miterLen, 0.5f * W, std::min(2.5f * W, maxLen));

                corners[i].cInner = pts[i + 1] + bisector * (s * miterLen);
                corners[i].vInOuter = -unitNorms[i] * s;
                corners[i].vOutOuter = -unitNorms[i + 1] * s;
            }

            // Draw every segment with strict, uniform constant width 2 * W
            for (size_t i = 0; i < numSegs; ++i) {
                sf::Vector2f startLeft, startRight;
                sf::Vector2f endLeft, endRight;

                // Start vertices for segment i:
                if (i == 0 || !corners[i - 1].hasTurn) {
                    startLeft = pts[i] + norms[i];
                    startRight = pts[i] - norms[i];
                } else {
                    const auto& c = corners[i - 1];
                    if (c.side > 0.0f) {
                        // Left turn at pts[i]: Left is inner miter, Right is outer cut
                        startLeft = c.cInner;
                        startRight = pts[i] - norms[i];
                    } else {
                        // Right turn at pts[i]: Right is inner miter, Left is outer cut
                        startLeft = pts[i] + norms[i];
                        startRight = c.cInner;
                    }
                }

                // End vertices for segment i:
                if (i + 1 == numSegs || !corners[i].hasTurn) {
                    endLeft = pts[i + 1] + norms[i];
                    endRight = pts[i + 1] - norms[i];
                } else {
                    const auto& c = corners[i];
                    if (c.side > 0.0f) {
                        // Left turn at pts[i+1]: Left is inner miter, Right is outer cut
                        endLeft = c.cInner;
                        endRight = pts[i + 1] - norms[i];
                    } else {
                        // Right turn at pts[i+1]: Right is inner miter, Left is outer cut
                        endLeft = pts[i + 1] + norms[i];
                        endRight = c.cInner;
                    }
                }

                // Two non-overlapping halves meeting at the centerline:
                // Guarantees constant width 2 * W everywhere along the segment!
                addQuad(startLeft, endLeft, pts[i + 1], pts[i]);
                addQuad(pts[i], pts[i + 1], endRight, startRight);
            }

            // Draw smooth non-overlapping circular fillet on the outer corner of each turn,
            // and round cap on dead-end reversals!
            for (size_t i = 0; i + 1 < numSegs; ++i) {
                if (corners[i].isReversal) {
                    // Turnaround at dead-end target
                    addRoundCap(pts[i + 1], dirs[i] * W, norms[i]);
                    continue;
                }

                if (!corners[i].hasTurn) continue;

                const auto& c = corners[i];
                constexpr int ARC_STEPS = 8;
                sf::Vector2f prevPt = pts[i + 1] + c.vInOuter * W;
                for (int k = 1; k <= ARC_STEPS; ++k) {
                    float t = static_cast<float>(k) / ARC_STEPS;
                    sf::Vector2f v = (1.0f - t) * c.vInOuter + t * c.vOutOuter;
                    float vLen = std::hypot(v.x, v.y);
                    sf::Vector2f curPt = pts[i + 1] + (vLen > 0.0001f ? (v / vLen) * W : c.vOutOuter * W);
                    if (c.side > 0.0f) {
                        addTri(pts[i + 1], prevPt, curPt);
                    } else {
                        addTri(pts[i + 1], curPt, prevPt);
                    }
                    prevPt = curPt;
                }
            }

            // Terminal end round cap at pts.back()
            addRoundCap(pts.back(), dirs.back() * W, norms.back());

            target.draw(va);
        };

    // Multi-layered cyber-neon route conduit
    drawSeamlessPolyline(waypoints, 12.0f,
                         sf::Color(0, 210, 245, 22)); // Ambient soft glow
    drawSeamlessPolyline(waypoints, 7.0f,
                         sf::Color(0, 230, 250, 75)); // Radiant neon body
    drawSeamlessPolyline(waypoints, 3.5f,
                         sf::Color(220, 255, 255, 245)); // Intense laser core

    // High-energy pulsing beacon head at the leading tip of the tracing line
    if (routeProgress < 0.999f) {
      float pulse = 0.5f + 0.5f * std::sin(animTime_ * 12.0f);
      float headR = 5.5f + pulse * 3.5f;
      sf::CircleShape headCircle(headR);
      headCircle.setOrigin({headR, headR});
      headCircle.setPosition(headPoint);
      headCircle.setFillColor(sf::Color(0, 255, 245, 240));
      headCircle.setOutlineThickness(1.8f);
      headCircle.setOutlineColor(sf::Color(255, 255, 255, 245));
      target.draw(headCircle);
    } else {
      // Path reached destination! Trigger 1-second entrance/exit beacon
      // animation
      if (!wasRouteCompleted_) {
        wasRouteCompleted_ = true;
        routeCompletionTimer_ = 1.0f;
      }
    }
    if (routeProgress < 0.1f) {
      wasRouteCompleted_ = false;
    }
  }

  void renderExitOverlay(sf::RenderTarget &target, const map::Maze &maze,
                         bool isExitDisabled = false, float scanRadius = -1.0f,
                         sf::Vector2f scanOrigin = {0.0f, 0.0f}) {
    core::Point exitPt = maze.getExitPoint();
    float px = exitPt.x * TILE_SIZE;
    float py = exitPt.y * TILE_SIZE;
    sf::Vector2f center{px + TILE_SIZE * 0.5f, py + TILE_SIZE * 0.5f};

    if (scanRadius >= 0.0f) {
      float dist = std::hypot(center.x - scanOrigin.x, center.y - scanOrigin.y);
      if (dist > scanRadius + 14.0f) {
        return; // Not reached by sonar scan yet
      }
    }

    if (isExitDisabled) {
      // Dark circular backing plate to isolate cross from route line
      float plateR = 14.0f;
      sf::CircleShape plate(plateR);
      plate.setOrigin({plateR, plateR});
      plate.setPosition(center);
      plate.setFillColor(sf::Color(24, 28, 36, 240));
      plate.setOutlineThickness(1.5f);
      plate.setOutlineColor(sf::Color(90, 95, 105, 230));
      target.draw(plate);

      float crossLen = 22.0f;
      float barThick = 4.2f;

      // Shadow outline
      for (float rot : {45.0f, -45.0f}) {
        sf::RectangleShape sBar({crossLen + 2.0f, barThick + 2.0f});
        sBar.setOrigin({(crossLen + 2.0f) * 0.5f, (barThick + 2.0f) * 0.5f});
        sBar.setPosition(center);
        sBar.setRotation(sf::degrees(rot));
        sBar.setFillColor(sf::Color(15, 15, 20, 240));
        target.draw(sBar);
      }

      // Vibrant red cross
      for (float rot : {45.0f, -45.0f}) {
        sf::RectangleShape rBar({crossLen, barThick});
        rBar.setOrigin({crossLen * 0.5f, barThick * 0.5f});
        rBar.setPosition(center);
        rBar.setRotation(sf::degrees(rot));
      }
    }
  }

  void renderDisabledExitOverlay(sf::RenderTarget &target,
                                 const map::Maze &maze,
                                 bool isExitDisabled = false,
                                 float scanRadius = -1.0f,
                                 sf::Vector2f scanOrigin = {0.0f, 0.0f}) {
    renderExitOverlay(target, maze, isExitDisabled, scanRadius, scanOrigin);
  }

  void renderEntryExitArrows(sf::RenderTarget &target, const map::Maze &maze,
                             const algorithms::routing::RoutePlan &plan,
                             bool isExitDisabled = false,
                             float scanRadius = -1.0f,
                             sf::Vector2f scanOrigin = {0.0f, 0.0f}) {
    struct ArrowTheme {
      sf::Color tipColor;
      sf::Color bodyColor;
      sf::Color tailColor;
      sf::Color outlineColor;
      sf::Color glowColor;
      sf::Color plateOutline;
    };

    const ArrowTheme entranceTheme{
        sf::Color(255, 252, 215, 255), // Radiant warm white-gold tip
        sf::Color(255, 190, 30, 255),  // Luminous golden amber body
        sf::Color(215, 125, 15, 255),  // Deep amber bronze tail
        sf::Color(255, 235, 130, 240), // Crisp glowing gold outline
        sf::Color(255, 175, 20, 65),   // Soft neon aura
        sf::Color(220, 155, 35, 230)   // Tactical plate outline
    };

    const ArrowTheme exitTheme{
        sf::Color(225, 255, 240, 255), // Radiant icy mint tip
        sf::Color(45, 235, 125, 255),  // Vivid cyber emerald body
        sf::Color(20, 160, 75, 255),   // Deep neon pine tail
        sf::Color(185, 255, 225, 240), // Crisp glowing mint outline
        sf::Color(40, 225, 115, 65),   // Soft neon aura
        sf::Color(45, 210, 110, 230)   // Tactical plate outline
    };

    auto drawTacticalArrow = [&](sf::Vector2f center, sf::Vector2f dirF,
                                 const ArrowTheme &theme) {
      float normLen = std::hypot(dirF.x, dirF.y);
      if (normLen < 0.001f)
        return;
      dirF.x /= normLen;
      dirF.y /= normLen;
      sf::Vector2f normF{-dirF.y, dirF.x};

      // Subtle pulsing bob along direction of motion
      float bob = std::sin(animTime_ * 4.5f) * 1.8f;
      sf::Vector2f c = center + dirF * bob;

      // 1. Tactical HUD circular backing plate
      float plateR = 14.5f;
      sf::CircleShape plate(plateR);
      plate.setOrigin({plateR, plateR});
      plate.setPosition(center);
      plate.setFillColor(sf::Color(14, 18, 24, 240));
      plate.setOutlineThickness(1.5f);
      plate.setOutlineColor(theme.plateOutline);
      target.draw(plate);

      // Inner HUD guide ring
      float innerR = 11.2f;
      sf::CircleShape innerRing(innerR);
      innerRing.setOrigin({innerR, innerR});
      innerRing.setPosition(center);
      innerRing.setFillColor(sf::Color::Transparent);
      innerRing.setOutlineThickness(1.0f);
      innerRing.setOutlineColor(sf::Color(theme.plateOutline.r,
                                          theme.plateOutline.g,
                                          theme.plateOutline.b, 55));
      target.draw(innerRing);

      // 4 cardinal tactical compass ticks on the plate rim
      for (int i = 0; i < 4; ++i) {
        float angle = i * 1.5707963f;
        sf::Vector2f radial{std::cos(angle), std::sin(angle)};
        sf::Vector2f t1 = center + radial * (plateR - 2.8f);
        sf::Vector2f t2 = center + radial * (plateR + 1.2f);
        sf::VertexArray tick(sf::PrimitiveType::Lines);
        tick.resize(2);
        tick[0] = {t1, sf::Color(theme.plateOutline.r, theme.plateOutline.g,
                                 theme.plateOutline.b, 160)};
        tick[1] = {t2, sf::Color(theme.plateOutline.r, theme.plateOutline.g,
                                 theme.plateOutline.b, 160)};
        target.draw(tick);
      }

      // 2. Soft neon glow aura behind the arrow
      {
        float glowScale = 1.25f;
        sf::Vector2f g0 = c + dirF * (9.5f * glowScale);
        sf::Vector2f g1 =
            c + dirF * (1.2f * glowScale) + normF * (6.5f * glowScale);
        sf::Vector2f g2 =
            c + dirF * (2.4f * glowScale) + normF * (2.6f * glowScale);
        sf::Vector2f g3 =
            c - dirF * (7.2f * glowScale) + normF * (2.6f * glowScale);
        sf::Vector2f g4 = c - dirF * (5.2f * glowScale);
        sf::Vector2f g5 =
            c - dirF * (7.2f * glowScale) - normF * (2.6f * glowScale);
        sf::Vector2f g6 =
            c + dirF * (2.4f * glowScale) - normF * (2.6f * glowScale);
        sf::Vector2f g7 =
            c + dirF * (1.2f * glowScale) - normF * (6.5f * glowScale);
        sf::Vector2f gCenter = c + dirF * (2.4f * glowScale);

        sf::VertexArray glowMesh(sf::PrimitiveType::Triangles);
        glowMesh.resize(24);
        sf::Color gCol = theme.glowColor;

        auto addGlowTri = [&](int idx, sf::Vector2f a, sf::Vector2f b,
                              sf::Vector2f d) {
          glowMesh[idx] = {a, gCol};
          glowMesh[idx + 1] = {b, gCol};
          glowMesh[idx + 2] = {d, gCol};
        };

        addGlowTri(0, g0, g1, gCenter);
        addGlowTri(3, g0, gCenter, g7);
        addGlowTri(6, g1, g2, gCenter);
        addGlowTri(9, g7, gCenter, g6);
        addGlowTri(12, gCenter, g2, g3);
        addGlowTri(15, gCenter, g3, g4);
        addGlowTri(18, gCenter, g4, g5);
        addGlowTri(21, gCenter, g5, g6);
        target.draw(glowMesh);
      }

      // 3. Core arrow shape: 8-point aerodynamic chevron polygon
      sf::Vector2f p0 = c + dirF * 9.5f;                // Nose tip
      sf::Vector2f p1 = c + dirF * 1.2f + normF * 6.5f; // Right wingtip
      sf::Vector2f p2 =
          c + dirF * 2.4f + normF * 2.6f; // Right inner barb notch
      sf::Vector2f p3 = c - dirF * 7.2f + normF * 2.6f; // Right tail
      sf::Vector2f p4 = c - dirF * 5.2f;                // Tail indented notch
      sf::Vector2f p5 = c - dirF * 7.2f - normF * 2.6f; // Left tail
      sf::Vector2f p6 = c + dirF * 2.4f - normF * 2.6f; // Left inner barb notch
      sf::Vector2f p7 = c + dirF * 1.2f - normF * 6.5f; // Left wingtip
      sf::Vector2f pCenter = c + dirF * 2.4f;           // Central spine point

      sf::VertexArray mesh(sf::PrimitiveType::Triangles);
      mesh.resize(24);

      auto addTri = [&](int idx, sf::Vector2f a, sf::Color ca, sf::Vector2f b,
                        sf::Color cb, sf::Vector2f d, sf::Color cd) {
        mesh[idx] = {a, ca};
        mesh[idx + 1] = {b, cb};
        mesh[idx + 2] = {d, cd};
      };

      addTri(0, p0, theme.tipColor, p1, theme.bodyColor, pCenter,
             theme.bodyColor);
      addTri(3, p0, theme.tipColor, pCenter, theme.bodyColor, p7,
             theme.bodyColor);
      addTri(6, p1, theme.bodyColor, p2, theme.bodyColor, pCenter,
             theme.bodyColor);
      addTri(9, p7, theme.bodyColor, pCenter, theme.bodyColor, p6,
             theme.bodyColor);
      addTri(12, pCenter, theme.bodyColor, p2, theme.bodyColor, p3,
             theme.tailColor);
      addTri(15, pCenter, theme.bodyColor, p3, theme.tailColor, p4,
             theme.tailColor);
      addTri(18, pCenter, theme.bodyColor, p4, theme.tailColor, p5,
             theme.tailColor);
      addTri(21, pCenter, theme.bodyColor, p5, theme.tailColor, p6,
             theme.bodyColor);
      target.draw(mesh);

      // 4. Crisp neon perimeter outline
      sf::VertexArray outline(sf::PrimitiveType::LineStrip);
      outline.resize(9);
      outline[0] = {p0, theme.outlineColor};
      outline[1] = {p1, theme.outlineColor};
      outline[2] = {p2, theme.outlineColor};
      outline[3] = {p3, theme.outlineColor};
      outline[4] = {p4, theme.outlineColor};
      outline[5] = {p5, theme.outlineColor};
      outline[6] = {p6, theme.outlineColor};
      outline[7] = {p7, theme.outlineColor};
      outline[8] = {p0, theme.outlineColor};
      target.draw(outline);

      // Central spine ridge highlight line for high-tech 3D bevel effect
      sf::VertexArray spine(sf::PrimitiveType::Lines);
      spine.resize(2);
      spine[0] = {p0, sf::Color(255, 255, 255, 230)};
      spine[1] = {p4, sf::Color(theme.outlineColor.r, theme.outlineColor.g,
                                theme.outlineColor.b, 60)};
      target.draw(spine);

      // 5. Leading dynamic energy pulse / micro-chevron
      float pulseT = std::fmod(animTime_ * 1.8f, 1.0f);
      float pulseDist = 10.5f + pulseT * 3.8f;
      float pulseAlpha = (1.0f - pulseT) * 210.0f;
      sf::Vector2f mTip = c + dirF * (pulseDist + 1.8f);
      sf::Vector2f mR = c + dirF * pulseDist + normF * 3.4f;
      sf::Vector2f mL = c + dirF * pulseDist - normF * 3.4f;

      sf::Color pulseCol(
          theme.outlineColor.r, theme.outlineColor.g, theme.outlineColor.b,
          static_cast<uint8_t>(std::clamp(pulseAlpha, 0.0f, 255.0f)));
      sf::VertexArray chevron(sf::PrimitiveType::LineStrip);
      chevron.resize(3);
      chevron[0] = {mL, pulseCol};
      chevron[1] = {mTip, pulseCol};
      chevron[2] = {mR, pulseCol};
      target.draw(chevron);
    };

    // --- Entrance Arrow (one cell before entrance pointing inward) ---
    core::Point startPt = maze.getStartPoint();
    sf::Vector2i inDir{0, 0};
    if (plan.routeValid && plan.fullPath.size() >= 2) {
      core::Point p1 = plan.fullPath[1];
      inDir = {std::clamp(p1.x - startPt.x, -1, 1),
               std::clamp(p1.y - startPt.y, -1, 1)};
    }
    if (inDir.x == 0 && inDir.y == 0) {
      for (auto [dx, dy] : {std::pair{1, 0}, {-1, 0}, {0, 1}, {0, -1}}) {
        int nx = startPt.x + dx;
        int ny = startPt.y + dy;
        if (maze.isValid(nx, ny) && maze.at(nx, ny).isPassable()) {
          inDir = {dx, dy};
          break;
        }
      }
    }
    if (inDir.x == 0 && inDir.y == 0) {
      int distL = startPt.x;
      int distR = (maze.getWidth() - 1) - startPt.x;
      int distT = startPt.y;
      int distB = (maze.getHeight() - 1) - startPt.y;
      int minDist = std::min({distL, distR, distT, distB});
      if (minDist == distL)
        inDir = {1, 0};
      else if (minDist == distR)
        inDir = {-1, 0};
      else if (minDist == distT)
        inDir = {0, 1};
      else
        inDir = {0, -1};
    }

    sf::Vector2i cellBeforeStart{startPt.x - inDir.x, startPt.y - inDir.y};
    sf::Vector2f startArrowCenter{(cellBeforeStart.x + 0.5f) * TILE_SIZE,
                                  (cellBeforeStart.y + 0.5f) * TILE_SIZE};
    sf::Vector2f inDirF{static_cast<float>(inDir.x),
                        static_cast<float>(inDir.y)};
    float normIn = std::hypot(inDirF.x, inDirF.y);
    if (normIn > 0.001f) {
      inDirF.x /= normIn;
      inDirF.y /= normIn;
    }
    sf::Vector2f inNormF{-inDirF.y, inDirF.x};

    bool drawStartArrow = true;
    if (scanRadius >= 0.0f) {
      float dist = std::hypot(startArrowCenter.x - scanOrigin.x,
                              startArrowCenter.y - scanOrigin.y);
      if (dist > scanRadius + 14.0f) {
        drawStartArrow = false;
      }
    }

    if (drawStartArrow) {
      if (isExitDisabled) {
        // Dual two-way arrows side by side (like mobile internet data transfer
        // icons ⇅): Inward & Outward
        float plateR = 15.0f;
        sf::CircleShape plate(plateR);
        plate.setOrigin({plateR, plateR});
        plate.setPosition(startArrowCenter);
        plate.setFillColor(sf::Color(14, 18, 24, 240));
        plate.setOutlineThickness(1.5f);
        plate.setOutlineColor(sf::Color(215, 165, 45, 230));
        target.draw(plate);

        // Inner guide ring
        float innerR = 11.5f;
        sf::CircleShape innerRing(innerR);
        innerRing.setOrigin({innerR, innerR});
        innerRing.setPosition(startArrowCenter);
        innerRing.setFillColor(sf::Color::Transparent);
        innerRing.setOutlineThickness(1.0f);
        innerRing.setOutlineColor(sf::Color(215, 165, 45, 55));
        target.draw(innerRing);

        // 4 cardinal ticks
        for (int i = 0; i < 4; ++i) {
          float angle = i * 1.5707963f;
          sf::Vector2f radial{std::cos(angle), std::sin(angle)};
          sf::Vector2f t1 = startArrowCenter + radial * (plateR - 2.8f);
          sf::Vector2f t2 = startArrowCenter + radial * (plateR + 1.2f);
          sf::VertexArray tick(sf::PrimitiveType::Lines);
          tick.resize(2);
          tick[0] = {t1, sf::Color(215, 165, 45, 160)};
          tick[1] = {t2, sf::Color(215, 165, 45, 160)};
          target.draw(tick);
        }

        auto drawCompactArrow = [&](sf::Vector2f aCenter, sf::Vector2f aDirF,
                                    float bobPhase, const ArrowTheme &theme) {
          float normLen = std::hypot(aDirF.x, aDirF.y);
          if (normLen < 0.001f)
            return;
          aDirF.x /= normLen;
          aDirF.y /= normLen;
          sf::Vector2f aNormF{-aDirF.y, aDirF.x};

          float bob = std::sin(animTime_ * 5.0f + bobPhase) * 1.4f;
          sf::Vector2f c = aCenter + aDirF * bob;

          float s = 0.65f; // Scale factor for compact side-by-side arrows

          sf::Vector2f p0 = c + aDirF * (9.5f * s);
          sf::Vector2f p1 = c + aDirF * (1.2f * s) + aNormF * (6.5f * s);
          sf::Vector2f p2 = c + aDirF * (2.4f * s) + aNormF * (2.6f * s);
          sf::Vector2f p3 = c - aDirF * (7.2f * s) + aNormF * (2.6f * s);
          sf::Vector2f p4 = c - aDirF * (5.2f * s);
          sf::Vector2f p5 = c - aDirF * (7.2f * s) - aNormF * (2.6f * s);
          sf::Vector2f p6 = c + aDirF * (2.4f * s) - aNormF * (2.6f * s);
          sf::Vector2f p7 = c + aDirF * (1.2f * s) - aNormF * (6.5f * s);
          sf::Vector2f pCenter = c + aDirF * (2.4f * s);

          sf::VertexArray mesh(sf::PrimitiveType::Triangles);
          mesh.resize(24);

          auto addTri = [&](int idx, sf::Vector2f a, sf::Color ca,
                            sf::Vector2f b, sf::Color cb, sf::Vector2f d,
                            sf::Color cd) {
            mesh[idx] = {a, ca};
            mesh[idx + 1] = {b, cb};
            mesh[idx + 2] = {d, cd};
          };

          addTri(0, p0, theme.tipColor, p1, theme.bodyColor, pCenter,
                 theme.bodyColor);
          addTri(3, p0, theme.tipColor, pCenter, theme.bodyColor, p7,
                 theme.bodyColor);
          addTri(6, p1, theme.bodyColor, p2, theme.bodyColor, pCenter,
                 theme.bodyColor);
          addTri(9, p7, theme.bodyColor, pCenter, theme.bodyColor, p6,
                 theme.bodyColor);
          addTri(12, pCenter, theme.bodyColor, p2, theme.bodyColor, p3,
                 theme.tailColor);
          addTri(15, pCenter, theme.bodyColor, p3, theme.tailColor, p4,
                 theme.tailColor);
          addTri(18, pCenter, theme.bodyColor, p4, theme.tailColor, p5,
                 theme.tailColor);
          addTri(21, pCenter, theme.bodyColor, p5, theme.tailColor, p6,
                 theme.bodyColor);
          target.draw(mesh);

          sf::VertexArray outline(sf::PrimitiveType::LineStrip);
          outline.resize(9);
          outline[0] = {p0, theme.outlineColor};
          outline[1] = {p1, theme.outlineColor};
          outline[2] = {p2, theme.outlineColor};
          outline[3] = {p3, theme.outlineColor};
          outline[4] = {p4, theme.outlineColor};
          outline[5] = {p5, theme.outlineColor};
          outline[6] = {p6, theme.outlineColor};
          outline[7] = {p7, theme.outlineColor};
          outline[8] = {p0, theme.outlineColor};
          target.draw(outline);

          // Central spine ridge
          sf::VertexArray spine(sf::PrimitiveType::Lines);
          spine.resize(2);
          spine[0] = {p0, sf::Color(255, 255, 255, 210)};
          spine[1] = {p4, sf::Color(theme.outlineColor.r, theme.outlineColor.g,
                                    theme.outlineColor.b, 60)};
          target.draw(spine);

          // Micro-chevron pulse
          float pulseT = std::fmod(animTime_ * 2.0f + bobPhase * 0.3f, 1.0f);
          float pulseDist = (10.0f + pulseT * 3.2f) * s;
          float pulseAlpha = (1.0f - pulseT) * 190.0f;
          sf::Vector2f mTip = c + aDirF * (pulseDist + 1.4f * s);
          sf::Vector2f mR = c + aDirF * pulseDist + aNormF * (2.8f * s);
          sf::Vector2f mL = c + aDirF * pulseDist - aNormF * (2.8f * s);

          sf::Color pulseCol(
              theme.outlineColor.r, theme.outlineColor.g, theme.outlineColor.b,
              static_cast<uint8_t>(std::clamp(pulseAlpha, 0.0f, 255.0f)));
          sf::VertexArray chevron(sf::PrimitiveType::LineStrip);
          chevron.resize(3);
          chevron[0] = {mL, pulseCol};
          chevron[1] = {mTip, pulseCol};
          chevron[2] = {mR, pulseCol};
          target.draw(chevron);
        };

        float sideOffset = 5.0f;
        // Arrow 1: Forward / Inward into the maze (Amber)
        drawCompactArrow(startArrowCenter - inNormF * sideOffset, inDirF, 0.0f,
                         entranceTheme);
        // Arrow 2: Backward / Outward back to base (Emerald)
        drawCompactArrow(startArrowCenter + inNormF * sideOffset, -inDirF,
                         3.14159f, exitTheme);
      } else {
        // Standard single forward entry arrow
        drawTacticalArrow(startArrowCenter, inDirF, entranceTheme);
      }
    }

    // --- Exit Arrow (one cell after exit pointing outward) ---
    if (!isExitDisabled) {
      core::Point exitPt = maze.getExitPoint();
      sf::Vector2i outDir{0, 0};
      if (plan.routeValid && plan.fullPath.size() >= 2 &&
          plan.fullPath.back() == exitPt) {
        core::Point pPrev = plan.fullPath[plan.fullPath.size() - 2];
        outDir = {std::clamp(exitPt.x - pPrev.x, -1, 1),
                  std::clamp(exitPt.y - pPrev.y, -1, 1)};
      }
      if (outDir.x == 0 && outDir.y == 0) {
        for (auto [dx, dy] : {std::pair{1, 0}, {-1, 0}, {0, 1}, {0, -1}}) {
          int nx = exitPt.x - dx;
          int ny = exitPt.y - dy;
          if (maze.isValid(nx, ny) && maze.at(nx, ny).isPassable()) {
            outDir = {dx, dy};
            break;
          }
        }
      }
      if (outDir.x == 0 && outDir.y == 0) {
        int distL = exitPt.x;
        int distR = (maze.getWidth() - 1) - exitPt.x;
        int distT = exitPt.y;
        int distB = (maze.getHeight() - 1) - exitPt.y;
        int minDist = std::min({distL, distR, distT, distB});
        if (minDist == distL)
          outDir = {-1, 0};
        else if (minDist == distR)
          outDir = {1, 0};
        else if (minDist == distT)
          outDir = {0, -1};
        else
          outDir = {0, 1};
      }

      sf::Vector2i cellAfterExit{exitPt.x + outDir.x, exitPt.y + outDir.y};
      sf::Vector2f exitArrowCenter{(cellAfterExit.x + 0.5f) * TILE_SIZE,
                                   (cellAfterExit.y + 0.5f) * TILE_SIZE};
      bool drawExitArrow = true;
      if (scanRadius >= 0.0f) {
        float dist = std::hypot(exitArrowCenter.x - scanOrigin.x,
                                exitArrowCenter.y - scanOrigin.y);
        if (dist > scanRadius + 14.0f) {
          drawExitArrow = false;
        }
      }
      if (drawExitArrow) {
        drawTacticalArrow(exitArrowCenter,
                          sf::Vector2f{static_cast<float>(outDir.x),
                                       static_cast<float>(outDir.y)},
                          exitTheme);
      }
    }
  }

  void renderRouteCompletionEffects(sf::RenderTarget &target,
                                    const map::Maze &maze,
                                    bool isExitDisabled = false,
                                    float scanRadius = -1.0f,
                                    sf::Vector2f scanOrigin = {0.0f, 0.0f}) {
    if (routeCompletionTimer_ <= 0.0f)
      return;
    if (scanRadius >= 0.0f) {
      core::Point startPt = maze.getStartPoint();
      sf::Vector2f sCenter{startPt.x * TILE_SIZE + TILE_SIZE * 0.5f,
                           startPt.y * TILE_SIZE + TILE_SIZE * 0.5f};
      if (std::hypot(sCenter.x - scanOrigin.x, sCenter.y - scanOrigin.y) >
          scanRadius + 14.0f)
        return;
    }

    float tNorm =
        1.0f - (routeCompletionTimer_ / 1.0f); // 0.0 to 1.0 over the 1.0s
    float fadeAlpha =
        (tNorm < 0.12f) ? (tNorm / 0.12f) : ((1.0f - tNorm) / 0.88f);
    fadeAlpha = std::clamp(fadeAlpha, 0.0f, 1.0f);
    float pulse = 0.5f + 0.5f * std::sin(tNorm * 18.0f);

    core::Point startPt = maze.getStartPoint();
    core::Point exitPt = maze.getExitPoint();
    bool isSamePoint =
        (startPt.x == exitPt.x && startPt.y == exitPt.y) || isExitDisabled;

    auto drawBeaconRing = [&](sf::Vector2f center, float radius,
                              float thickness, sf::Color color) {
      sf::CircleShape ring(radius);
      ring.setOrigin({radius, radius});
      ring.setPosition(center);
      ring.setFillColor(sf::Color::Transparent);
      ring.setOutlineThickness(thickness);
      ring.setOutlineColor(color);
      target.draw(ring);
    };

    if (isSamePoint) {
      // Return-to-entrance scenario: dual-element highlight on single
      // entry/exit tile
      sf::Vector2f center{startPt.x * TILE_SIZE + TILE_SIZE * 0.5f,
                          startPt.y * TILE_SIZE + TILE_SIZE * 0.5f};

      // 1. Dual glowing tile aura (outer emerald, inner amber)
      sf::RectangleShape auraPad({TILE_SIZE + 12.0f, TILE_SIZE + 12.0f});
      auraPad.setOrigin(
          {(TILE_SIZE + 12.0f) * 0.5f, (TILE_SIZE + 12.0f) * 0.5f});
      auraPad.setPosition(center);
      auraPad.setFillColor(
          sf::Color(40, 240, 140, static_cast<uint8_t>(80.0f * fadeAlpha)));
      target.draw(auraPad);

      sf::RectangleShape goldPad({TILE_SIZE + 4.0f, TILE_SIZE + 4.0f});
      goldPad.setOrigin({(TILE_SIZE + 4.0f) * 0.5f, (TILE_SIZE + 4.0f) * 0.5f});
      goldPad.setPosition(center);
      goldPad.setFillColor(
          sf::Color(255, 210, 50, static_cast<uint8_t>(110.0f * fadeAlpha)));
      target.draw(goldPad);

      // 2. Dual expanding concentric ripples (emerald extraction wave + amber
      // beacon)
      float rEmer = 8.0f + tNorm * (TILE_SIZE * 1.6f);
      drawBeaconRing(
          center, rEmer, 2.5f * (1.0f - 0.4f * tNorm),
          sf::Color(60, 255, 160,
                    static_cast<uint8_t>(240.0f * fadeAlpha * (1.0f - tNorm))));

      float tOffset = std::max(0.0f, tNorm - 0.15f) / 0.85f;
      float rGold = 6.0f + tOffset * (TILE_SIZE * 1.3f);
      drawBeaconRing(center, rGold, 2.0f * (1.0f - 0.4f * tOffset),
                     sf::Color(255, 220, 60,
                               static_cast<uint8_t>(230.0f * fadeAlpha *
                                                    (1.0f - tOffset))));

      // 3. Central dual-color starburst core
      float coreR = 7.0f + pulse * 3.5f;
      sf::CircleShape core(coreR);
      core.setOrigin({coreR, coreR});
      core.setPosition(center);
      core.setFillColor(
          sf::Color(255, 255, 220, static_cast<uint8_t>(255.0f * fadeAlpha)));
      core.setOutlineThickness(2.0f);
      core.setOutlineColor(
          sf::Color(60, 255, 160, static_cast<uint8_t>(240.0f * fadeAlpha)));
      target.draw(core);

      // 4. Tactical cross reticle
      float crossLen = 14.0f + pulse * 4.0f;
      sf::RectangleShape hBar({crossLen * 2.0f, 2.5f});
      hBar.setOrigin({crossLen, 1.25f});
      hBar.setPosition(center);
      hBar.setFillColor(
          sf::Color(255, 255, 255, static_cast<uint8_t>(230.0f * fadeAlpha)));
      target.draw(hBar);

      sf::RectangleShape vBar({2.5f, crossLen * 2.0f});
      vBar.setOrigin({1.25f, crossLen});
      vBar.setPosition(center);
      vBar.setFillColor(
          sf::Color(255, 255, 255, static_cast<uint8_t>(230.0f * fadeAlpha)));
      target.draw(vBar);
    } else {
      // Standard scenario: Entrance in amber/gold, Exit in emerald/green
      sf::Vector2f startCenter{startPt.x * TILE_SIZE + TILE_SIZE * 0.5f,
                               startPt.y * TILE_SIZE + TILE_SIZE * 0.5f};
      sf::Vector2f exitCenter{exitPt.x * TILE_SIZE + TILE_SIZE * 0.5f,
                              exitPt.y * TILE_SIZE + TILE_SIZE * 0.5f};

      // --- ENTRANCE (Amber Beacon) ---
      sf::RectangleShape startPad({TILE_SIZE + 8.0f, TILE_SIZE + 8.0f});
      startPad.setOrigin(
          {(TILE_SIZE + 8.0f) * 0.5f, (TILE_SIZE + 8.0f) * 0.5f});
      startPad.setPosition(startCenter);
      startPad.setFillColor(
          sf::Color(255, 205, 50, static_cast<uint8_t>(90.0f * fadeAlpha)));
      target.draw(startPad);

      float rStart = 6.0f + tNorm * (TILE_SIZE * 1.4f);
      drawBeaconRing(
          startCenter, rStart, 2.2f * (1.0f - 0.4f * tNorm),
          sf::Color(255, 220, 60,
                    static_cast<uint8_t>(230.0f * fadeAlpha * (1.0f - tNorm))));

      float startCoreR = 6.5f + pulse * 2.5f;
      sf::CircleShape startCore(startCoreR);
      startCore.setOrigin({startCoreR, startCoreR});
      startCore.setPosition(startCenter);
      startCore.setFillColor(
          sf::Color(255, 255, 200, static_cast<uint8_t>(250.0f * fadeAlpha)));
      startCore.setOutlineThickness(1.8f);
      startCore.setOutlineColor(
          sf::Color(255, 200, 40, static_cast<uint8_t>(240.0f * fadeAlpha)));
      target.draw(startCore);

      // --- EXIT (Emerald Airlock Landing Beacon) ---
      sf::RectangleShape exitPad({TILE_SIZE + 10.0f, TILE_SIZE + 10.0f});
      exitPad.setOrigin(
          {(TILE_SIZE + 10.0f) * 0.5f, (TILE_SIZE + 10.0f) * 0.5f});
      exitPad.setPosition(exitCenter);
      exitPad.setFillColor(
          sf::Color(40, 240, 120, static_cast<uint8_t>(100.0f * fadeAlpha)));
      target.draw(exitPad);

      float rExit1 = 6.0f + tNorm * (TILE_SIZE * 1.5f);
      drawBeaconRing(
          exitCenter, rExit1, 2.4f * (1.0f - 0.4f * tNorm),
          sf::Color(80, 255, 170,
                    static_cast<uint8_t>(240.0f * fadeAlpha * (1.0f - tNorm))));

      float tOffset = std::max(0.0f, tNorm - 0.20f) / 0.80f;
      float rExit2 = 5.0f + tOffset * (TILE_SIZE * 1.2f);
      drawBeaconRing(exitCenter, rExit2, 1.8f * (1.0f - 0.4f * tOffset),
                     sf::Color(140, 255, 200,
                               static_cast<uint8_t>(200.0f * fadeAlpha *
                                                    (1.0f - tOffset))));

      float exitCoreR = 7.0f + pulse * 3.0f;
      sf::CircleShape exitCore(exitCoreR);
      exitCore.setOrigin({exitCoreR, exitCoreR});
      exitCore.setPosition(exitCenter);
      exitCore.setFillColor(
          sf::Color(220, 255, 235, static_cast<uint8_t>(255.0f * fadeAlpha)));
      exitCore.setOutlineThickness(2.0f);
      exitCore.setOutlineColor(
          sf::Color(50, 255, 140, static_cast<uint8_t>(245.0f * fadeAlpha)));
      target.draw(exitCore);

      float crossLen = 13.0f + pulse * 3.0f;
      sf::RectangleShape hBar({crossLen * 2.0f, 2.5f});
      hBar.setOrigin({crossLen, 1.25f});
      hBar.setPosition(exitCenter);
      hBar.setFillColor(
          sf::Color(255, 255, 255, static_cast<uint8_t>(235.0f * fadeAlpha)));
      target.draw(hBar);

      sf::RectangleShape vBar({2.5f, crossLen * 2.0f});
      vBar.setOrigin({1.25f, crossLen});
      vBar.setPosition(exitCenter);
      vBar.setFillColor(
          sf::Color(255, 255, 255, static_cast<uint8_t>(235.0f * fadeAlpha)));
      target.draw(vBar);
    }
  }

  void renderItems(sf::RenderTarget &target, const map::Maze &maze,
                   const std::vector<int> &collectedItemIds,
                   float scanRadius = -1.0f,
                   sf::Vector2f scanOrigin = {0.0f, 0.0f}) {
    if (scanRadius >= 0.0f) {
      updateCachedDistances(maze, scanOrigin);
    }

    for (const auto &item : maze.getItems()) {
      bool collected =
          (std::find(collectedItemIds.begin(), collectedItemIds.end(),
                     item.id) != collectedItemIds.end());

      if (scanRadius >= 0.0f) {
        int lx = item.position.x + SCAN_PAD_TILES;
        int ly = item.position.y + SCAN_PAD_TILES;
        int totalW = maze.getWidth() + 2 * SCAN_PAD_TILES;
        float dist = cachedDistances_[ly * totalW + lx];
        if (dist > scanRadius + 14.0f) {
          continue; // Scrap not reached by radar scan yet
        }
      }

      renderSingleScrapItem(target, item, collected);
    }
  }

  void renderSingleScrapItem(sf::RenderTarget &target, const core::Item &item,
                             bool collected) {
    float px = item.position.x * TILE_SIZE;
    float py = item.position.y * TILE_SIZE;
    float cx = px + TILE_SIZE * 0.5f;
    float cy = py + TILE_SIZE * 0.5f;

    float radius = (item.category == core::ItemCategory::LightGem) ? 6.5f
                   : (item.category == core::ItemCategory::MediumRelic)
                       ? 8.5f
                       : 11.0f;

    sf::Color catColor = (item.category == core::ItemCategory::LightGem)
                             ? sf::Color(0, 230, 255)
                         : (item.category == core::ItemCategory::MediumRelic)
                             ? sf::Color(255, 215, 0)
                             : sf::Color(255, 75, 75);

    if (!collected) {
      // Active uncollected scrap item
      sf::RectangleShape pad({TILE_SIZE - 4.0f, TILE_SIZE - 4.0f});
      pad.setPosition({px + 2.0f, py + 2.0f});
      pad.setFillColor(sf::Color(24, 32, 44, 230));

      float pulse = 0.5f + 0.5f * std::sin(animTime_ * 3.5f + item.id * 0.8f);
      sf::Color padBorder = catColor;
      padBorder.a = static_cast<uint8_t>(130 + 100 * pulse);
      pad.setOutlineThickness(-1.2f);
      pad.setOutlineColor(padBorder);
      target.draw(pad);

      // Floor aura under item
      sf::CircleShape floorAura(radius + 5.0f);
      floorAura.setOrigin({radius + 5.0f, radius + 5.0f});
      floorAura.setPosition({cx, cy});
      sf::Color auraCol = catColor;
      auraCol.a = static_cast<uint8_t>(35 + 25 * pulse);
      floorAura.setFillColor(auraCol);
      target.draw(floorAura);

      // Corner indicator beacons
      sf::RectangleShape led({2.5f, 2.5f});
      led.setFillColor(sf::Color(catColor.r, catColor.g, catColor.b,
                                 static_cast<uint8_t>(160 + 95 * pulse)));
      led.setPosition({px + 3.0f, py + 3.0f});
      target.draw(led);
      led.setPosition({px + TILE_SIZE - 5.5f, py + 3.0f});
      target.draw(led);
      led.setPosition({px + 3.0f, py + TILE_SIZE - 5.5f});
      target.draw(led);
      led.setPosition({px + TILE_SIZE - 5.5f, py + TILE_SIZE - 5.5f});
      target.draw(led);

      // Tactical corner brackets
      float bSize = 6.0f;
      float off = 13.0f;
      sf::Color bracketColor(catColor.r, catColor.g, catColor.b, 200);
      sf::VertexArray brackets(sf::PrimitiveType::Lines);
      brackets.append({{cx - off, cy - off + bSize}, bracketColor});
      brackets.append({{cx - off, cy - off}, bracketColor});
      brackets.append({{cx - off, cy - off}, bracketColor});
      brackets.append({{cx - off + bSize, cy - off}, bracketColor});
      brackets.append({{cx + off - bSize, cy - off}, bracketColor});
      brackets.append({{cx + off, cy - off}, bracketColor});
      brackets.append({{cx + off, cy - off}, bracketColor});
      brackets.append({{cx + off, cy - off + bSize}, bracketColor});
      brackets.append({{cx - off, cy + off - bSize}, bracketColor});
      brackets.append({{cx - off, cy + off}, bracketColor});
      brackets.append({{cx - off, cy + off}, bracketColor});
      brackets.append({{cx - off + bSize, cy + off}, bracketColor});
      brackets.append({{cx + off - bSize, cy + off}, bracketColor});
      brackets.append({{cx + off, cy + off}, bracketColor});
      brackets.append({{cx + off, cy + off}, bracketColor});
      brackets.append({{cx + off, cy + off - bSize}, bracketColor});
      target.draw(brackets);

      // Item bobbing animation
      float bob = std::sin(animTime_ * 3.2f + item.id * 1.3f) * 2.2f;
      float itemY = cy + bob;

      // Pulsing scanner ring
      float pingPhase = std::fmod(animTime_ * 1.8f + item.id * 0.45f, 1.0f);
      float pingR = radius + pingPhase * 11.0f;
      sf::CircleShape pingRing(pingR);
      pingRing.setOrigin({pingR, pingR});
      pingRing.setPosition({cx, itemY});
      pingRing.setFillColor(sf::Color::Transparent);
      pingRing.setOutlineThickness(1.2f);
      sf::Color ringCol = catColor;
      ringCol.a = static_cast<uint8_t>((1.0f - pingPhase) * 170.0f);
      pingRing.setOutlineColor(ringCol);
      target.draw(pingRing);

      // Scrap item core and specular glint
      sf::CircleShape itemShape(radius);
      itemShape.setOrigin({radius, radius});
      itemShape.setPosition({cx, itemY});
      itemShape.setFillColor(catColor);
      itemShape.setOutlineThickness(2.0f);
      itemShape.setOutlineColor(sf::Color(255, 255, 255, 240));
      target.draw(itemShape);

      sf::CircleShape glint(radius * 0.35f);
      glint.setOrigin({radius * 0.35f, radius * 0.35f});
      glint.setPosition({cx - radius * 0.3f, itemY - radius * 0.3f});
      glint.setFillColor(sf::Color(255, 255, 255, 220));
      target.draw(glint);

    } else {
      // Empty scavenged pedestal for collected items
      sf::RectangleShape emptyPad({TILE_SIZE - 4.0f, TILE_SIZE - 4.0f});
      emptyPad.setPosition({px + 2.0f, py + 2.0f});
      emptyPad.setFillColor(sf::Color(14, 22, 32, 210));
      emptyPad.setOutlineThickness(-1.0f);
      emptyPad.setOutlineColor(sf::Color(52, 72, 94, 150));
      target.draw(emptyPad);

      sf::CircleShape emptySocket(5.0f);
      emptySocket.setOrigin({5.0f, 5.0f});
      emptySocket.setPosition({cx, cy});
      emptySocket.setFillColor(sf::Color(10, 14, 20, 240));
      emptySocket.setOutlineThickness(1.0f);
      emptySocket.setOutlineColor(sf::Color(55, 76, 98, 160));
      target.draw(emptySocket);

      sf::VertexArray cross(sf::PrimitiveType::Lines);
      sf::Color crossCol(65, 88, 112, 140);
      cross.append({{cx - 2.5f, cy - 2.5f}, crossCol});
      cross.append({{cx + 2.5f, cy + 2.5f}, crossCol});
      cross.append({{cx + 2.5f, cy - 2.5f}, crossCol});
      cross.append({{cx - 2.5f, cy + 2.5f}, crossCol});
      target.draw(cross);

      sf::RectangleShape rivet({2.0f, 2.0f});
      rivet.setFillColor(sf::Color(45, 60, 78, 140));
      rivet.setPosition({px + 3.0f, py + 3.0f});
      target.draw(rivet);
      rivet.setPosition({px + TILE_SIZE - 5.0f, py + 3.0f});
      target.draw(rivet);
      rivet.setPosition({px + 3.0f, py + TILE_SIZE - 5.0f});
      target.draw(rivet);
      rivet.setPosition({px + TILE_SIZE - 5.0f, py + TILE_SIZE - 5.0f});
      target.draw(rivet);

      float bSize = 4.5f;
      float off = 11.5f;
      sf::Color dimBracket(48, 62, 78, 110);
      sf::VertexArray brackets(sf::PrimitiveType::Lines);
      brackets.append({{cx - off, cy - off + bSize}, dimBracket});
      brackets.append({{cx - off, cy - off}, dimBracket});
      brackets.append({{cx - off, cy - off}, dimBracket});
      brackets.append({{cx - off + bSize, cy - off}, dimBracket});
      brackets.append({{cx + off - bSize, cy - off}, dimBracket});
      brackets.append({{cx + off, cy - off}, dimBracket});
      brackets.append({{cx + off, cy - off}, dimBracket});
      brackets.append({{cx + off, cy - off + bSize}, dimBracket});
      brackets.append({{cx - off, cy + off - bSize}, dimBracket});
      brackets.append({{cx - off, cy + off}, dimBracket});
      brackets.append({{cx - off, cy + off}, dimBracket});
      brackets.append({{cx - off + bSize, cy + off}, dimBracket});
      brackets.append({{cx + off - bSize, cy + off}, dimBracket});
      brackets.append({{cx + off, cy + off}, dimBracket});
      brackets.append({{cx + off, cy + off}, dimBracket});
      brackets.append({{cx + off, cy + off - bSize}, dimBracket});
      target.draw(brackets);
    }
  }

  void renderSpotlightOverlay(
      sf::RenderTarget &target, const map::Maze &maze, const sf::View &view,
      int hoveredCollectedItemId,
      const std::string &hoveredCatalogItemName,
      int hoveredCatalogCategory,
      const std::vector<int> &collectedIds) {
    // Smooth fade in / out animation for catalog item spotlight
    bool hasActiveTarget = (!hoveredCatalogItemName.empty() || hoveredCatalogCategory >= 0);
    float targetCatalogFade = hasActiveTarget ? 1.0f : 0.0f;
    if (!hoveredCatalogItemName.empty()) {
      lastActiveCatalogItemName_ = hoveredCatalogItemName;
      lastActiveCatalogCategory_ = -1;
    } else if (hoveredCatalogCategory >= 0) {
      lastActiveCatalogCategory_ = hoveredCatalogCategory;
      lastActiveCatalogItemName_.clear();
    }
    float fadeSpeed = 7.5f; // Smooth ~0.25s fade transition
    catalogHoverAlpha_ += (targetCatalogFade - catalogHoverAlpha_) *
                          std::clamp(lastDt_ * fadeSpeed, 0.0f, 1.0f);
    if (std::abs(targetCatalogFade - catalogHoverAlpha_) < 0.004f) {
      catalogHoverAlpha_ = targetCatalogFade;
    }

    if (hoveredCollectedItemId < 0 && catalogHoverAlpha_ <= 0.001f) {
      return;
    }

    if (hoveredCollectedItemId >= 0) {
      auto optItem = maze.getItemById(hoveredCollectedItemId);
      if (!optItem.has_value())
        return;

      float cx = (optItem->position.x + 0.5f) * TILE_SIZE;
      float cy = (optItem->position.y + 0.5f) * TILE_SIZE;
      float spotR = 1.55f * TILE_SIZE; // 1 cell radius outside the item cell

      // 1. Inverted dark mask covering entire screen outside the circle
      sf::CircleShape darkMask(spotR);
      darkMask.setOrigin({spotR, spotR});
      darkMask.setPosition({cx, cy});
      darkMask.setFillColor(sf::Color::Transparent);
      darkMask.setOutlineThickness(4000.0f);
      darkMask.setOutlineColor(sf::Color(6, 8, 12, 175));
      target.draw(darkMask);

      // 2. High-tech glowing neon pulse ring
      float pulse = 0.5f + 0.5f * std::sin(animTime_ * 6.0f);
      sf::Color ringCol = (optItem->category == core::ItemCategory::LightGem)
                              ? sf::Color(0, 230, 255)
                          : (optItem->category == core::ItemCategory::MediumRelic)
                              ? sf::Color(255, 215, 0)
                              : sf::Color(255, 75, 75);

      sf::CircleShape glowRing(spotR);
      glowRing.setOrigin({spotR, spotR});
      glowRing.setPosition({cx, cy});
      glowRing.setFillColor(sf::Color::Transparent);
      glowRing.setOutlineThickness(3.0f);
      glowRing.setOutlineColor(sf::Color(ringCol.r, ringCol.g, ringCol.b,
                                         static_cast<uint8_t>(100 + 80 * pulse)));
      target.draw(glowRing);

      sf::CircleShape coreRing(spotR);
      coreRing.setOrigin({spotR, spotR});
      coreRing.setPosition({cx, cy});
      coreRing.setFillColor(sf::Color::Transparent);
      coreRing.setOutlineThickness(1.5f);
      coreRing.setOutlineColor(
          sf::Color(255, 255, 255, static_cast<uint8_t>(200 + 55 * pulse)));
      target.draw(coreRing);

      // 4 Crosshair ticks on the ring
      for (float angleDeg : {0.0f, 90.0f, 180.0f, 270.0f}) {
        float rad = angleDeg * 3.14159265f / 180.0f;
        float p1x = cx + std::cos(rad) * (spotR - 5.0f);
        float p1y = cy + std::sin(rad) * (spotR - 5.0f);
        float p2x = cx + std::cos(rad) * (spotR + 5.0f);
        float p2y = cy + std::sin(rad) * (spotR + 5.0f);
        sf::Vertex tick[2] = {
            {{p1x, p1y}, sf::Color(255, 255, 255, 220)},
            {{p2x, p2y}, sf::Color(ringCol.r, ringCol.g, ringCol.b, 240)}};
        target.draw(tick, 2, sf::PrimitiveType::Lines);
      }
      return;
    }

    if (catalogHoverAlpha_ > 0.001f &&
        (!lastActiveCatalogItemName_.empty() || lastActiveCatalogCategory_ >= 0)) {
      std::vector<const core::Item *> matchedItems;
      if (lastActiveCatalogCategory_ >= 0) {
        for (const auto &item : maze.getItems()) {
          if (static_cast<int>(item.category) == lastActiveCatalogCategory_) {
            matchedItems.push_back(&item);
          }
        }
      } else {
        for (const auto &item : maze.getItems()) {
          if (item.name == lastActiveCatalogItemName_) {
            matchedItems.push_back(&item);
          }
        }
      }

      // Draw dark veil across the visible view with smooth fading alpha
      sf::Vector2f viewCenter = view.getCenter();
      sf::Vector2f viewSize = view.getSize();
      sf::RectangleShape darkVeil(viewSize * 2.5f);
      darkVeil.setOrigin(viewSize * 1.25f);
      darkVeil.setPosition(viewCenter);
      darkVeil.setFillColor(
          sf::Color(6, 8, 12, static_cast<uint8_t>(175.0f * catalogHoverAlpha_)));
      target.draw(darkVeil);

      float pulse = 0.5f + 0.5f * std::sin(animTime_ * 6.0f);

      // Re-render matching items and their cells with bright illumination
      for (const auto *pItem : matchedItems) {
        float px = pItem->position.x * TILE_SIZE;
        float py = pItem->position.y * TILE_SIZE;
        float cx = px + TILE_SIZE * 0.5f;
        float cy = py + TILE_SIZE * 0.5f;

        // 1. Fully bright cell terrain beneath the item
        sf::RectangleShape cellBg({TILE_SIZE, TILE_SIZE});
        cellBg.setPosition({px, py});
        const auto &cell = maze.at(pItem->position);
        uint8_t bgAlpha = static_cast<uint8_t>(255.0f * catalogHoverAlpha_);
        if (cell.getType() == core::CellType::Floor) {
          cellBg.setFillColor(sf::Color(44, 52, 64, bgAlpha));
        } else if (cell.getType() == core::CellType::Mud) {
          cellBg.setFillColor(sf::Color(58, 44, 28, bgAlpha));
        } else {
          cellBg.setFillColor(sf::Color(36, 44, 56, bgAlpha));
        }
        target.draw(cellBg);

        // 2. High-energy illuminated item icon
        bool isCollected =
            std::find(collectedIds.begin(), collectedIds.end(), pItem->id) !=
            collectedIds.end();
        renderSingleScrapItem(target, *pItem, isCollected);

        // Category color for rings & highlight
        sf::Color catColor = (pItem->category == core::ItemCategory::LightScrap)
                                 ? sf::Color(0, 230, 255)
                             : (pItem->category == core::ItemCategory::MediumScrap)
                                 ? sf::Color(255, 215, 0)
                                 : sf::Color(255, 75, 75);

        // 3. Glowing tactical highlight frame around the cell
        sf::RectangleShape highlightFrame({TILE_SIZE + 4.0f, TILE_SIZE + 4.0f});
        highlightFrame.setOrigin(
            {(TILE_SIZE + 4.0f) * 0.5f, (TILE_SIZE + 4.0f) * 0.5f});
        highlightFrame.setPosition({cx, cy});
        highlightFrame.setFillColor(sf::Color::Transparent);
        highlightFrame.setOutlineThickness(1.8f);
        highlightFrame.setOutlineColor(
            sf::Color(catColor.r, catColor.g, catColor.b,
                      static_cast<uint8_t>((180 + 75 * pulse) * catalogHoverAlpha_)));
        target.draw(highlightFrame);

        // 4. Subtle beacon ring expanding from the matching item
        float outRingR = 1.35f * TILE_SIZE;
        sf::CircleShape beaconRing(outRingR);
        beaconRing.setOrigin({outRingR, outRingR});
        beaconRing.setPosition({cx, cy});
        beaconRing.setFillColor(sf::Color::Transparent);
        beaconRing.setOutlineThickness(1.5f);
        beaconRing.setOutlineColor(
            sf::Color(catColor.r, catColor.g, catColor.b,
                      static_cast<uint8_t>((100 + 80 * pulse) * catalogHoverAlpha_)));
        target.draw(beaconRing);
      }
    }
  }

  void renderAgent(sf::RenderTarget &target, const simulation::Agent &agent,
                   float scanRadius = -1.0f,
                   sf::Vector2f scanOrigin = {0.0f, 0.0f}) {
    float cx = visualAgentPos_.x;
    float cy = visualAgentPos_.y;

    if (scanRadius >= 0.0f) {
      float dist = std::hypot(cx - scanOrigin.x, cy - scanOrigin.y);
      if (dist > scanRadius + 14.0f) {
        return; // Not reached by sonar scan yet
      }
    }

    // 1. EXTRACTION BEACON ANIMATION (Triggered when agent successfully
    // evacuated to exit)
    if (agent.getStatus() == simulation::AgentStatus::Success) {
      // Concentric expanding holographic extraction beacon rings
      for (int r = 0; r < 3; ++r) {
        float phase = std::fmod(animTime_ * 2.2f + r * 0.33f, 1.0f);
        float rSize = 10.0f + phase * 26.0f;
        float rAlpha = (1.0f - phase) * 220.0f;
        sf::CircleShape beaconRing(rSize);
        beaconRing.setOrigin({rSize, rSize});
        beaconRing.setPosition({cx, cy});
        beaconRing.setFillColor(sf::Color::Transparent);
        beaconRing.setOutlineThickness(2.0f);
        beaconRing.setOutlineColor(
            sf::Color(46, 255, 140, static_cast<uint8_t>(rAlpha)));
        target.draw(beaconRing);
      }

      // Sweeping holographic extraction beam
      float scanOffset = std::sin(animTime_ * 6.0f) * 15.0f;
      sf::RectangleShape scanBeam({30.0f, 2.5f});
      scanBeam.setOrigin({15.0f, 1.25f});
      scanBeam.setPosition({cx, cy + scanOffset});
      scanBeam.setFillColor(sf::Color(130, 255, 190, 230));
      target.draw(scanBeam);

      // Rotating 4-corner extraction brackets
      float bDist = 18.0f;
      float rotAngle = animTime_ * 1.5f;
      for (int i = 0; i < 4; ++i) {
        float a = rotAngle + i * (3.14159f * 0.5f);
        sf::Vector2f bPos = {cx + std::cos(a) * bDist,
                             cy + std::sin(a) * bDist};
        sf::CircleShape reticleDot(2.5f);
        reticleDot.setOrigin({2.5f, 2.5f});
        reticleDot.setPosition(bPos);
        reticleDot.setFillColor(sf::Color(46, 255, 140, 240));
        target.draw(reticleDot);
      }
    }

    // 2. Tactical agent aura (breathing pulse)
    float auraPulse = 0.5f + 0.5f * std::sin(animTime_ * 6.0f);
    float auraRadius = 14.0f + auraPulse * 2.5f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin({auraRadius, auraRadius});
    aura.setPosition({cx, cy});
    if (agent.getStatus() == simulation::AgentStatus::Success) {
      aura.setFillColor(
          sf::Color(46, 255, 140, static_cast<uint8_t>(90 + 50 * auraPulse)));
    } else if (agent.getStatus() == simulation::AgentStatus::Exhausted) {
      aura.setFillColor(
          sf::Color(255, 50, 50, static_cast<uint8_t>(90 + 50 * auraPulse)));
    } else {
      aura.setFillColor(
          sf::Color(255, 160, 0, static_cast<uint8_t>(85 + 45 * auraPulse)));
    }
    target.draw(aura);

    // 3. Core agent hazmat body
    sf::CircleShape body(11.0f);
    body.setOrigin({11.0f, 11.0f});
    body.setPosition({cx, cy});
    body.setFillColor(sf::Color(255, 200, 40));
    body.setOutlineThickness(2.5f);
    body.setOutlineColor(sf::Color(255, 255, 255));
    target.draw(body);
  }

  void renderPickupEffects(sf::RenderTarget &target, float scanRadius = -1.0f,
                           sf::Vector2f scanOrigin = {0.0f, 0.0f}) {
    for (const auto &effect : pickupEffects_) {
      if (scanRadius >= 0.0f) {
        float dist = std::hypot(effect.worldPos.x - scanOrigin.x,
                                effect.worldPos.y - scanOrigin.y);
        if (dist > scanRadius + 14.0f) {
          continue;
        }
      }
      float t = effect.elapsed / effect.duration; // 0.0 -> 1.0
      float alphaNorm = std::clamp(1.0f - t, 0.0f, 1.0f);
      uint8_t alpha = static_cast<uint8_t>(alphaNorm * 255.0f);

      sf::Color col = (effect.category == core::ItemCategory::LightGem)
                          ? sf::Color(0, 230, 255)
                      : (effect.category == core::ItemCategory::MediumRelic)
                          ? sf::Color(255, 215, 0)
                          : sf::Color(255, 75, 75);

      // 1. Expanding shockwave ring
      float ringRadius = 6.0f + 26.0f * std::sin(t * 1.57079f);
      sf::CircleShape ring(ringRadius);
      ring.setOrigin({ringRadius, ringRadius});
      ring.setPosition(effect.worldPos);
      ring.setFillColor(sf::Color::Transparent);
      ring.setOutlineThickness(std::max(1.0f, 2.8f * alphaNorm));
      ring.setOutlineColor(sf::Color(col.r, col.g, col.b, alpha));
      target.draw(ring);

      // 2. Starburst sparkle rays (6 particles bursting outward)
      float sparkleDist = 8.0f + 26.0f * t;
      for (int i = 0; i < 6; ++i) {
        float angle = i * (3.14159f / 3.0f) + t * 2.5f;
        sf::Vector2f spPos =
            effect.worldPos + sf::Vector2f{std::cos(angle) * sparkleDist,
                                           std::sin(angle) * sparkleDist};
        float sparkSize = std::max(0.8f, 2.5f * alphaNorm);
        sf::CircleShape spark(sparkSize);
        spark.setOrigin({sparkSize, sparkSize});
        spark.setPosition(spPos);
        spark.setFillColor(sf::Color(255, 255, 255, alpha));
        target.draw(spark);
      }

      // 3. Floating loot beacon (diamond icon rising upward)
      float floatY = effect.worldPos.y - 10.0f - 24.0f * t;
      float diamondSize = 6.0f * alphaNorm;
      sf::ConvexShape diamond(4);
      diamond.setPoint(0, {0.0f, -diamondSize});
      diamond.setPoint(1, {diamondSize, 0.0f});
      diamond.setPoint(2, {0.0f, diamondSize});
      diamond.setPoint(3, {-diamondSize, 0.0f});
      diamond.setPosition({effect.worldPos.x, floatY});
      diamond.setFillColor(
          sf::Color(col.r, col.g, col.b, static_cast<uint8_t>(alpha * 0.9f)));
      diamond.setOutlineThickness(1.2f);
      diamond.setOutlineColor(sf::Color(255, 255, 255, alpha));
      target.draw(diamond);
    }
  }

  // Lethal Company Radar Scanner World-Space Graphics
  void renderSonarScanEffects(sf::RenderTarget &target, sf::Vector2f scanOrigin,
                              float scanRadius, float maxRadius,
                              float scanProgress) {
    if (scanRadius < 0.0f)
      return;

    // Smooth fadeout as wave reaches outer map edges
    float waveFade = 1.0f;
    if (scanRadius > maxRadius - 260.0f) {
      float remain = std::max(0.0f, maxRadius - scanRadius);
      float frac = remain / 260.0f;
      waveFade = frac * frac * (3.0f - 2.0f * frac); // Smoothstep fadeout
    }
    if (waveFade <= 0.001f)
      return;

    // 1. Concentric distance range rings around facility entrance
    for (float r = 160.0f; r <= maxRadius + 120.0f; r += 160.0f) {
      sf::CircleShape rangeRing(r);
      rangeRing.setOrigin({r, r});
      rangeRing.setPosition(scanOrigin);
      rangeRing.setFillColor(sf::Color::Transparent);
      rangeRing.setOutlineThickness(1.0f);
      uint8_t ringAlpha = (r <= scanRadius) ? 48 : 22;
      rangeRing.setOutlineColor(
          sf::Color(0, 210, 245, static_cast<uint8_t>(ringAlpha * waveFade)));
      target.draw(rangeRing);
    }

    // 2. Tactical Radar Crosshair axes through scan origin
    sf::VertexArray crosshair(sf::PrimitiveType::Lines);
    sf::Color axisColor(0, 220, 250, static_cast<uint8_t>(42 * waveFade));
    crosshair.append({{scanOrigin.x - maxRadius, scanOrigin.y}, axisColor});
    crosshair.append({{scanOrigin.x + maxRadius, scanOrigin.y}, axisColor});
    crosshair.append({{scanOrigin.x, scanOrigin.y - maxRadius}, axisColor});
    crosshair.append({{scanOrigin.x, scanOrigin.y + maxRadius}, axisColor});
    target.draw(crosshair);

    // 3. Rotating Radar Sweep Beam & Trailing Cone Sector (2.5 full sweeps
    // across the animation)
    float sweepAngle = scanProgress * 5.0f * 3.14159265f;
    float beamLen = std::min(scanRadius + 45.0f, maxRadius);
    if (beamLen > 10.0f) {
      // Trailing sector wedge (45 degrees fan behind the beam)
      constexpr int FAN_STEPS = 16;
      constexpr float FAN_ANGLE = 0.785398f;
      sf::VertexArray sweepFan(sf::PrimitiveType::TriangleFan);
      sweepFan.append(
          {scanOrigin,
           sf::Color(0, 230, 255, static_cast<uint8_t>(65 * waveFade))});
      for (int i = 0; i <= FAN_STEPS; ++i) {
        float frac = static_cast<float>(i) / static_cast<float>(FAN_STEPS);
        float a = sweepAngle - frac * FAN_ANGLE;
        float cosA = std::cos(a);
        float sinA = std::sin(a);
        uint8_t edgeAlpha =
            static_cast<uint8_t>((1.0f - frac) * 55.0f * waveFade);
        sweepFan.append(
            {{scanOrigin.x + cosA * beamLen, scanOrigin.y + sinA * beamLen},
             sf::Color(0, 220, 255, edgeAlpha)});
      }
      target.draw(sweepFan);

      // Leading sharp sweep beam line
      sf::VertexArray sweepLine(sf::PrimitiveType::Lines);
      sf::Color beamColor(0, 255, 255, static_cast<uint8_t>(215 * waveFade));
      sweepLine.append({scanOrigin, beamColor});
      sweepLine.append({{scanOrigin.x + std::cos(sweepAngle) * beamLen,
                         scanOrigin.y + std::sin(sweepAngle) * beamLen},
                        beamColor});
      target.draw(sweepLine);
    }

    // 4. Secondary trailing echo rings
    if (scanRadius > 140.0f) {
      float r1 = scanRadius - 140.0f;
      sf::CircleShape echo1(r1);
      echo1.setOrigin({r1, r1});
      echo1.setPosition(scanOrigin);
      echo1.setFillColor(sf::Color::Transparent);
      echo1.setOutlineThickness(1.8f);
      echo1.setOutlineColor(
          sf::Color(0, 215, 255, static_cast<uint8_t>(75 * waveFade)));
      target.draw(echo1);
    }
    if (scanRadius > 280.0f) {
      float r2 = scanRadius - 280.0f;
      sf::CircleShape echo2(r2);
      echo2.setOrigin({r2, r2});
      echo2.setPosition(scanOrigin);
      echo2.setFillColor(sf::Color::Transparent);
      echo2.setOutlineThickness(1.2f);
      echo2.setOutlineColor(
          sf::Color(0, 195, 240, static_cast<uint8_t>(35 * waveFade)));
      target.draw(echo2);
    }

    // 5. Leading Sonar Wavefront: Outer soft glow and crisp cyan pulse ring
    if (scanRadius > 4.0f) {
      // Soft outer glow ring
      sf::CircleShape glowRing(scanRadius - 8.0f);
      glowRing.setOrigin({scanRadius - 8.0f, scanRadius - 8.0f});
      glowRing.setPosition(scanOrigin);
      glowRing.setFillColor(sf::Color::Transparent);
      glowRing.setOutlineThickness(16.0f);
      glowRing.setOutlineColor(
          sf::Color(0, 220, 255, static_cast<uint8_t>(45 * waveFade)));
      target.draw(glowRing);

      // Core high-energy pulse ring
      sf::CircleShape pulseRing(scanRadius);
      pulseRing.setOrigin({scanRadius, scanRadius});
      pulseRing.setPosition(scanOrigin);
      pulseRing.setFillColor(sf::Color::Transparent);
      pulseRing.setOutlineThickness(3.5f);
      pulseRing.setOutlineColor(
          sf::Color(0, 250, 255, static_cast<uint8_t>(235 * waveFade)));
      target.draw(pulseRing);
    }

    // 6. Center Pulse Beacon at origin (Start Airlock)
    float beaconPulse = std::fmod(animTime_ * 4.0f, 1.0f);
    float beaconR = 4.0f + beaconPulse * 24.0f;
    sf::CircleShape beaconWave(beaconR);
    beaconWave.setOrigin({beaconR, beaconR});
    beaconWave.setPosition(scanOrigin);
    beaconWave.setFillColor(sf::Color::Transparent);
    beaconWave.setOutlineThickness(1.5f);
    beaconWave.setOutlineColor(sf::Color(
        0, 255, 255, static_cast<uint8_t>((1.0f - beaconPulse) * 200.0f * waveFade)));
    target.draw(beaconWave);

    sf::CircleShape centerDot(5.0f);
    centerDot.setOrigin({5.0f, 5.0f});
    centerDot.setPosition(scanOrigin);
    centerDot.setFillColor(
        sf::Color(0, 255, 255, static_cast<uint8_t>(255 * waveFade)));
    target.draw(centerDot);
  }

private:
  sf::Vector2f visualAgentPos_{0.0f, 0.0f};
  bool visualAgentInitialized_ = false;
  float animTime_ = 0.0f;
  float lastDt_ = 0.016f;
  float catalogHoverAlpha_ = 0.0f;
  std::string lastActiveCatalogItemName_;
  int lastActiveCatalogCategory_ = -1;
  std::vector<PickupEffect> pickupEffects_;
  std::vector<int> lastCollectedIds_;
  std::vector<CellPaintEffect> paintEffects_;
  std::unordered_set<int> visitedHistorySet_;
  sf::Vector2f cachedScanOrigin_{-99999.0f, -99999.0f};
  int cachedMazeW_ = -1;
  int cachedMazeH_ = -1;
  std::vector<float> cachedDistances_;
  float routeCompletionTimer_ = 0.0f;
  bool wasRouteCompleted_ = false;

  sf::VertexArray baseTileQuads_{sf::PrimitiveType::Triangles};
  sf::VertexArray pingQuads_{sf::PrimitiveType::Triangles};
  sf::VertexArray pingBorders_{sf::PrimitiveType::Lines};
  sf::VertexArray gridLines_{sf::PrimitiveType::Lines};
};

} // namespace ui
