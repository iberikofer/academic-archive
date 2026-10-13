#pragma once

#include "ProceduralRenderer.hpp"
#include "algorithms/routing/RoutePlan.hpp"
#include "map/Maze.hpp"
#include "simulation/Agent.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>

namespace ui {

class Minimap {
public:
  static constexpr float BASE_FRAME_WIDTH =
      355.0f; // Exactly matches Legend panel width
  static constexpr float DEFAULT_MINIMAP_SIZE = BASE_FRAME_WIDTH;
  static constexpr float MINIMAP_SIZE = DEFAULT_MINIMAP_SIZE;
  static constexpr float PADDING = 2.0f;

  // Dimensions and inner transparent hole bounds of map.png monitor frame
  static constexpr float TEX_WIDTH = 1710.0f;
  static constexpr float TEX_HEIGHT = 1599.0f;
  static constexpr float HOLE_LEFT = 145.0f;
  static constexpr float HOLE_TOP = 142.0f;
  static constexpr float HOLE_WIDTH = 1415.0f;
  static constexpr float HOLE_HEIGHT = 1180.0f;

  // Monitor hardware power button and indicator LED on map.png bezel
  static constexpr float BTN_X = 1380.0f;
  static constexpr float BTN_Y = 1455.0f;
  static constexpr float BTN_W = 86.0f;
  static constexpr float BTN_H = 86.0f;
  static constexpr float LED_X = 1346.0f;
  static constexpr float LED_Y = 1491.0f;

  enum class PowerState { On, TurningOff, Off, TurningOn };

  struct Layout {
    float s = 1.0f;
    float sX = 1.0f;
    float sY = 1.0f;
    float frameW = 0.0f;
    float frameH = 0.0f;
    float frameX = 0.0f;
    float frameY = 0.0f;
    float screenX = 0.0f;
    float screenY = 0.0f;
    float screenW = 0.0f;
    float screenH = 0.0f;
    float contentW = 0.0f;
    float contentH = 0.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float mapScaleX = 1.0f;
    float mapScaleY = 1.0f;
    float mapScale = 1.0f;
  };

  static float getFrameScale(float scaleOrSize = 1.0f) noexcept {
    float scale =
        (scaleOrSize > 10.0f) ? (scaleOrSize / BASE_FRAME_WIDTH) : scaleOrSize;
    return (BASE_FRAME_WIDTH * scale) / TEX_WIDTH;
  }

  static float getFrameScaleX(float scaleOrSize = 1.0f) noexcept {
    return getFrameScale(scaleOrSize);
  }

  static float getFrameScaleY(float scaleOrSize = 1.0f) noexcept {
    return getFrameScale(scaleOrSize);
  }

  static float getFrameWidth(float scaleOrSize = 1.0f) noexcept {
    float scale =
        (scaleOrSize > 10.0f) ? (scaleOrSize / BASE_FRAME_WIDTH) : scaleOrSize;
    return BASE_FRAME_WIDTH * scale;
  }

  static float getFrameHeight(float scaleOrSize = 1.0f) noexcept {
    float scale =
        (scaleOrSize > 10.0f) ? (scaleOrSize / BASE_FRAME_WIDTH) : scaleOrSize;
    return TEX_HEIGHT * ((BASE_FRAME_WIDTH * scale) / TEX_WIDTH);
  }

  static Layout calculateLayout(float windowWidth, float windowHeight,
                                float scaleOrSize, float mazeW,
                                float mazeH) noexcept {
    Layout l;
    float scale =
        (scaleOrSize > 10.0f) ? (scaleOrSize / BASE_FRAME_WIDTH) : scaleOrSize;
    l.s = (BASE_FRAME_WIDTH * scale) / TEX_WIDTH;
    l.sX = l.s;
    l.sY = l.s;
    l.frameW =
        TEX_WIDTH * l.s; // Exactly BASE_FRAME_WIDTH * scale = 355.0f * scale
    l.frameH = TEX_HEIGHT * l.s;
    l.frameX = windowWidth - l.frameW - PADDING;
    l.frameY = windowHeight - l.frameH - PADDING;

    l.screenX = l.frameX + HOLE_LEFT * l.s;
    l.screenY = l.frameY + HOLE_TOP * l.s - 3.0f;
    l.screenW = HOLE_WIDTH * l.s;
    l.screenH = HOLE_HEIGHT * l.s + 3.0f;

    // Clean bezel margin inside monitor frame to sit neatly within rounded screen
    float innerPadding = 9.0f * scale;
    float fitW = std::max(10.0f, l.screenW - innerPadding * 2.0f);
    float fitH = std::max(10.0f, l.screenH - innerPadding * 2.0f);
    l.mapScaleX = (mazeW > 0.0f) ? (fitW / mazeW) : 1.0f;
    l.mapScaleY = (mazeH > 0.0f) ? (fitH / mazeH) : 1.0f;
    l.mapScale = std::min(l.mapScaleX, l.mapScaleY);
    l.contentW = mazeW * l.mapScaleX;
    l.contentH = mazeH * l.mapScaleY;
    l.offsetX = l.screenX + innerPadding;
    l.offsetY = l.screenY + innerPadding - 0.25f;
    return l;
  }

  void togglePower() {
    if (powerState_ == PowerState::On || powerState_ == PowerState::TurningOn) {
      powerState_ = PowerState::TurningOff;
    } else {
      powerState_ = PowerState::TurningOn;
    }
  }

  [[nodiscard]] bool isPoweredOn() const noexcept {
    return powerState_ == PowerState::On;
  }

  [[nodiscard]] bool isPowerButtonHovered() const noexcept {
    return isButtonHovered_;
  }

  [[nodiscard]] bool isOverPowerButton(int px, int py,
                                       const Layout &l) const noexcept {
    float btnX = l.frameX + BTN_X * l.s;
    float btnY = l.frameY + BTN_Y * l.s;
    float btnW = BTN_W * l.s;
    float btnH = BTN_H * l.s;
    float m = 6.0f * l.s;
    return px >= btnX - m && px <= btnX + btnW + m && py >= btnY - m &&
           py <= btnY + btnH + m;
  }

  [[nodiscard]] bool isOverFrame(int px, int py,
                                const Layout &l) const noexcept {
    return px >= l.frameX && px <= l.frameX + l.frameW && py >= l.frameY &&
           py <= l.frameY + l.frameH;
  }

    [[nodiscard]] bool isInsideMazeContent(int px, int py,
                                         const Layout &l) const noexcept {
    return px >= l.offsetX && px <= l.offsetX + l.contentW &&
           py >= l.offsetY && py <= l.offsetY + l.contentH;
  }

  [[nodiscard]] bool isInsideScreen(int px, int py,
                                    const Layout &l) const noexcept {
    return px >= l.screenX && px <= l.screenX + l.screenW && py >= l.screenY &&
           py <= l.screenY + l.screenH;
  }

  [[nodiscard]] bool isMouseOver(int px, int py, float windowWidth,
                                 float windowHeight,
                                 float scaleOrSize = 1.0f) const noexcept {
    Layout l =
        calculateLayout(windowWidth, windowHeight, scaleOrSize, 100.0f, 100.0f);
    return isOverFrame(px, py, l);
  }

  [[nodiscard]] bool isInsideMinimap(int px, int py, float windowWidth,
                                     float windowHeight,
                                     float scaleOrSize = 1.0f) const noexcept {
    Layout l =
        calculateLayout(windowWidth, windowHeight, scaleOrSize, 100.0f, 100.0f);
    return isInsideScreen(px, py, l);
  }

  [[nodiscard]] float getZoomLevel() const noexcept { return 1.0f; }

  [[nodiscard]] bool isRmbPanning() const noexcept { return false; }

  void resetZoom() noexcept {}

  Minimap() { loadTexture(); }

  void loadTexture() {
    if (!hasFrameTexture_) {
      hasFrameTexture_ =
          frameTexture_.loadFromFile("assets/map.png") ||
          frameTexture_.loadFromFile("../assets/map.png") ||
          frameTexture_.loadFromFile("build/assets/map.png") ||
          frameTexture_.loadFromFile("course-projects/DSA-cpp/assets/map.png");
      if (hasFrameTexture_) {
        frameTexture_.setSmooth(true);
      }
    }
  }

  void render(sf::RenderTarget &target, const map::Maze &maze,
              const sf::View &cameraView, const simulation::Agent &agent,
              const std::vector<int> &collectedItemIds, float windowWidth,
              float windowHeight,
              sf::Vector2f visualAgentWorldPos = {-1.0f, -1.0f},
              float scaleOrSize = 1.0f, float scanRadius = -1.0f,
              sf::Vector2f scanOrigin = {0.0f, 0.0f}, float fadeAlpha = 0.0f,
              const algorithms::routing::RoutePlan *routePlan = nullptr,
              float routeProgress = 1.0f, bool isExitDisabled = false) {
    loadTexture();

    float dt = glowClock_.restart().asSeconds();
    if (dt > 0.1f)
      dt = 0.1f;

    if (routeCompletionTimer_ > 0.0f) {
      routeCompletionTimer_ = std::max(0.0f, routeCompletionTimer_ - dt);
    }

    // Animate monitor CRT power state (fast 0.20s authentic CRT power
    // transition)
    constexpr float POWER_ANIM_DUR = 0.20f;
    if (powerState_ == PowerState::TurningOff) {
      powerAnimProgress_ -= dt / POWER_ANIM_DUR;
      if (powerAnimProgress_ <= 0.0f) {
        powerAnimProgress_ = 0.0f;
        powerState_ = PowerState::Off;
      }
    } else if (powerState_ == PowerState::TurningOn) {
      powerAnimProgress_ += dt / POWER_ANIM_DUR;
      if (powerAnimProgress_ >= 1.0f) {
        powerAnimProgress_ = 1.0f;
        powerState_ = PowerState::On;
      }
    }

    float mazeW = maze.getWidth() * ProceduralRenderer::TILE_SIZE;
    float mazeH = maze.getHeight() * ProceduralRenderer::TILE_SIZE;
    Layout l =
        calculateLayout(windowWidth, windowHeight, scaleOrSize, mazeW, mazeH);

    // Hardware OpenGL clipping strictly to the CRT monitor screen aperture hole
    sf::View prevView = target.getView();
    sf::Vector2u winSize = target.getSize();
    float tw = static_cast<float>(winSize.x);
    float th = static_cast<float>(winSize.y);

    sf::View apertureView(sf::FloatRect(sf::Vector2f{l.screenX, l.screenY},
                                        sf::Vector2f{l.screenW, l.screenH}));
    apertureView.setViewport(
        sf::FloatRect(sf::Vector2f{l.screenX / tw, l.screenY / th},
                      sf::Vector2f{l.screenW / tw, l.screenH / th}));
    target.setView(apertureView);

    // 1. CRT Monitor inner screen background panel (dark cathode glass when On,
    // pitch black when Off)
    sf::Color screenBgColor;
    if (powerState_ == PowerState::Off) {
      screenBgColor = sf::Color(0, 0, 0, 255);
    } else if (powerState_ == PowerState::On) {
      screenBgColor = sf::Color(22, 27, 36, 255);
    } else {
      float t = powerAnimProgress_;
      uint8_t r = static_cast<uint8_t>(22.0f * t);
      uint8_t g = static_cast<uint8_t>(27.0f * t);
      uint8_t b = static_cast<uint8_t>(36.0f * t);
      screenBgColor = sf::Color(r, g, b, 255);
    }

    sf::RectangleShape bg({l.screenW, l.screenH});
    bg.setPosition({l.screenX, l.screenY});
    bg.setFillColor(screenBgColor);
    target.draw(bg);

    // If monitor is not completely powered off, render map contents with CRT
    // beam scaling!
    if (powerState_ != PowerState::Off) {
      float vScale = 1.0f;
      float hScale = 1.0f;
      float flashAlpha = 0.0f;

      if (powerState_ == PowerState::TurningOff ||
          powerState_ == PowerState::TurningOn) {
        float p = powerAnimProgress_;
        if (p < 0.25f) {
          hScale = std::clamp(p / 0.25f, 0.005f, 1.0f);
          vScale = 0.012f;
        } else {
          hScale = 1.0f;
          vScale = std::clamp((p - 0.25f) / 0.75f, 0.012f, 1.0f);
        }
        if (powerState_ == PowerState::TurningOn) {
          flashAlpha = std::clamp((1.0f - std::abs(p - 0.5f) * 2.0f) * 0.45f,
                                  0.0f, 1.0f);
        }
      }

      float curW = l.screenW * hScale;
      float curH = l.screenH * vScale;
      float curX = l.screenX + (l.screenW - curW) * 0.5f;
      float curY = l.screenY + (l.screenH - curH) * 0.5f;

      sf::View screenView(sf::FloatRect(sf::Vector2f{l.screenX, l.screenY},
                                        sf::Vector2f{l.screenW, l.screenH}));
      screenView.setViewport(sf::FloatRect(sf::Vector2f{curX / tw, curY / th},
                                           sf::Vector2f{curW / tw, curH / th}));
      target.setView(screenView);

      // 2. Draw miniature tiles inside the monitor screen (revealed gradually
      // from scanOrigin if scanning)
      // 2. Draw miniature tiles inside the monitor screen (revealed gradually
      // from scanOrigin if scanning) - Batched Geometry Optimization
      float tileScaledW = ProceduralRenderer::TILE_SIZE * l.mapScaleX;
      float tileScaledH = ProceduralRenderer::TILE_SIZE * l.mapScaleY;
      float tileW = std::max(1.5f, tileScaledW);
      float tileH = std::max(1.5f, tileScaledH);

      auto &miniQuads = minimapQuads_;
      size_t maxMiniVerts = static_cast<size_t>(maze.getWidth() * maze.getHeight() * 6);
      if (miniQuads.getVertexCount() < maxMiniVerts) {
        miniQuads.resize(maxMiniVerts);
      }
      size_t miniQuadCount = 0;

      auto &miniBorders = minimapBorders_;
      size_t maxBorderVerts = static_cast<size_t>(maze.getWidth() * maze.getHeight() * 8);
      if (miniBorders.getVertexCount() < maxBorderVerts) {
        miniBorders.resize(maxBorderVerts);
      }
      size_t miniBorderCount = 0;

      bool hasExitMarker = false;
      sf::Vector2f exitTileCenter{0.0f, 0.0f};

      for (int y = 0; y < maze.getHeight(); ++y) {
        for (int x = 0; x < maze.getWidth(); ++x) {
          const auto &cell = maze.at(x, y);
          if (cell.getType() == core::CellType::Wall)
            continue;

          float cx = (x + 0.5f) * ProceduralRenderer::TILE_SIZE;
          float cy = (y + 0.5f) * ProceduralRenderer::TILE_SIZE;
          float d = (scanRadius >= 0.0f)
                        ? std::hypot(cx - scanOrigin.x, cy - scanOrigin.y)
                        : 0.0f;
          if (scanRadius >= 0.0f && d > scanRadius + 14.0f) {
            continue; // Unscanned cell hidden on minimap
          }

          float px = l.offsetX + x * tileScaledW;
          float py = l.offsetY + y * tileScaledH;

          sf::Color baseColor = sf::Color(75, 88, 108);
          bool hasOutline = false;
          sf::Color outlineColor = sf::Color::Transparent;

          if (cell.isWalked() && cell.getType() != core::CellType::Start &&
              cell.getType() != core::CellType::Exit) {
            if (cell.getType() == core::CellType::Mud) {
              baseColor = sf::Color(88, 125, 115);
              hasOutline = true;
              outlineColor = sf::Color(65, 225, 215, 220);
            } else {
              baseColor = sf::Color(0, 95, 125);
              hasOutline = true;
              outlineColor = sf::Color(0, 230, 245, 190);
            }
          } else {
            switch (cell.getType()) {
            case core::CellType::Floor:
              baseColor = sf::Color(75, 88, 108);
              break;
            case core::CellType::Mud:
              baseColor = sf::Color(120, 90, 50);
              break;
            case core::CellType::Start:
              baseColor = sf::Color(255, 140, 0);
              break;
            case core::CellType::Exit:
              baseColor = isExitDisabled ? sf::Color(85, 92, 102)
                                         : sf::Color(50, 205, 50);
              break;
            default:
              break;
            }
          }

          sf::Color finalTileColor = baseColor;
          if (scanRadius >= 0.0f && scanRadius - d < 55.0f &&
              scanRadius - d >= -12.0f) {
            float waveDist = scanRadius - d;
            float t = (waveDist < 0.0f) ? ((waveDist + 12.0f) / 12.0f)
                                        : (1.0f - (waveDist / 55.0f));
            float intensity = t * t * (3.0f - 2.0f * t);
            sf::Color pingColor(0, 245, 255);
            finalTileColor = sf::Color(
                static_cast<uint8_t>(intensity * pingColor.r +
                                     (1.0f - intensity) * baseColor.r),
                static_cast<uint8_t>(intensity * pingColor.g +
                                     (1.0f - intensity) * baseColor.g),
                static_cast<uint8_t>(intensity * pingColor.b +
                                     (1.0f - intensity) * baseColor.b));
          }

          float pxNext = px + tileW;
          float pyNext = py + tileH;

          miniQuads[miniQuadCount + 0] = {{px, py}, finalTileColor};
          miniQuads[miniQuadCount + 1] = {{pxNext, py}, finalTileColor};
          miniQuads[miniQuadCount + 2] = {{pxNext, pyNext}, finalTileColor};
          miniQuads[miniQuadCount + 3] = {{px, py}, finalTileColor};
          miniQuads[miniQuadCount + 4] = {{pxNext, pyNext}, finalTileColor};
          miniQuads[miniQuadCount + 5] = {{px, pyNext}, finalTileColor};
          miniQuadCount += 6;

          if (hasOutline) {
            miniBorders[miniBorderCount + 0] = {{px, py}, outlineColor};
            miniBorders[miniBorderCount + 1] = {{pxNext, py}, outlineColor};
            miniBorders[miniBorderCount + 2] = {{pxNext, py}, outlineColor};
            miniBorders[miniBorderCount + 3] = {{pxNext, pyNext}, outlineColor};
            miniBorders[miniBorderCount + 4] = {{pxNext, pyNext}, outlineColor};
            miniBorders[miniBorderCount + 5] = {{px, pyNext}, outlineColor};
            miniBorders[miniBorderCount + 6] = {{px, pyNext}, outlineColor};
            miniBorders[miniBorderCount + 7] = {{px, py}, outlineColor};
            miniBorderCount += 8;
          }

          if (cell.getType() == core::CellType::Exit) {
            hasExitMarker = true;
            exitTileCenter = {l.offsetX + (x + 0.5f) * tileScaledW,
                              l.offsetY + (y + 0.5f) * tileScaledH};
          }
        }
      }

      if (miniQuadCount > 0) {
        miniQuads.resize(miniQuadCount);
        target.draw(miniQuads);
      }
      if (miniBorderCount > 0) {
        miniBorders.resize(miniBorderCount);
        target.draw(miniBorders);
      }

      if (hasExitMarker) {
        float crossSz =
            std::max(2.5f, std::min(tileScaledW, tileScaledH) * 0.75f);
        if (isExitDisabled) {
          sf::RectangleShape d1({crossSz, 1.2f});
          d1.setOrigin({crossSz * 0.5f, 0.6f});
          d1.setPosition(exitTileCenter);
          d1.setRotation(sf::degrees(45.0f));
          d1.setFillColor(sf::Color(220, 65, 65, 230));
          target.draw(d1);

          sf::RectangleShape d2({crossSz, 1.2f});
          d2.setOrigin({crossSz * 0.5f, 0.6f});
          d2.setPosition(exitTileCenter);
          d2.setRotation(sf::degrees(-45.0f));
          d2.setFillColor(sf::Color(220, 65, 65, 230));
          target.draw(d2);
        } else {
          sf::RectangleShape p1({crossSz, 1.2f});
          p1.setOrigin({crossSz * 0.5f, 0.6f});
          p1.setPosition(exitTileCenter);
          p1.setFillColor(sf::Color(255, 255, 255, 240));
          target.draw(p1);

          sf::RectangleShape p2({1.2f, crossSz});
          p2.setOrigin({0.6f, crossSz * 0.5f});
          p2.setPosition(exitTileCenter);
          p2.setFillColor(sf::Color(255, 255, 255, 240));
          target.draw(p2);
        }
      }

      // 3. Draw miniature scrap items
      for (const auto &item : maze.getItems()) {
        bool collected =
            (std::find(collectedItemIds.begin(), collectedItemIds.end(),
                       item.id) != collectedItemIds.end());
        if (collected)
          continue;

        float icx = (item.position.x + 0.5f) * ProceduralRenderer::TILE_SIZE;
        float icy = (item.position.y + 0.5f) * ProceduralRenderer::TILE_SIZE;
        if (scanRadius >= 0.0f &&
            std::hypot(icx - scanOrigin.x, icy - scanOrigin.y) >
                scanRadius + 14.0f) {
          continue; // Unscanned item hidden on minimap
        }

        float ix = l.offsetX + (item.position.x + 0.5f) * tileScaledW;
        float iy = l.offsetY + (item.position.y + 0.5f) * tileScaledH;
        float itemRadius =
            std::max(2.0f, std::min(tileScaledW, tileScaledH) * 0.45f);

        sf::CircleShape miniItem(itemRadius);
        miniItem.setOrigin({itemRadius, itemRadius});
        miniItem.setPosition({ix, iy});

        switch (item.category) {
        case core::ItemCategory::LightGem:
          miniItem.setFillColor(sf::Color(0, 230, 255));
          break;
        case core::ItemCategory::MediumRelic:
          miniItem.setFillColor(sf::Color(255, 215, 0));
          break;
        case core::ItemCategory::HeavyArtifact:
          miniItem.setFillColor(sf::Color(255, 75, 75));
          break;
        }
        target.draw(miniItem);
      }

      // 4. Draw miniature route line (animated tracing up to routeProgress)
      if (routePlan && routePlan->fullPath.size() >= 2 &&
          routeProgress > 0.001f) {
        float totalDist = 0.0f;
        std::vector<float> segLens;
        segLens.reserve(routePlan->fullPath.size() - 1);
        for (size_t i = 0; i + 1 < routePlan->fullPath.size(); ++i) {
          float dx = static_cast<float>(routePlan->fullPath[i + 1].x -
                                        routePlan->fullPath[i].x);
          float dy = static_cast<float>(routePlan->fullPath[i + 1].y -
                                        routePlan->fullPath[i].y);
          float d = std::hypot(dx, dy);
          segLens.push_back(d);
          totalDist += d;
        }

        if (totalDist > 0.001f) {
          float targetDist = totalDist * std::clamp(routeProgress, 0.0f, 1.0f);
          float accDist = 0.0f;

          std::vector<sf::Vector2f> activePts;
          activePts.reserve(routePlan->fullPath.size());
          activePts.push_back(
              {l.offsetX + (routePlan->fullPath[0].x + 0.5f) * tileScaledW,
               l.offsetY + (routePlan->fullPath[0].y + 0.5f) * tileScaledH});

          for (size_t i = 0; i < segLens.size(); ++i) {
            float p1x =
                l.offsetX + (routePlan->fullPath[i].x + 0.5f) * tileScaledW;
            float p1y =
                l.offsetY + (routePlan->fullPath[i].y + 0.5f) * tileScaledH;
            float p2x =
                l.offsetX + (routePlan->fullPath[i + 1].x + 0.5f) * tileScaledW;
            float p2y =
                l.offsetY + (routePlan->fullPath[i + 1].y + 0.5f) * tileScaledH;

            if (accDist + segLens[i] <= targetDist) {
              activePts.push_back({p2x, p2y});
              accDist += segLens[i];
            } else {
              float remain = targetDist - accDist;
              float frac = (segLens[i] > 0.001f) ? (remain / segLens[i]) : 0.0f;
              activePts.push_back(
                  {p1x + (p2x - p1x) * frac, p1y + (p2y - p1y) * frac});
              break;
            }
          }

          if (activePts.size() >= 2) {
            sf::VertexArray va(sf::PrimitiveType::LineStrip);
            va.resize(activePts.size());
            for (size_t i = 0; i < activePts.size(); ++i) {
              va[i] = {activePts[i], sf::Color(0, 240, 255, 230)};
            }
            target.draw(va);
          }
        }
      }

      if (routeProgress >= 0.999f) {
        if (!wasRouteCompleted_) {
          wasRouteCompleted_ = true;
          routeCompletionTimer_ = 1.0f;
        }
      } else if (routeProgress < 0.1f) {
        wasRouteCompleted_ = false;
      }

      // 4b. Draw miniature entrance and exit destination highlights on minimap
      if (routeCompletionTimer_ > 0.0f) {
        float tNorm = 1.0f - (routeCompletionTimer_ / 1.0f);
        float fadeAlpha =
            (tNorm < 0.12f) ? (tNorm / 0.12f) : ((1.0f - tNorm) / 0.88f);
        fadeAlpha = std::clamp(fadeAlpha, 0.0f, 1.0f);

        core::Point startPt = maze.getStartPoint();
        core::Point exitPt = maze.getExitPoint();
        bool isSamePoint =
            (startPt.x == exitPt.x && startPt.y == exitPt.y) || isExitDisabled;

        float startX = l.offsetX + (startPt.x + 0.5f) * tileScaledW;
        float startY = l.offsetY + (startPt.y + 0.5f) * tileScaledH;

        if (isSamePoint) {
          float rEmer = 4.0f + tNorm * 18.0f * l.mapScaleX;
          sf::CircleShape emerRing(rEmer);
          emerRing.setOrigin({rEmer, rEmer});
          emerRing.setPosition({startX, startY});
          emerRing.setFillColor(sf::Color::Transparent);
          emerRing.setOutlineThickness(1.8f);
          emerRing.setOutlineColor(sf::Color(
              60, 255, 160,
              static_cast<uint8_t>(240.0f * fadeAlpha * (1.0f - tNorm))));
          target.draw(emerRing);

          float rGold = 3.0f + tNorm * 14.0f * l.mapScaleX;
          sf::CircleShape goldRing(rGold);
          goldRing.setOrigin({rGold, rGold});
          goldRing.setPosition({startX, startY});
          goldRing.setFillColor(sf::Color::Transparent);
          goldRing.setOutlineThickness(1.6f);
          goldRing.setOutlineColor(sf::Color(
              255, 220, 60,
              static_cast<uint8_t>(230.0f * fadeAlpha * (1.0f - tNorm))));
          target.draw(goldRing);
        } else {
          float exitX = l.offsetX + (exitPt.x + 0.5f) * tileScaledW;
          float exitY = l.offsetY + (exitPt.y + 0.5f) * tileScaledH;

          float rStart = 3.0f + tNorm * 14.0f * l.mapScaleX;
          sf::CircleShape goldRing(rStart);
          goldRing.setOrigin({rStart, rStart});
          goldRing.setPosition({startX, startY});
          goldRing.setFillColor(sf::Color::Transparent);
          goldRing.setOutlineThickness(1.8f);
          goldRing.setOutlineColor(sf::Color(
              255, 220, 60,
              static_cast<uint8_t>(230.0f * fadeAlpha * (1.0f - tNorm))));
          target.draw(goldRing);

          float rExit = 3.0f + tNorm * 16.0f * l.mapScaleX;
          sf::CircleShape emerRing(rExit);
          emerRing.setOrigin({rExit, rExit});
          emerRing.setPosition({exitX, exitY});
          emerRing.setFillColor(sf::Color::Transparent);
          emerRing.setOutlineThickness(2.0f);
          emerRing.setOutlineColor(sf::Color(
              60, 255, 160,
              static_cast<uint8_t>(240.0f * fadeAlpha * (1.0f - tNorm))));
          target.draw(emerRing);
        }
      }

      // 5. Draw miniature agent (High-visibility tactical GPS beacon) -
      // revealed when scanned
      bool isAgentScanned = (scanRadius < 0.0f);
      float ax, ay;
      if (visualAgentWorldPos.x >= 0.0f && visualAgentWorldPos.y >= 0.0f) {
        ax = l.offsetX +
             (visualAgentWorldPos.x / ProceduralRenderer::TILE_SIZE) *
                 tileScaledW;
        ay = l.offsetY +
             (visualAgentWorldPos.y / ProceduralRenderer::TILE_SIZE) *
                 tileScaledH;
        if (!isAgentScanned &&
            std::hypot(visualAgentWorldPos.x - scanOrigin.x,
                       visualAgentWorldPos.y - scanOrigin.y) <=
                scanRadius + 6.0f) {
          isAgentScanned = true;
        }
      } else {
        ax = l.offsetX + (agent.getPosition().x + 0.5f) * tileScaledW;
        ay = l.offsetY + (agent.getPosition().y + 0.5f) * tileScaledH;
        float worldAx =
            (agent.getPosition().x + 0.5f) * ProceduralRenderer::TILE_SIZE;
        float worldAy =
            (agent.getPosition().y + 0.5f) * ProceduralRenderer::TILE_SIZE;
        if (!isAgentScanned &&
            std::hypot(worldAx - scanOrigin.x, worldAy - scanOrigin.y) <=
                scanRadius + 6.0f) {
          isAgentScanned = true;
        }
      }

      if (isAgentScanned) {
        float agentRadius =
            std::max(5.5f, std::min(tileScaledW, tileScaledH) * 1.0f);

        // Outer glowing pulse ring
        sf::CircleShape pulseRing(agentRadius + 3.5f);
        pulseRing.setOrigin({agentRadius + 3.5f, agentRadius + 3.5f});
        pulseRing.setPosition({ax, ay});
        pulseRing.setFillColor(sf::Color(255, 140, 0, 75));
        pulseRing.setOutlineThickness(1.5f);
        pulseRing.setOutlineColor(sf::Color(0, 255, 230, 230));
        target.draw(pulseRing);

        // Core agent body
        sf::CircleShape miniAgent(agentRadius);
        miniAgent.setOrigin({agentRadius, agentRadius});
        miniAgent.setPosition({ax, ay});
        miniAgent.setFillColor(sf::Color(255, 190, 20));
        miniAgent.setOutlineThickness(1.8f);
        miniAgent.setOutlineColor(sf::Color(255, 255, 255, 255));
        target.draw(miniAgent);

        // Center white beacon dot
        sf::CircleShape centerDot(1.8f);
        centerDot.setOrigin({1.8f, 1.8f});
        centerDot.setPosition({ax, ay});
        centerDot.setFillColor(sf::Color(255, 255, 255));
        target.draw(centerDot);
      }

      // 6. Camera Frustum Box: Clamped strictly to visible maze area
      sf::Vector2f viewCenter = cameraView.getCenter();
      sf::Vector2f viewSize = cameraView.getSize();

      float left = viewCenter.x - viewSize.x * 0.5f;
      float top = viewCenter.y - viewSize.y * 0.5f;
      float right = viewCenter.x + viewSize.x * 0.5f;
      float bottom = viewCenter.y + viewSize.y * 0.5f;

      float visL = std::clamp(left, 0.0f, mazeW);
      float visT = std::clamp(top, 0.0f, mazeH);
      float visR = std::clamp(right, 0.0f, mazeW);
      float visB = std::clamp(bottom, 0.0f, mazeH);

      if (visR > visL && visB > visT) {
        float frustumX = l.offsetX + visL * l.mapScaleX;
        float frustumY = l.offsetY + visT * l.mapScaleY;
        float frustumW = (visR - visL) * l.mapScaleX;
        float frustH = (visB - visT) * l.mapScaleY;

        // Outer subtle glow shadow
        sf::RectangleShape frustumGlow({frustumW, frustH});
        frustumGlow.setPosition({frustumX, frustumY});
        frustumGlow.setFillColor(sf::Color::Transparent);
        frustumGlow.setOutlineThickness(3.0f);
        frustumGlow.setOutlineColor(sf::Color(0, 230, 245, 55));
        target.draw(frustumGlow);

        // Core camera frustum rectangle
        sf::RectangleShape frustum({frustumW, frustH});
        frustum.setPosition({frustumX, frustumY});
        frustum.setFillColor(sf::Color(0, 255, 230, 30));
        frustum.setOutlineThickness(1.8f);
        frustum.setOutlineColor(sf::Color(0, 255, 230, 245));
        target.draw(frustum);

        // Center camera crosshairs
        float centerX =
            l.offsetX + std::clamp(viewCenter.x, 0.0f, mazeW) * l.mapScaleX;
        float centerY =
            l.offsetY + std::clamp(viewCenter.y, 0.0f, mazeH) * l.mapScaleY;
        sf::Color crosshairColor(0, 255, 230, 120);

        sf::RectangleShape hLine({frustumW, 1.0f});
        hLine.setPosition({frustumX, centerY - 0.5f});
        hLine.setFillColor(crosshairColor);
        target.draw(hLine);

        sf::RectangleShape vLine({1.0f, frustH});
        vLine.setPosition({centerX - 0.5f, frustumY});
        vLine.setFillColor(crosshairColor);
        target.draw(vLine);
      }

      // 7. Miniature radar pulse circle on minimap during sonar scanning
      if (scanRadius >= 0.0f) {
        float originMiniX = l.offsetX + scanOrigin.x * l.mapScaleX;
        float originMiniY = l.offsetY + scanOrigin.y * l.mapScaleY;
        float miniRY = scanRadius * l.mapScaleY;
        float maxMiniR = std::hypot(l.contentW * 0.5f, l.contentH * 0.5f);

        float alphaMult = 1.0f;
        if (miniRY > maxMiniR * 0.85f) {
          alphaMult = std::clamp(
              1.0f - (miniRY - maxMiniR * 0.85f) / (maxMiniR * 0.25f), 0.0f,
              1.0f);
        }

        if (alphaMult > 0.01f) {
          sf::CircleShape miniPulse(miniRY);
          if (l.mapScaleY > 0.0001f) {
            miniPulse.setScale({l.mapScaleX / l.mapScaleY, 1.0f});
          }
          miniPulse.setOrigin({miniRY, miniRY});
          miniPulse.setPosition({originMiniX, originMiniY});
          miniPulse.setFillColor(sf::Color::Transparent);
          miniPulse.setOutlineThickness(1.4f);
          miniPulse.setOutlineColor(
              sf::Color(0, 240, 255, static_cast<uint8_t>(210.0f * alphaMult)));
          target.draw(miniPulse);
        }
      }

      // Switch back to apertureView for CRT scan beam and flash overlays
      target.setView(apertureView);

      if (vScale < 0.98f) {
        float beamH = std::max(2.5f, 6.0f * (1.0f - vScale));
        float beamW = l.screenW * hScale;
        float beamX = l.screenX + (l.screenW - beamW) * 0.5f;
        float beamY = l.screenY + (l.screenH - beamH) * 0.5f;
        sf::RectangleShape beam({beamW, beamH});
        beam.setPosition({beamX, beamY});
        beam.setFillColor(
            sf::Color(220, 255, 255,
                      static_cast<uint8_t>(255 * (1.0f - vScale * 0.25f))));
        target.draw(beam);
      }

      if (flashAlpha > 0.01f) {
        sf::RectangleShape flashCover({l.screenW, l.screenH});
        flashCover.setPosition({l.screenX, l.screenY});
        flashCover.setFillColor(sf::Color(
            100, 230, 255, static_cast<uint8_t>(flashAlpha * 255.0f)));
        target.draw(flashCover);
      }
    }

    // 8. Dark blue fade-out overlay inside the monitor hole
    if (fadeAlpha > 0.0f && powerState_ != PowerState::Off) {
      target.setView(apertureView);
      sf::RectangleShape fadeCover({l.screenW, l.screenH});
      fadeCover.setPosition({l.screenX, l.screenY});
      fadeCover.setFillColor(
          sf::Color(22, 27, 36, static_cast<uint8_t>(fadeAlpha * 255.0f)));
      target.draw(fadeCover);
    }

    // Restore target view before rendering the monitor frame casing
    target.setView(prevView);

    // 9. Draw the authentic Lethal Company monitor frame sprite on top!
    if (isDragging_) {
      if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        isDragging_ = false;
      } else {
        activeGlow_ = 1.0f;
      }
    }
    if (!isDragging_) {
      activeGlow_ = std::max(0.0f, activeGlow_ - dt * 3.5f);
    }

    if (hasFrameTexture_) {
      sf::Sprite frameSprite(frameTexture_);
      frameSprite.setScale({l.sX, l.sY});
      frameSprite.setPosition({l.frameX, l.frameY});
      target.draw(frameSprite);

      // Authentic rounded bezel corner masks: guaranteed to cover and round off
      // any map corners behind the monitor frame
      const float cornerR = 26.0f * l.s;
      const sf::Color bezelColor(26, 21, 17, 255);
      constexpr int arcSegments = 8;
      constexpr float PI_VAL = 3.14159265f;

      auto drawCornerFillet = [&](float cx, float cy, float ox, float oy,
                                  float startAngleRad) {
        sf::VertexArray fillet(sf::PrimitiveType::TriangleFan);
        fillet.append({{ox, oy}, bezelColor});
        for (int i = 0; i <= arcSegments; ++i) {
          float theta = startAngleRad +
                        (static_cast<float>(i) / arcSegments) * (PI_VAL * 0.5f);
          float px = cx + cornerR * std::cos(theta);
          float py = cy + cornerR * std::sin(theta);
          fillet.append({{px, py}, bezelColor});
        }
        target.draw(fillet);
      };

      // Top-Left corner
      drawCornerFillet(l.screenX + cornerR, l.screenY + cornerR, l.screenX,
                       l.screenY, PI_VAL);
      // Top-Right corner
      drawCornerFillet(l.screenX + l.screenW - cornerR, l.screenY + cornerR,
                       l.screenX + l.screenW, l.screenY, PI_VAL * 1.5f);
      // Bottom-Right corner
      drawCornerFillet(l.screenX + l.screenW - cornerR,
                       l.screenY + l.screenH - cornerR, l.screenX + l.screenW,
                       l.screenY + l.screenH, 0.0f);
      // Bottom-Left corner
      drawCornerFillet(l.screenX + cornerR, l.screenY + l.screenH - cornerR,
                       l.screenX, l.screenY + l.screenH, PI_VAL * 0.5f);

      // Active dragging cyan border rendered OVER monitor photo with rounded
      // corners matching the screen bezel
      if (activeGlow_ > 0.02f && powerState_ == PowerState::On) {
        const float thick = 2.0f;
        const float rOut = cornerR;
        const float rIn = std::max(0.0f, cornerR - thick);
        const uint8_t glowAlpha = static_cast<uint8_t>(
            std::clamp(175.0f * activeGlow_, 0.0f, 255.0f));
        const sf::Color glowColor(0, 245, 255, glowAlpha);

        sf::VertexArray roundedBorder(sf::PrimitiveType::TriangleStrip);

        auto appendArc = [&](float cx, float cy, float startAngle,
                             float sweepAngle) {
          for (int i = 0; i <= arcSegments; ++i) {
            float angle =
                startAngle + sweepAngle * (static_cast<float>(i) / arcSegments);
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);
            roundedBorder.append(
                {{cx + rOut * cosA, cy + rOut * sinA}, glowColor});
            roundedBorder.append(
                {{cx + rIn * cosA, cy + rIn * sinA}, glowColor});
          }
        };

        // 1. Top-Left corner (180° -> 270°)
        appendArc(l.screenX + cornerR, l.screenY + cornerR, PI_VAL,
                  PI_VAL * 0.5f);
        // 2. Top-Right corner (270° -> 360°)
        appendArc(l.screenX + l.screenW - cornerR, l.screenY + cornerR,
                  PI_VAL * 1.5f, PI_VAL * 0.5f);
        // 3. Bottom-Right corner (0° -> 90°)
        appendArc(l.screenX + l.screenW - cornerR,
                  l.screenY + l.screenH - cornerR, 0.0f, PI_VAL * 0.5f);
        // 4. Bottom-Left corner (90° -> 180°)
        appendArc(l.screenX + cornerR, l.screenY + l.screenH - cornerR,
                  PI_VAL * 0.5f, PI_VAL * 0.5f);

        // Close the loop with the initial point pair
        float initCos = std::cos(PI_VAL);
        float initSin = std::sin(PI_VAL);
        float initCx = l.screenX + cornerR;
        float initCy = l.screenY + cornerR;
        roundedBorder.append(
            {{initCx + rOut * initCos, initCy + rOut * initSin}, glowColor});
        roundedBorder.append(
            {{initCx + rIn * initCos, initCy + rIn * initSin}, glowColor});

        target.draw(roundedBorder);
      }
    } else {
      // Fallback outline if texture failed to load
      const sf::Color defaultBorder(60, 75, 96, 255);
      const sf::Color cyanBorder(0, 230, 245, 255);
      sf::Color currentBorder(
          static_cast<uint8_t>(defaultBorder.r +
                               (cyanBorder.r - defaultBorder.r) * activeGlow_),
          static_cast<uint8_t>(defaultBorder.g +
                               (cyanBorder.g - defaultBorder.g) * activeGlow_),
          static_cast<uint8_t>(defaultBorder.b +
                               (cyanBorder.b - defaultBorder.b) * activeGlow_),
          255);

      sf::RectangleShape borderFrame({l.screenW, l.screenH});
      borderFrame.setPosition({l.screenX, l.screenY});
      borderFrame.setFillColor(sf::Color::Transparent);
      borderFrame.setOutlineThickness(2.0f + 0.6f * activeGlow_);
      borderFrame.setOutlineColor(currentBorder);
      target.draw(borderFrame);
    }

    // 10. Power LED indicator on bezel next to button
    float ledX = l.frameX + LED_X * l.s;
    float ledY = l.frameY + LED_Y * l.s;
    float ledR = 4.2f * l.s;

    if (powerState_ == PowerState::On || powerState_ == PowerState::TurningOn) {
      // Soft outer green glow
      sf::CircleShape ledGlow(ledR * 2.2f);
      ledGlow.setOrigin({ledR * 2.2f, ledR * 2.2f});
      ledGlow.setPosition({ledX, ledY});
      ledGlow.setFillColor(sf::Color(40, 255, 75, 80));
      target.draw(ledGlow);

      // Core bright green LED
      sf::CircleShape led(ledR);
      led.setOrigin({ledR, ledR});
      led.setPosition({ledX, ledY});
      led.setFillColor(sf::Color(65, 255, 95));
      target.draw(led);

      // Specular highlight
      sf::CircleShape ledCore(ledR * 0.45f);
      ledCore.setOrigin({ledR * 0.45f, ledR * 0.45f});
      ledCore.setPosition({ledX - ledR * 0.15f, ledY - ledR * 0.15f});
      ledCore.setFillColor(sf::Color(220, 255, 230));
      target.draw(ledCore);
    } else {
      // Powered off: dark red/unlit diode
      sf::CircleShape led(ledR * 0.85f);
      led.setOrigin({ledR * 0.85f, ledR * 0.85f});
      led.setPosition({ledX, ledY});
      led.setFillColor(sf::Color(135, 25, 25, 210));
      target.draw(led);
    }

    // 11. Power Button tactile feedback / hover
    float btnX = l.frameX + BTN_X * l.s;
    float btnY = l.frameY + BTN_Y * l.s;
    float btnW = BTN_W * l.s;
    float btnH = BTN_H * l.s;

    if (buttonPressVisual_ > 0.01f) {
      sf::RectangleShape pressTint({btnW, btnH});
      pressTint.setPosition({btnX, btnY});
      pressTint.setFillColor(sf::Color(
          0, 0, 0, static_cast<uint8_t>(buttonPressVisual_ * 110.0f)));
      target.draw(pressTint);
      buttonPressVisual_ = std::max(0.0f, buttonPressVisual_ - dt * 6.0f);
    } else if (isButtonHovered_) {
      sf::RectangleShape hoverGlow({btnW, btnH});
      hoverGlow.setPosition({btnX, btnY});
      hoverGlow.setFillColor(sf::Color(0, 230, 245, 35));
      hoverGlow.setOutlineThickness(1.2f);
      hoverGlow.setOutlineColor(sf::Color(0, 255, 240, 130));
      target.draw(hoverGlow);
    }
  }

  bool handleEvent(const sf::Event &event, float windowWidth,
                   float windowHeight, const map::Maze &maze,
                   sf::Vector2f &outWorldTarget, bool &outIsDragging,
                   float &outZoomFactor, float scaleOrSize = 1.0f) {
    float mazeW = maze.getWidth() * ProceduralRenderer::TILE_SIZE;
    float mazeH = maze.getHeight() * ProceduralRenderer::TILE_SIZE;
    Layout l =
        calculateLayout(windowWidth, windowHeight, scaleOrSize, mazeW, mazeH);

    outZoomFactor = 1.0f;

    if (const auto *mouseMove = event.getIf<sf::Event::MouseMoved>()) {
      isButtonHovered_ =
          isOverPowerButton(mouseMove->position.x, mouseMove->position.y, l);
    }

    // Power button click on LMB
    if (const auto *mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
      if (mouseBtn->button == sf::Mouse::Button::Left) {
        if (isOverPowerButton(mouseBtn->position.x, mouseBtn->position.y, l)) {
          togglePower();
          buttonPressVisual_ = 1.0f;
          isDragging_ = false;
          outIsDragging = false;
          return false; // Handled power button, do not pan world camera!
        }
      }
    }

    // If monitor is turned off or in the middle of powering off/on, ignore map interaction
    if (powerState_ != PowerState::On) {
      isDragging_ = false;
      outIsDragging = false;
      return false;
    }

    // 1. Mouse wheel zoom on minimap -> zooms general labyrinth camera anchored to hovered position ONLY inside maze bounds!
    if (const auto *wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
      if (isInsideMazeContent(wheel->position.x, wheel->position.y, l)) {
        convertPixelToWorld(wheel->position, windowWidth, windowHeight, maze,
                            outWorldTarget, scaleOrSize);
        outZoomFactor = (wheel->delta > 0) ? 0.85f : 1.18f;
        outIsDragging = false;
        return true;
      }
    }

    // 2. LMB press on minimap -> starts dragging general labyrinth camera
    if (const auto *mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
      if (mouseBtn->button == sf::Mouse::Button::Left &&
          isInsideScreen(mouseBtn->position.x, mouseBtn->position.y, l)) {
        isDragging_ = true;
        activeGlow_ = 1.0f;
        outIsDragging = false;
        outZoomFactor = 1.0f;
        return convertPixelToWorld(mouseBtn->position, windowWidth,
                                   windowHeight, maze, outWorldTarget,
                                   scaleOrSize);
      }
    }

    // 3. LMB release -> stops dragging
    if (const auto *mouseBtn = event.getIf<sf::Event::MouseButtonReleased>()) {
      if (mouseBtn->button == sf::Mouse::Button::Left ||
          mouseBtn->button == sf::Mouse::Button::Right) {
        isDragging_ = false;
      }
    }

    // 4. Mouse move while LMB dragging on minimap -> drags general labyrinth camera
    if (const auto *mouseMove = event.getIf<sf::Event::MouseMoved>()) {
      if (isDragging_) {
        outIsDragging = true;
        outZoomFactor = 1.0f;
        return convertPixelToWorld(mouseMove->position, windowWidth,
                                   windowHeight, maze, outWorldTarget,
                                   scaleOrSize);
      }
    }

    return false;
  }

  bool handleEvent(const sf::Event &event, float windowWidth,
                   float windowHeight, const map::Maze &maze,
                   sf::Vector2f &outWorldTarget, bool &outIsDragging,
                   float scaleOrSize = 1.0f) {
    float dummyZoom = 1.0f;
    return handleEvent(event, windowWidth, windowHeight, maze, outWorldTarget,
                       outIsDragging, dummyZoom, scaleOrSize);
  }

  bool handleEvent(const sf::Event &event, float windowWidth,
                   float windowHeight, const map::Maze &maze,
                   sf::Vector2f &outWorldTarget, float scaleOrSize = 1.0f) {
    bool unused = false;
    float dummyZoom = 1.0f;
    return handleEvent(event, windowWidth, windowHeight, maze, outWorldTarget,
                       unused, dummyZoom, scaleOrSize);
  }

  [[nodiscard]] bool isDragging() const noexcept { return isDragging_; }

private:
  bool convertPixelToWorld(const sf::Vector2i &mousePixel, float windowWidth,
                           float windowHeight, const map::Maze &maze,
                           sf::Vector2f &outWorldTarget,
                           float scaleOrSize = 1.0f) const {
    float mazeW = maze.getWidth() * ProceduralRenderer::TILE_SIZE;
    float mazeH = maze.getHeight() * ProceduralRenderer::TILE_SIZE;
    Layout l =
        calculateLayout(windowWidth, windowHeight, scaleOrSize, mazeW, mazeH);

    float clickRelX =
        (static_cast<float>(mousePixel.x) - l.offsetX) / l.mapScaleX;
    float clickRelY =
        (static_cast<float>(mousePixel.y) - l.offsetY) / l.mapScaleY;

    outWorldTarget = {std::clamp(clickRelX, 0.0f, mazeW),
                      std::clamp(clickRelY, 0.0f, mazeH)};
    return true;
  }

  sf::Texture frameTexture_;
  bool hasFrameTexture_ = false;
  bool isDragging_ = false;
  float activeGlow_ = 0.0f;
  sf::Clock glowClock_;
  PowerState powerState_ = PowerState::On;
  float powerAnimProgress_ = 1.0f;
  bool isButtonHovered_ = false;
  float buttonPressVisual_ = 0.0f;
  float routeCompletionTimer_ = 0.0f;
  bool wasRouteCompleted_ = false;
  sf::VertexArray minimapQuads_{sf::PrimitiveType::Triangles};
  sf::VertexArray minimapBorders_{sf::PrimitiveType::Lines};
};

} // namespace ui
