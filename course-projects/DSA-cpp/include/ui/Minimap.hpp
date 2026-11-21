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
  // Transparent padding + anti-aliased bevel edge on left side of map.png
  static constexpr float FRAME_LEFT_INSET = 25.0f;
  static constexpr float HOLE_LEFT = 145.0f;
  static constexpr float HOLE_TOP = 142.0f;
  static constexpr float HOLE_WIDTH = 1415.0f;
  static constexpr float HOLE_HEIGHT = 1180.0f;

  // Monitor hardware power button and indicator LED on map.png bezel
  static constexpr float BTN_X = 1380.0f;
  static constexpr float BTN_Y = 1455.0f;
  static constexpr float BTN_W = 86.0f;
  static constexpr float BTN_H = 86.0f;
  static constexpr float LED_X = 1349.5f;
  static constexpr float LED_Y = 1492.5f;

  // Monitor hardware tactical layers toggle switch and indicator LED on bezel
  static constexpr float TOGGLE_SW_X = 1020.0f;
  static constexpr float TOGGLE_SW_Y = 1445.0f;
  static constexpr float TOGGLE_SW_W = 44.0f;
  static constexpr float TOGGLE_SW_H = 90.0f;
  static constexpr float TOGGLE_LED_X = 1091.5f;
  static constexpr float TOGGLE_LED_UPPER_Y = 1463.0f;
  static constexpr float TOGGLE_LED_LOWER_Y = 1508.0f;

  // Monitor hardware zoom slider on bezel
  static constexpr float SLIDER_MIN_X = 222.0f;
  static constexpr float SLIDER_MAX_X = 761.0f;
  static constexpr float SLIDER_Y = 1489.0f;
  static constexpr float SLIDER_KNOB_W = 34.0f;
  static constexpr float SLIDER_KNOB_H = 62.0f;

  // Monitor hardware overview dial button on bezel (Fit whole maze)
  static constexpr float DIAL_BTN_X = 895.0f;
  static constexpr float DIAL_BTN_Y = 1488.0f;
  static constexpr float DIAL_BTN_R = 44.0f;

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

  static float getVisualLeftInset(float scaleOrSize = 1.0f) noexcept {
    return FRAME_LEFT_INSET * getFrameScale(scaleOrSize);
  }

  static Layout calculateLayout(float windowWidth, float windowHeight,
                                float scaleOrSize, float mazeW,
                                float mazeH, float offsetY = 0.0f) noexcept {
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
    l.frameY = windowHeight - l.frameH - PADDING + offsetY;

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

  void setPoweredOffImmediate() noexcept {
    powerState_ = PowerState::Off;
    powerAnimProgress_ = 0.0f;
  }

  void setPoweredOnImmediate() noexcept {
    powerState_ = PowerState::On;
    powerAnimProgress_ = 1.0f;
  }

  void powerOn() noexcept {
    powerState_ = PowerState::TurningOn;
    powerAnimProgress_ = 0.0f;
  }

  [[nodiscard]] bool isPoweredOn() const noexcept {
    return powerState_ == PowerState::On;
  }

  [[nodiscard]] bool isPoweringOn() const noexcept {
    return powerState_ == PowerState::TurningOn;
  }

  [[nodiscard]] bool isPowerButtonHovered() const noexcept {
    return isButtonHovered_;
  }

  [[nodiscard]] bool isToggleSwitchHovered() const noexcept {
    return isSwitchHovered_;
  }

  [[nodiscard]] bool isZoomSliderHovered() const noexcept {
    return isZoomSliderHovered_;
  }

  [[nodiscard]] bool isZoomSliderDragging() const noexcept {
    return isZoomSliderDragging_;
  }

  void setCameraZoom(float zoom) noexcept {
    currentCameraZoom_ = zoom;
  }

  [[nodiscard]] bool consumeRequestedCameraZoom(float &outZoom) noexcept {
    if (hasRequestedCameraZoom_) {
      hasRequestedCameraZoom_ = false;
      outZoom = requestedCameraZoom_;
      return true;
    }
    return false;
  }

  [[nodiscard]] bool isOverZoomSliderKnob(int px, int py,
                                          const Layout &l) const noexcept {
    float knobTexX = SLIDER_MIN_X + sliderVisualT_ * (SLIDER_MAX_X - SLIDER_MIN_X);
    float knobScreenX = l.frameX + knobTexX * l.s;
    float knobScreenY = l.frameY + SLIDER_Y * l.s;
    float halfW = (SLIDER_KNOB_W * 0.5f + 9.0f) * l.s;
    float halfH = (SLIDER_KNOB_H * 0.5f + 9.0f) * l.s;
    float x = static_cast<float>(px);
    float y = static_cast<float>(py);
    return x >= (knobScreenX - halfW) && x <= (knobScreenX + halfW) &&
           y >= (knobScreenY - halfH) && y <= (knobScreenY + halfH);
  }

  [[nodiscard]] bool isOverZoomSlider(int px, int py,
                                      const Layout &l) const noexcept {
    return isOverZoomSliderKnob(px, py, l);
  }

  [[nodiscard]] float calculateZoomFromMouseX(float mouseX,
                                              const Layout &l) const noexcept {
    float minX = l.frameX + SLIDER_MIN_X * l.s;
    float maxX = l.frameX + SLIDER_MAX_X * l.s;
    float trackW = maxX - minX;
    if (trackW <= 1.0f) {
      return currentCameraZoom_;
    }
    float t = std::clamp((mouseX - minX) / trackW, 0.0f, 1.0f);
    constexpr float logMin = -1.6094379f; // std::log(0.20f)
    constexpr float logMax = 1.5040774f;  // std::log(4.50f)
    float targetMag = std::exp(logMin + t * (logMax - logMin));
    return 1.0f / targetMag;
  }

  [[nodiscard]] bool isDialButtonHovered() const noexcept {
    return isDialButtonHovered_;
  }

  [[nodiscard]] bool consumeFitMazeRequest() noexcept {
    bool r = requestFitMaze_;
    requestFitMaze_ = false;
    return r;
  }

  [[nodiscard]] bool isOverDialButton(int px, int py,
                                      const Layout &l) const noexcept {
    float dialX = l.frameX + DIAL_BTN_X * l.s;
    float dialY = l.frameY + DIAL_BTN_Y * l.s;
    float dialR = (DIAL_BTN_R + 4.0f) * l.s;
    float dx = static_cast<float>(px) - dialX;
    float dy = static_cast<float>(py) - dialY;
    return (dx * dx + dy * dy) <= (dialR * dialR);
  }

  [[nodiscard]] bool consumeTacticalLayersToggle() noexcept {
    bool r = requestToggleTacticalLayers_;
    requestToggleTacticalLayers_ = false;
    return r;
  }

  void setTacticalLayersActive(bool active) noexcept {
    areTacticalLayersActive_ = active;
  }

  [[nodiscard]] bool isOverToggleSwitch(int px, int py,
                                        const Layout &l) const noexcept {
    float swX = l.frameX + TOGGLE_SW_X * l.s;
    float swY = l.frameY + TOGGLE_SW_Y * l.s;
    float swW = TOGGLE_SW_W * l.s;
    float swH = TOGGLE_SW_H * l.s;
    float m = 6.0f * l.s;
    return px >= swX - m && px <= swX + swW + m && py >= swY - m &&
           py <= swY + swH + m;
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

  void setIntroOffsetY(float offsetY) noexcept { introOffsetY_ = offsetY; }

  [[nodiscard]] bool isMouseOver(int px, int py, float windowWidth,
                                 float windowHeight,
                                 float scaleOrSize = 1.0f) const noexcept {
    Layout l = calculateLayout(windowWidth, windowHeight, scaleOrSize, 100.0f,
                               100.0f, introOffsetY_);
    return isOverFrame(px, py, l);
  }

  [[nodiscard]] bool isInsideMinimap(int px, int py, float windowWidth,
                                     float windowHeight,
                                     float scaleOrSize = 1.0f) const noexcept {
    Layout l = calculateLayout(windowWidth, windowHeight, scaleOrSize, 100.0f,
                               100.0f, introOffsetY_);
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
    if (!hasKnobTexture_) {
      hasKnobTexture_ =
          knobTexture_.loadFromFile("assets/slider_knob.png") ||
          knobTexture_.loadFromFile("../assets/slider_knob.png") ||
          knobTexture_.loadFromFile("build/assets/slider_knob.png") ||
          knobTexture_.loadFromFile("course-projects/DSA-cpp/assets/slider_knob.png");
      if (hasKnobTexture_) {
        knobTexture_.setSmooth(true);
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
              float routeProgress = 1.0f, bool isExitDisabled = false,
              float monitorAlpha = 1.0f, bool showMapContent = true,
              const std::string &hoveredCatalogItemName = "",
              int hoveredCatalogCategory = -1,
              bool showWalkedCells = true) {
    if (monitorAlpha <= 0.001f) {
      return;
    }

    if (windowWidth > 1.0f && cameraView.getSize().x > 1.0f) {
      currentCameraZoom_ = cameraView.getSize().x / windowWidth;
    }
    float safeAlpha = std::clamp(monitorAlpha, 0.0f, 1.0f);
    loadTexture();

    float dt = glowClock_.restart().asSeconds();
    if (dt > 0.1f)
      dt = 0.1f;
    animTime_ += dt;

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
    Layout l = calculateLayout(windowWidth, windowHeight, scaleOrSize, mazeW,
                               mazeH, introOffsetY_);

    if (l.frameY >= windowHeight - 1.0f) {
      return;
    }

    // Hardware OpenGL clipping strictly to the visible part of the CRT screen aperture hole
    sf::View prevView = target.getView();
    sf::Vector2u winSize = target.getSize();
    float tw = static_cast<float>(winSize.x);
    float th = static_cast<float>(winSize.y);

    float visScreenTop = std::max(0.0f, l.screenY);
    float visScreenBot = std::min(th, l.screenY + l.screenH);
    bool screenVisible =
        (visScreenBot > visScreenTop + 1.0f && l.screenX < tw &&
         l.screenX + l.screenW > 0.0f);

    if (screenVisible) {
      float visScreenH = visScreenBot - visScreenTop;
      sf::View apertureView(sf::FloatRect(sf::Vector2f{l.screenX, visScreenTop},
                                          sf::Vector2f{l.screenW, visScreenH}));
      apertureView.setViewport(
          sf::FloatRect(sf::Vector2f{l.screenX / tw, visScreenTop / th},
                        sf::Vector2f{l.screenW / tw, visScreenH / th}));
      target.setView(apertureView);

    // 1. CRT Monitor inner screen background panel (dark cathode glass when On,
    // pitch black when Off)
    sf::Color screenBgColor;
    if (powerState_ == PowerState::Off) {
      screenBgColor = sf::Color(0, 0, 0, static_cast<uint8_t>(255 * safeAlpha));
    } else if (powerState_ == PowerState::On) {
      screenBgColor = sf::Color(22, 27, 36, static_cast<uint8_t>(255 * safeAlpha));
    } else {
      float t = powerAnimProgress_;
      uint8_t r = static_cast<uint8_t>(22.0f * t);
      uint8_t g = static_cast<uint8_t>(27.0f * t);
      uint8_t b = static_cast<uint8_t>(36.0f * t);
      screenBgColor = sf::Color(r, g, b, static_cast<uint8_t>(255 * safeAlpha));
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

      if (showMapContent) {
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

          if (showWalkedCells && cell.isWalked() && cell.getType() != core::CellType::Start &&
              cell.getType() != core::CellType::Exit) {
            if (cell.getType() == core::CellType::Mud) {
              baseColor = sf::Color(115, 88, 54);
              hasOutline = true;
              outlineColor = sf::Color(0, 230, 245, 200);
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

        bool isTargeted = false;
        if (!hoveredCatalogItemName.empty() && item.name == hoveredCatalogItemName) {
          isTargeted = true;
        } else if (hoveredCatalogCategory >= 0 &&
                   static_cast<int>(item.category) == hoveredCatalogCategory) {
          isTargeted = true;
        }

        if (isTargeted) {
          float pulse = 0.5f + 0.5f * std::sin(animTime_ * 8.0f);
          float haloR = itemRadius * (2.0f + 0.6f * pulse);
          sf::CircleShape halo(haloR);
          halo.setOrigin({haloR, haloR});
          halo.setPosition({ix, iy});
          halo.setFillColor(
              sf::Color(255, 255, 255, static_cast<uint8_t>(90 + 90 * pulse)));
          target.draw(halo);
        }

        sf::CircleShape miniItem(isTargeted ? (itemRadius * 1.35f) : itemRadius);
        miniItem.setOrigin({miniItem.getRadius(), miniItem.getRadius()});
        miniItem.setPosition({ix, iy});
        if (isTargeted) {
          miniItem.setOutlineThickness(1.2f);
          miniItem.setOutlineColor(sf::Color(255, 255, 255, 240));
        }

        switch (item.category) {
        case core::ItemCategory::LightGem:
          miniItem.setFillColor(sf::Color(50, 105, 255));
          break;
        case core::ItemCategory::MediumRelic:
          miniItem.setFillColor(sf::Color(255, 255, 0));
          break;
        case core::ItemCategory::HeavyArtifact:
          miniItem.setFillColor(sf::Color(255, 0, 255));
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
      } // end if (showMapContent)

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
    } // end if (screenVisible)

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
      frameSprite.setColor(sf::Color(255, 255, 255, static_cast<uint8_t>(255 * safeAlpha)));
      target.draw(frameSprite);

      // Authentic rounded bezel corner masks: guaranteed to cover and round off
      // any map corners behind the monitor frame
      const float cornerR = 26.0f * l.s;
      const sf::Color bezelColor(26, 21, 17, static_cast<uint8_t>(255 * safeAlpha));
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
          static_cast<uint8_t>(255 * safeAlpha));

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
    float ledR = 4.0f * l.s;
    float pCoverR = 7.8f * l.s;

    // Mask/cover the baked green LED from map.png with an unlit metallic socket (same as toggle indicators)
    sf::CircleShape powerCover(pCoverR);
    powerCover.setOrigin({pCoverR, pCoverR});
    powerCover.setPosition({ledX, ledY});
    powerCover.setFillColor(sf::Color(38, 52, 24, static_cast<uint8_t>(255 * safeAlpha)));
    target.draw(powerCover);

    sf::CircleShape powerRing(ledR * 1.15f);
    powerRing.setOrigin({ledR * 1.15f, ledR * 1.15f});
    powerRing.setPosition({ledX, ledY});
    powerRing.setFillColor(sf::Color(28, 36, 22, static_cast<uint8_t>(255 * safeAlpha)));
    powerRing.setOutlineThickness(1.0f * l.s);
    powerRing.setOutlineColor(sf::Color(65, 78, 50, static_cast<uint8_t>(200 * safeAlpha)));
    target.draw(powerRing);

    sf::CircleShape powerHole(ledR * 0.72f);
    powerHole.setOrigin({ledR * 0.72f, ledR * 0.72f});
    powerHole.setPosition({ledX, ledY});
    powerHole.setFillColor(sf::Color(14, 18, 12, static_cast<uint8_t>(255 * safeAlpha)));
    target.draw(powerHole);

    // Smoothly fade in/out the glowing green LED based on powerAnimProgress_
    if (powerAnimProgress_ > 0.005f) {
      float pAlpha = powerAnimProgress_ * safeAlpha;

      sf::CircleShape ledGlow(ledR * 2.2f);
      ledGlow.setOrigin({ledR * 2.2f, ledR * 2.2f});
      ledGlow.setPosition({ledX, ledY});
      ledGlow.setFillColor(sf::Color(40, 255, 75, static_cast<uint8_t>(80 * pAlpha)));
      target.draw(ledGlow);

      sf::CircleShape led(ledR);
      led.setOrigin({ledR, ledR});
      led.setPosition({ledX, ledY});
      led.setFillColor(sf::Color(65, 255, 95, static_cast<uint8_t>(255 * pAlpha)));
      target.draw(led);

      sf::CircleShape ledCore(ledR * 0.45f);
      ledCore.setOrigin({ledR * 0.45f, ledR * 0.45f});
      ledCore.setPosition({ledX - ledR * 0.15f, ledY - ledR * 0.15f});
      ledCore.setFillColor(sf::Color(220, 255, 230, static_cast<uint8_t>(255 * pAlpha)));
      target.draw(ledCore);
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
          0, 0, 0, static_cast<uint8_t>(buttonPressVisual_ * 110.0f * safeAlpha)));
      target.draw(pressTint);
      buttonPressVisual_ = std::max(0.0f, buttonPressVisual_ - dt * 6.0f);
    } else if (isButtonHovered_) {
      sf::RectangleShape hoverGlow({btnW, btnH});
      hoverGlow.setPosition({btnX, btnY});
      hoverGlow.setFillColor(sf::Color(0, 230, 245, static_cast<uint8_t>(35 * safeAlpha)));
      hoverGlow.setOutlineThickness(1.2f);
      hoverGlow.setOutlineColor(sf::Color(0, 255, 240, static_cast<uint8_t>(130 * safeAlpha)));
      target.draw(hoverGlow);
    }

    // 12. Tactical Layers Toggle Switch tactile feedback / LEDs
    float swX = l.frameX + TOGGLE_SW_X * l.s;
    float swY = l.frameY + TOGGLE_SW_Y * l.s;
    float swW = TOGGLE_SW_W * l.s;
    float swH = TOGGLE_SW_H * l.s;

    if (switchPressVisual_ > 0.01f) {
      sf::RectangleShape pressTint({swW, swH});
      pressTint.setPosition({swX, swY});
      pressTint.setFillColor(sf::Color(
          0, 0, 0, static_cast<uint8_t>(switchPressVisual_ * 120.0f * safeAlpha)));
      target.draw(pressTint);
      switchPressVisual_ = std::max(0.0f, switchPressVisual_ - dt * 6.0f);
    } else if (isSwitchHovered_) {
      sf::RectangleShape hoverGlow({swW, swH});
      hoverGlow.setPosition({swX, swY});
      hoverGlow.setFillColor(sf::Color(0, 230, 245, static_cast<uint8_t>(35 * safeAlpha)));
      hoverGlow.setOutlineThickness(1.2f);
      hoverGlow.setOutlineColor(sf::Color(0, 255, 240, static_cast<uint8_t>(130 * safeAlpha)));
      target.draw(hoverGlow);
    }

    // Toggle switch indicator LEDs (UPPER = layers ON, LOWER = layers OFF)
    float swLedX = l.frameX + TOGGLE_LED_X * l.s;
    float upperLedY = l.frameY + TOGGLE_LED_UPPER_Y * l.s;
    float lowerLedY = l.frameY + TOGGLE_LED_LOWER_Y * l.s;
    float swLedR = 3.8f * l.s;
    float coverR = 8.5f * l.s;

    // Smooth transition animation between upper (ON) and lower (OFF) states
    if (areTacticalLayersActive_) {
      tacticalLayersAnim_ = std::min(1.0f, tacticalLayersAnim_ + dt * 6.0f);
    } else {
      tacticalLayersAnim_ = std::max(0.0f, tacticalLayersAnim_ - dt * 6.0f);
    }
    float upperAlpha = tacticalLayersAnim_;
    float lowerAlpha = 1.0f - tacticalLayersAnim_;

    // --- 1. UPPER HOLE: Mask baked green light from map.png and draw metallic socket ---
    sf::CircleShape upperCover(coverR);
    upperCover.setOrigin({coverR, coverR});
    upperCover.setPosition({swLedX, upperLedY});
    upperCover.setFillColor(sf::Color(51, 60, 41, static_cast<uint8_t>(255 * safeAlpha)));
    target.draw(upperCover);

    sf::CircleShape upperRing(swLedR * 1.1f);
    upperRing.setOrigin({swLedR * 1.1f, swLedR * 1.1f});
    upperRing.setPosition({swLedX, upperLedY});
    upperRing.setFillColor(sf::Color(32, 38, 28, static_cast<uint8_t>(255 * safeAlpha)));
    upperRing.setOutlineThickness(1.0f * l.s);
    upperRing.setOutlineColor(sf::Color(70, 80, 58, static_cast<uint8_t>(200 * safeAlpha)));
    target.draw(upperRing);

    sf::CircleShape upperHole(swLedR * 0.7f);
    upperHole.setOrigin({swLedR * 0.7f, swLedR * 0.7f});
    upperHole.setPosition({swLedX, upperLedY});
    upperHole.setFillColor(sf::Color(14, 18, 12, static_cast<uint8_t>(255 * safeAlpha)));
    target.draw(upperHole);

    // Glowing green diode at UPPER socket when tactical layers are ON
    if (upperAlpha > 0.005f) {
      float uA = upperAlpha * safeAlpha;

      sf::CircleShape glow(swLedR * 2.2f);
      glow.setOrigin({swLedR * 2.2f, swLedR * 2.2f});
      glow.setPosition({swLedX, upperLedY});
      glow.setFillColor(sf::Color(40, 255, 75, static_cast<uint8_t>(75 * uA)));
      target.draw(glow);

      sf::CircleShape diode(swLedR);
      diode.setOrigin({swLedR, swLedR});
      diode.setPosition({swLedX, upperLedY});
      diode.setFillColor(sf::Color(65, 255, 95, static_cast<uint8_t>(255 * uA)));
      target.draw(diode);

      sf::CircleShape diodeCore(swLedR * 0.45f);
      diodeCore.setOrigin({swLedR * 0.45f, swLedR * 0.45f});
      diodeCore.setPosition({swLedX - swLedR * 0.15f, upperLedY - swLedR * 0.15f});
      diodeCore.setFillColor(sf::Color(220, 255, 230, static_cast<uint8_t>(255 * uA)));
      target.draw(diodeCore);
    }

    // --- 2. LOWER HOLE: Draw metallic socket and glow when layers are OFF ---
    sf::CircleShape lowerRing(swLedR * 1.1f);
    lowerRing.setOrigin({swLedR * 1.1f, swLedR * 1.1f});
    lowerRing.setPosition({swLedX, lowerLedY});
    lowerRing.setFillColor(sf::Color(32, 38, 28, static_cast<uint8_t>(255 * safeAlpha)));
    lowerRing.setOutlineThickness(1.0f * l.s);
    lowerRing.setOutlineColor(sf::Color(70, 80, 58, static_cast<uint8_t>(200 * safeAlpha)));
    target.draw(lowerRing);

    sf::CircleShape lowerHole(swLedR * 0.7f);
    lowerHole.setOrigin({swLedR * 0.7f, swLedR * 0.7f});
    lowerHole.setPosition({swLedX, lowerLedY});
    lowerHole.setFillColor(sf::Color(14, 18, 12, static_cast<uint8_t>(255 * safeAlpha)));
    target.draw(lowerHole);

    // Glowing green diode at LOWER socket when tactical layers are OFF
    if (lowerAlpha > 0.005f) {
      float lA = lowerAlpha * safeAlpha;

      sf::CircleShape glow(swLedR * 2.2f);
      glow.setOrigin({swLedR * 2.2f, swLedR * 2.2f});
      glow.setPosition({swLedX, lowerLedY});
      glow.setFillColor(sf::Color(40, 255, 75, static_cast<uint8_t>(75 * lA)));
      target.draw(glow);

      sf::CircleShape diode(swLedR);
      diode.setOrigin({swLedR, swLedR});
      diode.setPosition({swLedX, lowerLedY});
      diode.setFillColor(sf::Color(65, 255, 95, static_cast<uint8_t>(255 * lA)));
      target.draw(diode);

      sf::CircleShape diodeCore(swLedR * 0.45f);
      diodeCore.setOrigin({swLedR * 0.45f, swLedR * 0.45f});
      diodeCore.setPosition({swLedX - swLedR * 0.15f, lowerLedY - swLedR * 0.15f});
      diodeCore.setFillColor(sf::Color(220, 255, 230, static_cast<uint8_t>(255 * lA)));
      target.draw(diodeCore);
    }

    // 13. Interactive CRT Bezel Zoom Slider Knob
    float currentMag = 1.0f / std::clamp(currentCameraZoom_, 1.0f / 4.50f, 1.0f / 0.20f);
    constexpr float logMin = -1.6094379f; // std::log(0.20f)
    constexpr float logMax = 1.5040774f;  // std::log(4.50f)
    float targetSliderT = std::clamp((std::log(currentMag) - logMin) / (logMax - logMin), 0.0f, 1.0f);

    // Smoothly glide knob towards current zoom unless actively dragged
    if (isZoomSliderDragging_) {
      sliderVisualT_ = targetSliderT;
    } else {
      sliderVisualT_ += (targetSliderT - sliderVisualT_) * std::min(1.0f, dt * 20.0f);
    }

    float knobTexX = SLIDER_MIN_X + sliderVisualT_ * (SLIDER_MAX_X - SLIDER_MIN_X);
    float knobScreenX = l.frameX + knobTexX * l.s;
    float knobScreenY = l.frameY + SLIDER_Y * l.s;

    if (hasKnobTexture_) {
      sf::Sprite knobSprite(knobTexture_);
      knobSprite.setOrigin({SLIDER_KNOB_W * 0.5f, SLIDER_KNOB_H * 0.5f});
      knobSprite.setPosition({knobScreenX, knobScreenY});
      knobSprite.setScale({l.s, l.s});
      knobSprite.setColor(sf::Color(255, 255, 255, static_cast<uint8_t>(255 * safeAlpha)));
      target.draw(knobSprite);
    }

    // Hover or drag visual feedback: clean cyan outline matching bezel buttons
    if (isZoomSliderDragging_ || isZoomSliderHovered_) {
      float glowW = SLIDER_KNOB_W * l.s;
      float glowH = SLIDER_KNOB_H * l.s;
      sf::RectangleShape knobGlow({glowW, glowH});
      knobGlow.setOrigin({glowW * 0.5f, glowH * 0.5f});
      knobGlow.setPosition({knobScreenX, knobScreenY});
      if (isZoomSliderDragging_) {
        knobGlow.setFillColor(sf::Color(0, 230, 245, static_cast<uint8_t>(45 * safeAlpha)));
        knobGlow.setOutlineThickness(1.2f);
        knobGlow.setOutlineColor(sf::Color(0, 255, 240, static_cast<uint8_t>(200 * safeAlpha)));
      } else {
        knobGlow.setFillColor(sf::Color(0, 230, 245, static_cast<uint8_t>(35 * safeAlpha)));
        knobGlow.setOutlineThickness(1.2f);
        knobGlow.setOutlineColor(sf::Color(0, 255, 240, static_cast<uint8_t>(130 * safeAlpha)));
      }
      target.draw(knobGlow);
    }

    // 14. Overview Dial Button (Fit Whole Maze) tactile feedback / circular cyan hover
    float dialX = l.frameX + DIAL_BTN_X * l.s;
    float dialY = l.frameY + DIAL_BTN_Y * l.s;
    float dialR = DIAL_BTN_R * l.s;

    if (dialPressVisual_ > 0.01f) {
      sf::CircleShape pressTint(dialR);
      pressTint.setOrigin({dialR, dialR});
      pressTint.setPosition({dialX, dialY});
      pressTint.setFillColor(sf::Color(
          0, 0, 0, static_cast<uint8_t>(dialPressVisual_ * 110.0f * safeAlpha)));
      target.draw(pressTint);
      dialPressVisual_ = std::max(0.0f, dialPressVisual_ - dt * 6.0f);
    } else if (isDialButtonHovered_) {
      sf::CircleShape hoverGlow(dialR);
      hoverGlow.setOrigin({dialR, dialR});
      hoverGlow.setPosition({dialX, dialY});
      hoverGlow.setFillColor(sf::Color(0, 230, 245, static_cast<uint8_t>(35 * safeAlpha)));
      hoverGlow.setOutlineThickness(1.2f);
      hoverGlow.setOutlineColor(sf::Color(0, 255, 240, static_cast<uint8_t>(130 * safeAlpha)));
      target.draw(hoverGlow);
    }
  }

  bool handleEvent(const sf::Event &event, float windowWidth,
                   float windowHeight, const map::Maze &maze,
                   sf::Vector2f &outWorldTarget, bool &outIsDragging,
                   float &outZoomFactor, float scaleOrSize = 1.0f) {
    float mazeW = maze.getWidth() * ProceduralRenderer::TILE_SIZE;
    float mazeH = maze.getHeight() * ProceduralRenderer::TILE_SIZE;
    Layout l = calculateLayout(windowWidth, windowHeight, scaleOrSize, mazeW,
                               mazeH, introOffsetY_);

    outZoomFactor = 1.0f;

    if (const auto *mouseBtn = event.getIf<sf::Event::MouseButtonReleased>()) {
      if (mouseBtn->button == sf::Mouse::Button::Left) {
        isDragging_ = false;
        isZoomSliderDragging_ = false;
      }
    }

    if (const auto *mouseMove = event.getIf<sf::Event::MouseMoved>()) {
      isButtonHovered_ =
          isOverPowerButton(mouseMove->position.x, mouseMove->position.y, l);
      isSwitchHovered_ =
          isOverToggleSwitch(mouseMove->position.x, mouseMove->position.y, l);
      isZoomSliderHovered_ =
          isOverZoomSlider(mouseMove->position.x, mouseMove->position.y, l);
      isDialButtonHovered_ =
          isOverDialButton(mouseMove->position.x, mouseMove->position.y, l);

      if (isZoomSliderDragging_) {
        requestedCameraZoom_ = calculateZoomFromMouseX(
            static_cast<float>(mouseMove->position.x), l);
        hasRequestedCameraZoom_ = true;
        isDragging_ = false;
        outIsDragging = false;
        return false;
      }
    }

    // Power button, Tactical Layers toggle switch, Zoom Slider, or Overview Dial click on LMB
    if (const auto *mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
      if (mouseBtn->button == sf::Mouse::Button::Left) {
        if (isOverDialButton(mouseBtn->position.x, mouseBtn->position.y, l)) {
          requestFitMaze_ = true;
          dialPressVisual_ = 1.0f;
          isDragging_ = false;
          outIsDragging = false;
          return false; // Handled overview dial button, do not pan world camera!
        }
        if (isOverZoomSlider(mouseBtn->position.x, mouseBtn->position.y, l)) {
          isZoomSliderDragging_ = true;
          requestedCameraZoom_ = calculateZoomFromMouseX(
              static_cast<float>(mouseBtn->position.x), l);
          hasRequestedCameraZoom_ = true;
          isDragging_ = false;
          outIsDragging = false;
          return false; // Handled zoom slider, do not pan world camera!
        }
        if (isOverPowerButton(mouseBtn->position.x, mouseBtn->position.y, l)) {
          togglePower();
          buttonPressVisual_ = 1.0f;
          isDragging_ = false;
          outIsDragging = false;
          return false; // Handled power button, do not pan world camera!
        }
        if (isOverToggleSwitch(mouseBtn->position.x, mouseBtn->position.y, l)) {
          requestToggleTacticalLayers_ = true;
          switchPressVisual_ = 1.0f;
          isDragging_ = false;
          outIsDragging = false;
          return false; // Handled toggle switch!
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
      if (mouseBtn->button == sf::Mouse::Button::Left) {
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
    Layout l = calculateLayout(windowWidth, windowHeight, scaleOrSize, mazeW,
                               mazeH, introOffsetY_);

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
  sf::Texture knobTexture_;
  bool hasKnobTexture_ = false;
  bool isZoomSliderHovered_ = false;
  bool isZoomSliderDragging_ = false;
  float currentCameraZoom_ = 1.0f;
  float sliderVisualT_ = 0.517f;
  bool hasRequestedCameraZoom_ = false;
  float requestedCameraZoom_ = 1.0f;
  bool isDragging_ = false;
  float activeGlow_ = 0.0f;
  sf::Clock glowClock_;
  PowerState powerState_ = PowerState::On;
  float powerAnimProgress_ = 1.0f;
  bool isButtonHovered_ = false;
  float buttonPressVisual_ = 0.0f;
  bool isSwitchHovered_ = false;
  float switchPressVisual_ = 0.0f;
  bool isDialButtonHovered_ = false;
  float dialPressVisual_ = 0.0f;
  bool requestFitMaze_ = false;
  bool requestToggleTacticalLayers_ = false;
  bool areTacticalLayersActive_ = true;
  float tacticalLayersAnim_ = 1.0f;
  float routeCompletionTimer_ = 0.0f;
  bool wasRouteCompleted_ = false;
  float introOffsetY_ = 0.0f;
  float animTime_ = 0.0f;
  sf::VertexArray minimapQuads_{sf::PrimitiveType::Triangles};
  sf::VertexArray minimapBorders_{sf::PrimitiveType::Lines};
};

} // namespace ui
