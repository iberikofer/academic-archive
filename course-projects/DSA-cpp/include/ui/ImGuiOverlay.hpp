#pragma once

#include "CameraController.hpp"
#include "Minimap.hpp"
#include "ProceduralRenderer.hpp"
#include "core/Config.hpp"
#include "imgui.h"
#include "io/CsvExporter.hpp"
#include "map/Maze.hpp"
#include "simulation/SimulationEngine.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>

namespace ui {

class ImGuiOverlay {
public:
  static constexpr float MIN_SIDEBAR_WIDTH = 595.0f;
  static constexpr float DEFAULT_SIDEBAR_WIDTH = 605.0f;

  struct UIState {
    int knapsackAlgoIndex = 0;   // 0: DP, 1: Greedy
    int pathfinderAlgoIndex = 0; // 0: A*, 1: Dijkstra, 2: BFS
    bool triggerRegeneration = false;
    bool triggerRecalculate = false;
    bool triggerBatchBenchmark = false;
    bool triggerExportCsv = false;
    bool triggerImportJson = false;
    bool triggerSaveConfigJson = false;
    bool triggerExportReportJson = false;
    std::string statusNotice;

    // Custom seed configuration
    bool customSeedEnabled = false;
    char customSeedInput[32] = "";
    bool useSpecifiedSeed = false;
    unsigned int specifiedSeedValue = 0;

    // Tracks parameters currently applied to the active simulation plan
    int appliedKnapsackAlgoIndex = 0;
    int appliedPathfinderAlgoIndex = 0;
    int appliedBackpackCapacity = 0;
    core::RouteMode appliedRouteMode = core::RouteMode::PointAToPointB;
    int appliedInitialEnergy = 600;
    int appliedCompanyQuota = 0;
    bool appliedEnergyLimitEnabled = true;
    bool appliedBackpackLimitEnabled = true;

    // Real-time slider coupling feedback
    std::string sliderAutoAdjustNotice;
    float sliderAutoAdjustTimer = 0.0f;

    // Camera & display commands
    bool requestCenterCameraOnAgent = false;
    bool requestCenterCameraOnMaze = false;
    bool requestFitMazeInView = false;
    bool requestReturnToLastAutoStep = false;
    bool vsyncChanged = false;
    float cameraZoom = 1.0f;
    float targetCameraZoom = 1.0f;
    bool cameraZoomChanged = false;
    bool requestSmoothZoom = false;

    // Generation transition & sonar scan telemetry
    bool isTransitionActive = false;
    bool isSonarScanning = false;
    float sonarProgress = 0.0f;
    bool isRouteTracing = false;
    float routeProgress = 0.0f;
    bool isCameraZooming = false;
  };

  static ImVec4 lerpColor(const ImVec4 &a, const ImVec4 &b, float t) {
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
                  a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
  }

  [[nodiscard]] float getSidebarWidth() const noexcept {
    float easeT = sidebarOpenProgress_ * sidebarOpenProgress_ *
                  (3.0f - 2.0f * sidebarOpenProgress_);
    return sidebarWidth_ * easeT;
  }
  [[nodiscard]] bool isSidebarCollapsed() const noexcept {
    return sidebarCollapsed_;
  }
  void setSidebarCollapsed(bool collapsed) noexcept {
    sidebarCollapsed_ = collapsed;
  }
  void setSidebarWidth(float w, float uiScale = 1.0f) noexcept {
    sidebarWidth_ = w;
    savedSidebarBaseWidth_ = (uiScale > 0.001f) ? (w / uiScale) : w;
  }

  void toggleSidebar(float uiScale = 1.0f) noexcept {
    sidebarCollapsed_ = !sidebarCollapsed_;
    if (!sidebarCollapsed_) {
      startupHintDismissed_ = true;
      sidebarWidth_ = std::max(MIN_SIDEBAR_WIDTH * uiScale,
                               savedSidebarBaseWidth_ * uiScale);
    }
  }

  void setMissionPanelCollapsed(bool collapsed) noexcept {
    missionPanelCollapsed_ = collapsed;
  }

  [[nodiscard]] bool isMissionPanelCollapsed() const noexcept {
    return missionPanelCollapsed_;
  }

  void setMissionPanelOpenProgress(float progress) noexcept {
    missionPanelOpenProgress_ = progress;
  }

  void startHudIntro() noexcept {
    hudIntroActive_ = true;
    hudIntroProgress_ = 0.0f;
    hudAttachProgress_ = 0.0f;
  }

  void updateHudIntro(float dt) noexcept {
    if (!hudIntroActive_)
      return;
    const float HUD_INTRO_DUR = 0.44f;
    const float HUD_ATTACH_DUR = 0.36f;
    if (hudIntroProgress_ < 1.0f) {
      hudIntroProgress_ =
          std::min(1.0f, hudIntroProgress_ + dt / HUD_INTRO_DUR);
    } else if (hudAttachProgress_ < 1.0f) {
      hudAttachProgress_ =
          std::min(1.0f, hudAttachProgress_ + dt / HUD_ATTACH_DUR);
      if (hudAttachProgress_ >= 1.0f) {
        hudAttachProgress_ = 1.0f;
        hudIntroActive_ = false;
      }
    }
  }

  [[nodiscard]] float getHudIntroEase() const noexcept {
    float t = std::clamp(hudIntroProgress_, 0.0f, 1.0f);
    return 1.0f - std::pow(1.0f - t, 3.0f);
  }

  [[nodiscard]] float getHudAttachEase() const noexcept {
    float t = std::clamp(hudAttachProgress_, 0.0f, 1.0f);
    return 1.0f - std::pow(1.0f - t, 3.0f);
  }

  [[nodiscard]] bool isHudIntroComplete() const noexcept {
    return !hudIntroActive_ && hudIntroProgress_ >= 1.0f &&
           hudAttachProgress_ >= 1.0f;
  }

  [[nodiscard]] bool isMouseInLegendMarginDeadzone(
      float mx, float my, const core::Config &config, float windowWidth,
      float windowHeight) const noexcept {
    float legendW = 355.0f * config.uiScale;
    float btnX = windowWidth - legendW - Minimap::PADDING;
    float btnY = Minimap::PADDING;
    float btnH =
        std::max(34.0f * config.uiScale, ImGui::GetFrameHeight() + 6.0f);

    // Strip above and right of the Legend toggle button (and right edge beside legend)
    if (mx >= btnX - 2.0f && mx <= windowWidth && my >= 0.0f &&
        my <= btnY + 1.0f) {
      return true;
    }
    if (mx >= windowWidth - Minimap::PADDING - 1.0f && mx <= windowWidth &&
        my >= 0.0f && my <= btnY + btnH + 2.0f) {
      return true;
    }
    if (config.showLegend && mx >= windowWidth - Minimap::PADDING - 1.0f &&
        mx <= windowWidth && my >= 0.0f && my <= windowHeight) {
      return true;
    }
    return false;
  }

  [[nodiscard]] int getHoveredCollectedItemId() const noexcept {
    return hoveredCollectedItemId_;
  }
  [[nodiscard]] const std::string &getHoveredCatalogItemName() const noexcept {
    return hoveredCatalogItemName_;
  }
  [[nodiscard]] int getHoveredCatalogCategory() const noexcept {
    return hoveredCatalogCategory_;
  }

  void fitMazeToVisibleArea(CameraController &camera, const map::Maze &maze,
                            const core::Config &config,
                            const simulation::SimulationEngine &engine,
                            float windowWidth, float windowHeight,
                            float extraPaddingOffset = 0.0f,
                            bool smooth = true) const {
    const float tile = ProceduralRenderer::TILE_SIZE;
    int minTX = maze.getWidth(), maxTX = -1;
    int minTY = maze.getHeight(), maxTY = -1;
    for (int y = 0; y < maze.getHeight(); ++y) {
      for (int x = 0; x < maze.getWidth(); ++x) {
        if (maze.isPassable(x, y)) {
          minTX = std::min(minTX, x);
          maxTX = std::max(maxTX, x);
          minTY = std::min(minTY, y);
          maxTY = std::max(maxTY, y);
        }
      }
    }

    float contentCenterX = maze.getWidth() * tile * 0.5f;
    float contentCenterY = maze.getHeight() * tile * 0.5f;
    float contentW = maze.getWidth() * tile;
    float contentH = maze.getHeight() * tile;
    int activeTiles = std::max(maze.getWidth(), maze.getHeight());

    if (maxTX >= minTX && maxTY >= minTY) {
      // Include 1-tile wall margin around active rooms/corridors
      float wx0 = std::max(0, minTX - 1) * tile;
      float wy0 = std::max(0, minTY - 1) * tile;
      float wx1 = std::min(maze.getWidth(), maxTX + 2) * tile;
      float wy1 = std::min(maze.getHeight(), maxTY + 2) * tile;
      contentW = std::max(tile * 6.0f, wx1 - wx0);
      contentH = std::max(tile * 6.0f, wy1 - wy0);
      contentCenterX = (wx0 + wx1) * 0.5f;
      contentCenterY = (wy0 + wy1) * 0.5f;
      activeTiles = std::max(maxTX - minTX + 3, maxTY - minTY + 3);
    }

    // Scale padding gently by maze size: small mazes stay zoomed in, larger ones get a bit more breathing room
    float sizeFactor =
        std::clamp((static_cast<float>(activeTiles) - 20.0f) / 80.0f, 0.0f, 1.0f);
    float padding = 1.07f + 0.08f * sizeFactor + extraPaddingOffset;

    float sbEase = sidebarOpenProgress_ * sidebarOpenProgress_ *
                   (3.0f - 2.0f * sidebarOpenProgress_);
    float leftOcc = sidebarCollapsed_
                        ? (20.0f * config.uiScale)
                        : (sidebarWidth_ * sbEase + 16.0f * config.uiScale);

    float legendOcc = (legendAnimT_ > 0.05f || config.showLegend)
                          ? (355.0f * config.uiScale)
                          : 0.0f;
    float rightHudOcc =
        Minimap::PADDING +
        std::max(legendOcc, Minimap::getFrameWidth(config.minimapScale)) +
        14.0f * config.uiScale;

    float safeLeft = leftOcc + 10.0f * config.uiScale;
    float safeRight = windowWidth - rightHudOcc;
    if (safeRight - safeLeft < 240.0f) {
      safeLeft = 24.0f;
      safeRight = windowWidth - 24.0f;
    }

    // Center slightly back toward full screen center when right panel leaves plenty of width
    float safeCenterX = (safeLeft + safeRight) * 0.5f;
    float blendedCenterX =
        safeCenterX + (windowWidth * 0.5f - safeCenterX) * 0.12f;
    float halfSafeW = (safeRight - safeLeft) * 0.5f;
    float adjLeft = blendedCenterX - halfSafeW;
    float adjRight = blendedCenterX + halfSafeW;

    float safeTop = 26.0f * config.uiScale;
    bool hasStatusPanel =
        !missionPanelCollapsed_ || missionPanelOpenProgress_ > 0.05f;
    bool hasManualCard =
        engine.isManualMode() || manualBannerOpenProgress_ > 0.05f;
    // Status panel sits only in bottom-right corner, so reserve modest vertical margin instead of full-height cut
    float bottomReserve = 32.0f * config.uiScale;
    if (hasStatusPanel) {
      bottomReserve =
          (hasManualCard ? 150.0f : 112.0f) * config.uiScale;
    }
    float safeBottom = windowHeight - bottomReserve;
    if (safeBottom - safeTop < 260.0f) {
      safeBottom = std::min(windowHeight - 24.0f, safeTop + 260.0f);
    }

    camera.fitMazeInRect(contentCenterX, contentCenterY, contentW, contentH,
                         adjLeft, safeTop, adjRight, safeBottom, windowWidth,
                         windowHeight, padding, smooth, 1.0f);
  }

  static void showStyledTooltip(float uiScale, const char *fmt, ...) {
    float s = std::clamp(uiScale, 0.75f, 1.50f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(12.0f * s, 9.0f * s));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(8.0f * s, 6.0f * s));
    ImGui::PushStyleColor(ImGuiCol_PopupBg,
                          ImVec4(0.09f, 0.08f, 0.10f, 0.96f));
    ImGui::PushStyleColor(ImGuiCol_Border,
                          ImVec4(0.55f, 0.19f, 0.28f, 0.90f));
    if (ImGui::BeginTooltip()) {
      float maxWrap = std::clamp(ImGui::GetIO().DisplaySize.x * 0.34f,
                                 260.0f * s, 460.0f * s);
      ImGui::PushTextWrapPos(maxWrap);
      va_list args;
      va_start(args, fmt);
      ImGui::TextV(fmt, args);
      va_end(args);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(5);
  }

  static void applyLethalCompanyCherryTheme(ImGuiStyle &style) {
    ImVec4 *colors = style.Colors;

    // Visual layout & borders (Tactical industrial terminal styling)
    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;

    // Comfortable spacing & padding
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.ItemSpacing =
        ImVec2(10.0f, 9.0f); // Increased spacing between elements
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.FramePadding =
        ImVec2(8.0f, 5.0f); // Roomy frame padding inside inputs/buttons

    // Base text & dark industrial backdrop
    colors[ImGuiCol_Text] = ImVec4(0.96f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.48f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.08f, 0.09f, 0.96f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.09f, 0.10f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.48f, 0.16f, 0.24f, 0.65f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame backgrounds (Inputs, Checkboxes, Sliders - Deep dark cherry wine)
    colors[ImGuiCol_FrameBg] = ImVec4(0.24f, 0.08f, 0.12f, 0.65f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.13f, 0.19f, 0.78f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.60f, 0.16f, 0.24f, 0.90f);

    // Window Title & Menu bar
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.07f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.42f, 0.11f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.05f, 0.06f, 0.60f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.10f, 0.12f, 1.00f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.05f, 0.06f, 0.55f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.48f, 0.14f, 0.20f, 0.70f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.72f, 0.18f, 0.26f, 0.85f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.92f, 0.24f, 0.32f, 1.00f);

    // CheckMark & Slider Grabs (Vivid Lethal Company Cherry #E82A3E / #FA4D60)
    colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.30f, 0.40f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.85f, 0.20f, 0.28f, 0.95f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.35f, 0.45f, 1.00f);

    // Buttons (Lethal Company light-cherry stencil badge)
    colors[ImGuiCol_Button] = ImVec4(0.62f, 0.14f, 0.22f, 0.65f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.85f, 0.20f, 0.28f, 0.90f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.28f, 0.36f, 1.00f);

    // Collapsing Headers & Selectables
    colors[ImGuiCol_Header] = ImVec4(0.56f, 0.13f, 0.22f, 0.90f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.82f, 0.20f, 0.30f, 0.96f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.96f, 0.26f, 0.36f, 1.00f);

    // Separators
    colors[ImGuiCol_Separator] = ImVec4(0.42f, 0.16f, 0.22f, 0.65f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.82f, 0.20f, 0.28f, 0.80f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.98f, 0.26f, 0.35f, 1.00f);

    // Resize grips
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.55f, 0.14f, 0.20f, 0.35f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.85f, 0.22f, 0.30f, 0.75f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.28f, 0.38f, 0.95f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.45f, 0.12f, 0.18f, 0.75f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.85f, 0.20f, 0.28f, 0.85f);
    colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.16f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.28f, 0.08f, 0.12f, 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.50f, 0.13f, 0.18f, 0.90f);

    // Progress bar / Histogram
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.20f, 0.28f, 0.90f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.28f, 0.36f, 1.00f);

    // Tables
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.26f, 0.09f, 0.14f, 0.90f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.48f, 0.16f, 0.24f, 0.75f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.35f, 0.12f, 0.18f, 0.60f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.30f, 0.10f, 0.15f, 0.20f);

    // Text Selection & Nav Highlight
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.82f, 0.20f, 0.28f, 0.40f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.92f, 0.26f, 0.34f, 1.00f);
  }

  static bool renderSectionHeader(const char *label, float uiScale,
                                  bool defaultOpen = true) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                        ImVec2(12.0f * uiScale, 8.5f * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f * uiScale);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f * uiScale);

    // Deep high-contrast dark maroon background to make the blue border pop
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.36f, 0.11f, 0.19f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                          ImVec4(0.50f, 0.15f, 0.25f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                          ImVec4(0.62f, 0.18f, 0.30f, 1.00f));
    // Vibrant electric cyan/blue border
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.95f));
    // Crisp white-cyan text
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.96f, 0.99f, 1.00f, 1.00f));

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (defaultOpen) {
      flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    bool open = ImGui::CollapsingHeader(label, flags);

    ImVec2 pMin = ImGui::GetItemRectMin();
    ImVec2 pMax = ImGui::GetItemRectMax();
    ImDrawList *drawList = ImGui::GetWindowDrawList();

    bool isHovered = ImGui::IsItemHovered();

    // 1. Crisp electric cyan/blue outer border
    ImU32 borderCol =
        isHovered ? IM_COL32(90, 235, 255, 255) : IM_COL32(0, 210, 255, 240);
    drawList->AddRect(pMin, pMax, borderCol, 4.0f * uiScale, 0, 1.5f * uiScale);

    // 2. High-visibility glowing cyan accent stripe on the left edge
    ImU32 stripeCol =
        isHovered ? IM_COL32(140, 248, 255, 255) : IM_COL32(0, 225, 255, 255);
    drawList->AddLine(ImVec2(pMin.x + 3.0f * uiScale, pMin.y + 2.0f * uiScale),
                      ImVec2(pMin.x + 3.0f * uiScale, pMax.y - 2.0f * uiScale),
                      stripeCol, 4.5f * uiScale);

    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(3);

    return open;
  }

  void render(core::Config &config, map::Maze &maze,
              simulation::SimulationEngine &engine,
              const std::vector<io::BenchmarkRow> &benchmarkRows,
              float windowHeight, UIState &state) {
    // Handle dynamic UI scaling with crisp text (oversampled TTF)
    static float appliedScale = -1.0f;
    static ImGuiStyle baseStyle;
    static bool baseStyleCaptured = false;
    if (!baseStyleCaptured) {
      applyLethalCompanyCherryTheme(ImGui::GetStyle());
      baseStyle = ImGui::GetStyle();
      baseStyleCaptured = true;
    }

    const float BASE_FONT_SIZE = 17.0f;
    const float NATIVE_FONT_SIZE = 26.0f;
    if (std::abs(config.uiScale - appliedScale) > 0.001f) {
      ImGui::GetStyle() = baseStyle;
      ImGui::GetStyle().ScaleAllSizes(config.uiScale);
      ImGui::GetIO().FontGlobalScale =
          (BASE_FONT_SIZE / NATIVE_FONT_SIZE) * config.uiScale;
      sidebarWidth_ = savedSidebarBaseWidth_ * config.uiScale;
      appliedScale = config.uiScale;
    }

    float minSidebarW = MIN_SIDEBAR_WIDTH * config.uiScale;
    float maxSidebarWidth = ImGui::GetIO().DisplaySize.x * 0.55f;
    sidebarWidth_ = std::clamp(sidebarWidth_, minSidebarW, maxSidebarWidth);

    float dt = ImGui::GetIO().DeltaTime;
    const float SIDEBAR_ANIM_DURATION =
        0.24f; // Clean, responsive 240ms transition
    if (sidebarCollapsed_) {
      sidebarOpenProgress_ =
          std::max(0.0f, sidebarOpenProgress_ - dt / SIDEBAR_ANIM_DURATION);
    } else {
      sidebarOpenProgress_ =
          std::min(1.0f, sidebarOpenProgress_ + dt / SIDEBAR_ANIM_DURATION);
    }

    float easeT = sidebarOpenProgress_ * sidebarOpenProgress_ *
                  (3.0f - 2.0f * sidebarOpenProgress_);
    float hudEase = getHudIntroEase();
    if ((easeT <= 0.0001f && sidebarCollapsed_) || hudEase <= 0.0001f) {
      return;
    }

    float currentWindowX = -sidebarWidth_ * (1.0f - easeT * hudEase);
    ImGui::SetNextWindowPos(ImVec2(currentWindowX, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(sidebarWidth_, windowHeight),
                             ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin("Керування та аналітика##Sidebar", nullptr, flags);
    ImGui::SetScrollX(0.0f);

    if (state.sliderAutoAdjustTimer > 0.0f) {
      state.sliderAutoAdjustTimer -= ImGui::GetIO().DeltaTime;
    }

    // 1. Display and camera settings
    if (renderSectionHeader("1.  Відображення та огляд", config.uiScale,
                            true)) {
      ImGui::Spacing();

      // Zoom Scale mixer slider
      float currentMagnification = 1.0f / std::max(0.01f, state.cameraZoom);
      float userZoom = currentMagnification;
      if (ImGui::SliderFloat("Масштаб огляду (Zoom)", &userZoom, 0.20f, 4.50f,
                             "%.2fx")) {
        userZoom = std::clamp(userZoom, 0.20f, 4.50f);
        state.cameraZoom = 1.0f / userZoom;
        state.targetCameraZoom = state.cameraZoom;
        state.cameraZoomChanged = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Мікшер масштабу огляду камери:\n"
            "Вліво (0.2x - 0.9x): віддалити для ширшого огляду лабіринту.\n"
            "Вправо (1.1x - 4.5x): наблизити для детального огляду клітинок.\n"
            "Також можна наближати/віддаляти скролом коліщатка миші.");
      }

      // Quick preset buttons for zoom
      float availZoomWidth = ImGui::GetContentRegionAvail().x;
      float btnSpacing = ImGui::GetStyle().ItemSpacing.x;
      float zoomBtnW =
          std::max(10.0f, (availZoomWidth - 3.0f * btnSpacing) / 4.0f);
      float zoomBtnH =
          std::max(26.0f * config.uiScale, ImGui::GetFrameHeight());

      if (ImGui::Button("[-] Віддалити", ImVec2(zoomBtnW, zoomBtnH))) {
        float baseZoom = (state.targetCameraZoom > 0.01f)
                             ? state.targetCameraZoom
                             : state.cameraZoom;
        float currentTargetMag = 1.0f / std::max(0.01f, baseZoom);
        float nextMag = std::clamp(currentTargetMag * 0.80f, 0.20f, 4.50f);
        state.targetCameraZoom = 1.0f / nextMag;
        state.requestSmoothZoom = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "%s", "Плавно віддалити огляд камери (зменшити масштаб на 20%).");
      }

      ImGui::SameLine();
      if (ImGui::Button("[+] Наблизити", ImVec2(zoomBtnW, zoomBtnH))) {
        float baseZoom = (state.targetCameraZoom > 0.01f)
                             ? state.targetCameraZoom
                             : state.cameraZoom;
        float currentTargetMag = 1.0f / std::max(0.01f, baseZoom);
        float nextMag = std::clamp(currentTargetMag * 1.25f, 0.20f, 4.50f);
        state.targetCameraZoom = 1.0f / nextMag;
        state.requestSmoothZoom = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "%s", "Плавно наблизити огляд камери (збільшити масштаб на 25%).");
      }

      ImGui::SameLine();
      if (ImGui::Button("[1.0x] Скинути", ImVec2(zoomBtnW, zoomBtnH))) {
        state.targetCameraZoom = 1.0f;
        state.requestSmoothZoom = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Плавно скинути масштаб камери до стандартного 1.00x.");
      }

      ImGui::SameLine();
      if (ImGui::Button("Весь лабіринт", ImVec2(zoomBtnW, zoomBtnH))) {
        state.requestFitMazeInView = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Огляд усього лабіринту: плавно підібрати масштаб так, щоб "
            "увесь лабіринт вмістився в екрані.");
      }

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      // Display checkbox: Grid
      ImGui::Checkbox("Показувати сітку", &config.showGrid);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Відображати тонку координатно-тактичну сітку між "
                          "клітинками лабіринту.");
      }

      ImGui::Checkbox("Показувати оптимальний маршрут",
                      &config.showOptimalRoute);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Увімкнути або вимкнути відображення розрахованої "
                          "оптимальної траєкторії маршруту.");
      }

      ImGui::Checkbox("Показувати пройдені клітинки",
                      &config.showWalkedCells);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Відображати або приховувати підсвічування "
                          "пройдених робітником клітинок.");
      }

      // Camera follow & center on agent
      if (state.isTransitionActive) {
        ImGui::BeginDisabled(true);
      }
      if (ImGui::Checkbox("Прив'язка камери до агента",
                          &config.cameraFollowAgent)) {
        if (config.cameraFollowAgent) {
          state.requestCenterCameraOnAgent = true;
        } else {
          state.requestCenterCameraOnMaze = true;
        }
      }
      if (state.isTransitionActive) {
        ImGui::EndDisabled();
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        if (state.isTransitionActive) {
          ImGui::SetTooltip("Центрування недоступне під час генерації та "
                            "трасування маршруту.");
        } else {
          ImGui::SetTooltip("Якщо увімкнено: камера автоматично слідує за "
                            "робітником при кожному його кроці.\n"
                            "Якщо вимкнено: вільний огляд без прив'язки камери "
                            "(пересувайте стрілочками або ПКМ).");
        }
      }

      // VSync
      if (ImGui::Checkbox("Вертикальна синхронізація (VSync)",
                          &config.vsyncEnabled)) {
        state.vsyncChanged = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Усуває розриви кадрів ('драбинку') та ривки при плавному русі "
            "камери.\n"
            "Синхронізує частоту кадрів із герцовкою вашого монітора.");
      }

      // UI Scale
      ImGui::SliderFloat("Масштаб UI", &config.uiScale, 0.75f, 1.50f, "%.2fx");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Масштабування розміру інтерфейсу, тексту та кнопок "
                          "(від 0.75x до 1.50x).");
      }

      // Minimap Scale slider
      ImGui::SliderFloat("Розмір міні-карти", &config.minimapScale, 0.70f,
                         1.30f, "%.2fx");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Масштабування розміру тактичної "
                          "міні-карти (від 0.70x до 1.30x).");
      }

      ImGui::Spacing();
      ImGui::TextColored(ImVec4(0.85f, 0.75f, 0.50f, 1.0f),
                         "Гарячі клавіші та керування:");
      ImGui::TextDisabled(
          "- [W, A, S, D] - ручне керування переміщенням робітника");
      ImGui::TextDisabled("- [M] - Показати / сховати легенду позначок");
      ImGui::TextDisabled("- [C] - Центрувати / прив'язати камеру до агента");
      ImGui::TextDisabled(
          "- [Space] - Крок вперед (затиснути для швидкого руху)");
      ImGui::TextDisabled("- [Shift+Space] - Крок назад (перемотування)");
      ImGui::TextDisabled(
          "- [Ctrl+Space] - Повернутись на старт / Телепорт на фініш");
      ImGui::TextDisabled("- [ЛКМ] - панорамування карти");
      ImGui::TextDisabled("- [ЛКМ на міні-карті] - перехід або панорамування");
      ImGui::TextDisabled("- [Колесо миші] - масштабування в точку курсора");
      ImGui::TextDisabled(
          "- [Колесо на міні-карті] - масштабування загальної карти");
      ImGui::TextDisabled("- [F11] - увімкнути / вимкнути повний екран");
      ImGui::Spacing();
    }

    ImGui::Dummy(ImVec2(0.0f, 12.0f * config.uiScale));

    // 2. Maze configuration
    if (renderSectionHeader("2.  Налаштування лабіринту BSP", config.uiScale,
                            true)) {
      ImGui::Spacing();

      ImGui::Checkbox("Власний сід", &state.customSeedEnabled);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Увімкнути генерацію лабіринту за фіксованим числовим насінням "
            "(сідом).\n"
            "Дозволяє точно відтворити будь-яку згенеровану раніше мапу.");
      }

      if (state.customSeedEnabled) {
        ImGui::PushStyleVar(
            ImGuiStyleVar_FramePadding,
            ImVec2(6.0f * config.uiScale, 5.0f * config.uiScale));
        float btnPasteWidth = ImGui::CalcTextSize(" Вставити з буфера").x +
                              16.0f * config.uiScale;
        float availW = ImGui::GetContentRegionAvail().x;
        float inputWidth = std::max(
            120.0f, availW - btnPasteWidth - ImGui::GetStyle().ItemSpacing.x);

        ImGui::SetNextItemWidth(inputWidth);
        ImGui::InputTextWithHint("##custom_seed_input", "Введіть число сіду...",
                                 state.customSeedInput,
                                 sizeof(state.customSeedInput),
                                 ImGuiInputTextFlags_CharsDecimal);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Введіть число сіду для генерації лабіринту.");
        }

        ImGui::SameLine();
        if (ImGui::Button(" Вставити з буфера", ImVec2(btnPasteWidth, 0))) {
          const char *clip = ImGui::GetClipboardText();
          if (clip) {
            std::string digits;
            for (const char *c = clip; *c; ++c) {
              if (*c >= '0' && *c <= '9') {
                digits.push_back(*c);
              }
            }
            if (!digits.empty()) {
              snprintf(state.customSeedInput, sizeof(state.customSeedInput),
                       "%s", digits.c_str());
            }
          }
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Вставити число сіду із буфера обміну Windows.");
        }
        ImGui::PopStyleVar();
      }
      ImGui::Spacing();

      auto beginChangedWidgetBorder = [](bool changed) {
        if (changed) {
          ImGui::PushStyleColor(ImGuiCol_Border,
                                ImVec4(1.00f, 0.82f, 0.20f, 1.00f));
          ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        }
      };
      auto endChangedWidgetBorder = [](bool changed) {
        if (changed) {
          ImGui::PopStyleVar();
          ImGui::PopStyleColor();
        }
      };

      bool mazeSizeChanged = (config.mapWidth != maze.getWidth());
      beginChangedWidgetBorder(mazeSizeChanged);
      ImGui::SliderInt("Розмір (N x N)", &config.mapWidth, 20, 100);
      endChangedWidgetBorder(mazeSizeChanged);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Розмір лабіринту по ширині та висоті (кількість "
                          "клітинок від 20x20 до 100x100).");
      }
      config.mapHeight = config.mapWidth; // Square grid

      auto triggerLabyrinthGeneration = [&]() {
        if (state.isTransitionActive)
          return;
        bool hasValidSeed = false;
        if (state.customSeedEnabled) {
          std::string seedStr = state.customSeedInput;
          seedStr.erase(0, seedStr.find_first_not_of(" \t\n\r"));
          seedStr.erase(seedStr.find_last_not_of(" \t\n\r") + 1);

          if (!seedStr.empty()) {
            try {
              unsigned long val = std::stoul(seedStr);
              state.useSpecifiedSeed = true;
              state.specifiedSeedValue = static_cast<unsigned int>(val);
              hasValidSeed = true;
            } catch (...) {
              hasValidSeed = false;
            }
          }

          if (!hasValidSeed) {
            // Uncheck checkbox if no seed was provided, and proceed with random
            // generation
            state.customSeedEnabled = false;
            state.useSpecifiedSeed = false;
            state.customSeedInput[0] = '\0';
          }
        } else {
          state.useSpecifiedSeed = false;
        }
        state.triggerRegeneration = true;
        sidebarCollapsed_ = true;
      };

      bool mazeNeedsRegenerate = mazeSizeChanged;
      if (mazeNeedsRegenerate) {
        ImGui::Spacing();
        std::string regenText = "Змінено:";
        if (mazeSizeChanged) {
          char szBuf[64];
          snprintf(szBuf, sizeof(szBuf), " розмір (%dx%d -> %dx%d);",
                   maze.getWidth(), maze.getWidth(), config.mapWidth,
                   config.mapWidth);
          regenText += szBuf;
        }
        regenText += " Потрібна генерація:";
        ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.20f, 1.0f), "%s",
                           regenText.c_str());

        float pulse =
            0.5f + 0.5f * std::sin(static_cast<float>(ImGui::GetTime()) * 6.0f);
        ImVec4 btnColor = lerpColor(ImVec4(0.85f, 0.30f, 0.12f, 0.95f),
                                    ImVec4(1.00f, 0.55f, 0.15f, 1.00f), pulse);
        ImVec4 btnHover = ImVec4(1.00f, 0.68f, 0.22f, 1.00f);
        ImVec4 btnActive = ImVec4(1.00f, 0.40f, 0.10f, 1.00f);

        ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, btnHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, btnActive);
        ImGui::PushStyleColor(ImGuiCol_Border,
                              ImVec4(1.00f, 0.88f, 0.35f, 1.00f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

        char btnLabel[128];
        snprintf(btnLabel, sizeof(btnLabel), " ЗГЕНЕРУВАТИ ЛАБІРИНТ (%dx%d)",
                 config.mapWidth, config.mapWidth);
        if (state.isTransitionActive) {
          ImGui::BeginDisabled();
          ImGui::Button(" [ СКАНУВАННЯ КОМПЛЕКСУ... ]",
                        ImVec2(-1, std::max(34.0f * config.uiScale,
                                            ImGui::GetFrameHeight())));
          ImGui::EndDisabled();
        } else {
          if (ImGui::Button(btnLabel,
                            ImVec2(-1, std::max(34.0f * config.uiScale,
                                                ImGui::GetFrameHeight())))) {
            triggerLabyrinthGeneration();
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "Створити новий процедурний лабіринт розміром %dx%d клітинок.",
                config.mapWidth, config.mapWidth);
          }
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
      } else {
        if (state.isTransitionActive) {
          ImGui::BeginDisabled();
          ImGui::Button(" [ Сканування... ]",
                        ImVec2(-1, std::max(32.0f * config.uiScale,
                                            ImGui::GetFrameHeight())));
          ImGui::EndDisabled();
        } else {
          if (ImGui::Button(" Згенерувати новий лабіринт",
                            ImVec2(-1, std::max(32.0f * config.uiScale,
                                                ImGui::GetFrameHeight())))) {
            triggerLabyrinthGeneration();
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "Створити новий процедурний лабіринт алгоритмом BSP (нове "
                "планування кімнат, коридорів, багнюки та брухту).");
          }
        }
      }
      ImGui::Spacing();

      // Display the seed of the currently generated labyrinth (gray text,
      // underlined number, hover glow, click-to-copy, selectable)
      unsigned int currentSeed =
          maze.getSeed() != 0 ? maze.getSeed() : config.randomSeed;
      char seedStr[32];
      snprintf(seedStr, sizeof(seedStr), "%u", currentSeed);

      static float s_seedCopiedFeedbackTime = 0.0f;
      if (s_seedCopiedFeedbackTime > 0.0f) {
        s_seedCopiedFeedbackTime -= ImGui::GetIO().DeltaTime;
      }

      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.70f, 1.0f));
      ImGui::TextUnformatted("Насіння поточного лабіринту:");
      ImGui::PopStyleColor();

      if (ImGui::IsItemClicked()) {
        ImGui::SetClipboardText(seedStr);
        s_seedCopiedFeedbackTime = 2.0f;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Натисніть для копіювання насіння (%s)", seedStr);
      }

      ImGui::SameLine();

      float actualTextWidth = ImGui::CalcTextSize(seedStr).x;
      ImGui::SetNextItemWidth(actualTextWidth);

      // Frameless, completely transparent text widget looking like pure text
      ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
      ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,
                            ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
      ImGui::PushStyleColor(ImGuiCol_FrameBgActive,
                            ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
      ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

      bool isCopied = (s_seedCopiedFeedbackTime > 0.0f);
      ImVec4 numColor = isCopied ? ImVec4(0.35f, 0.95f, 0.55f, 1.0f)
                                 : ImVec4(0.88f, 0.88f, 0.92f, 1.0f);
      ImGui::PushStyleColor(ImGuiCol_Text, numColor);

      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

      char editBuf[32];
      snprintf(editBuf, sizeof(editBuf), "%s", seedStr);
      ImGui::InputText("##current_seed_val", editBuf, sizeof(editBuf),
                       ImGuiInputTextFlags_ReadOnly |
                           ImGuiInputTextFlags_AutoSelectAll);

      bool itemHovered = ImGui::IsItemHovered();
      if (ImGui::IsItemClicked()) {
        ImGui::SetClipboardText(seedStr);
        s_seedCopiedFeedbackTime = 2.0f;
      }

      // Draw elegant underline strictly below the digits (no empty space to the
      // right)
      ImVec2 rMin = ImGui::GetItemRectMin();
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      ImU32 lineCol;
      if (isCopied) {
        lineCol = IM_COL32(90, 240, 140, 230);
      } else if (itemHovered) {
        lineCol =
            IM_COL32(255, 215, 120, 230); // Subtle warm tactical glow on hover
      } else {
        lineCol = IM_COL32(160, 160, 175, 130); // Refined subdued underline
      }
      float lineY = ImGui::GetItemRectMax().y;
      drawList->AddLine(ImVec2(rMin.x, lineY),
                        ImVec2(rMin.x + actualTextWidth, lineY), lineCol,
                        itemHovered ? 1.5f : 1.0f);

      ImGui::PopStyleVar(2);
      ImGui::PopStyleColor(5);

      if (itemHovered) {
        if (isCopied) {
          ImGui::SetTooltip("Скопійовано в буфер обміну: %s", seedStr);
        } else {
          ImGui::SetTooltip(
              "Натисніть для копіювання або виділіть курсором: %s", seedStr);
        }
      }

      if (isCopied) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.35f, 0.90f, 0.55f, 1.0f), "[Скопійовано!]");
      }

      ImGui::Spacing();
    }

    ImGui::Dummy(ImVec2(0.0f, 12.0f * config.uiScale));

    // 3. Knapsack and algorithm selection
    if (renderSectionHeader("3.  Керування експедицією", config.uiScale,
                            true)) {
      ImGui::Spacing();

      auto beginChangedWidgetBorder = [](bool changed) {
        if (changed) {
          ImGui::PushStyleColor(ImGuiCol_Border,
                                ImVec4(1.00f, 0.82f, 0.20f, 1.00f));
          ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        }
      };
      auto endChangedWidgetBorder = [](bool changed) {
        if (changed) {
          ImGui::PopStyleVar();
          ImGui::PopStyleColor();
        }
      };

      const auto &items = maze.getItems();
      int totalMazeWeight = std::max(20, maze.getTotalItemWeight());

      // Helper: calculate maximum scrap value that fits within capacity
      auto calcMaxValForCap = [&items](int cap) -> int {
        if (cap <= 0 || items.empty())
          return 0;
        std::vector<const core::Item *> ptrs;
        ptrs.reserve(items.size());
        for (const auto &it : items)
          ptrs.push_back(&it);
        std::sort(ptrs.begin(), ptrs.end(),
                  [](const core::Item *a, const core::Item *b) {
                    return a->getValueDensity() > b->getValueDensity();
                  });
        int curW = 0, curV = 0;
        for (const auto *it : ptrs) {
          if (curW + it->weight <= cap) {
            curW += it->weight;
            curV += it->value;
          }
        }
        return curV;
      };

      // Helper: calculate minimum capacity required to collect at least
      // targetQuota
      auto calcMinCapForQuota = [&items](int quota) -> int {
        if (quota <= 0 || items.empty())
          return 10;
        std::vector<const core::Item *> ptrs;
        ptrs.reserve(items.size());
        for (const auto &it : items)
          ptrs.push_back(&it);
        std::sort(ptrs.begin(), ptrs.end(),
                  [](const core::Item *a, const core::Item *b) {
                    return a->getValueDensity() > b->getValueDensity();
                  });
        int curW = 0, curV = 0;
        for (const auto *it : ptrs) {
          curW += it->weight;
          curV += it->value;
          if (curV >= quota) {
            return std::max(10, curW);
          }
        }
        return std::max(10, curW);
      };

      // 1. Quota Slider (First in Section 3)
      // Maximum achievable quota depends directly on the current backpack capacity (or total maze weight if unlimited)
      int effectiveCapForQuota = config.backpackLimitEnabled
                                     ? config.manualBackpackCapacity
                                     : std::max(10, totalMazeWeight);
      int maxQuotaForCurCap = calcMaxValForCap(effectiveCapForQuota);
      int qSliderMin = 50;
      int qSliderMax = std::max(100, maxQuotaForCurCap);

      // Natural hard clamp: quota can never exceed what current backpack capacity can hold
      if (config.companyQuota > qSliderMax) {
        config.companyQuota = qSliderMax;
      }
      if (config.companyQuota < qSliderMin) {
        config.companyQuota = qSliderMin;
      }

      bool quotaChanged = (config.companyQuota != state.appliedCompanyQuota);
      ImGui::Text("Квота Компанії (грн):");
      beginChangedWidgetBorder(quotaChanged);
      ImGui::SetNextItemWidth(-1.0f);
      ImGui::SliderInt("##CompanyQuota", &config.companyQuota, qSliderMin,
                           qSliderMax);
      endChangedWidgetBorder(quotaChanged);
      if (ImGui::IsItemHovered()) {
        if (config.backpackLimitEnabled) {
          ImGui::SetTooltip(
              "Мінімальна фінансова вимога Компанії за місію (від %d до %d грн).\n"
              "Максимальна квота обмежена місткістю рюкзака (%d грн для %d кг).\n"
              "Щоб виставити більшу квоту, збільшіть місткість рюкзака нижче.",
              qSliderMin, qSliderMax, maxQuotaForCurCap,
              config.manualBackpackCapacity);
        } else {
          ImGui::SetTooltip(
              "Мінімальна фінансова вимога Компанії за місію (від %d до %d грн).\n"
              "При увімкненому необмеженому рюкзаку доступна повна вартість брухту на мапі.",
              qSliderMin, qSliderMax);
        }
      }

      // 2. Energy Slider (Directly after Quota)
      ImGui::Spacing();
      ImGui::Text("Запас енергії:");
      bool energyChanged = (config.initialEnergy != state.appliedInitialEnergy);
      beginChangedWidgetBorder(energyChanged);
      ImGui::SetNextItemWidth(-1.0f);
      ImGui::SliderInt("##InitialEnergy", &config.initialEnergy, 100, 4000);
      endChangedWidgetBorder(energyChanged);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Початковий запас енергії робітника. Витрата: 10 прямо, 14 по "
            "діагоналі, +15 у багнюці, помножено на вагу вантажу.");
      }

      // 3. Auto-fit Energy Button
      if (ImGui::Button(" [+] Підігнати енергію під поточний маршрут",
                        ImVec2(-1, std::max(26.0f * config.uiScale,
                                            ImGui::GetFrameHeight())))) {
        int safeReserve =
            std::max(120, static_cast<int>(std::round(
                              engine.getPlan().totalEnergyCost * 0.22f)));
        config.initialEnergy =
            std::max(config.initialEnergy,
                     engine.getPlan().totalEnergyCost + safeReserve);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Автоматично встановити запас енергії, достатній для "
                          "повного проходження розрахованого маршруту "
                          "(вартість шляху + 22%% запасу).");
      }

      // 4. Backpack Capacity Slider (Directly after Energy)
      ImGui::Spacing();
      ImGui::Text("Місткість рюкзака (кг):");
      int minCapNeededForQuota = calcMinCapForQuota(config.companyQuota);
      int capSliderMin = 10;
      int capSliderMax = std::max(500, totalMazeWeight);
      bool capacityChanged =
          (config.manualBackpackCapacity != state.appliedBackpackCapacity);
      beginChangedWidgetBorder(capacityChanged);
      ImGui::SetNextItemWidth(-1.0f);
      if (ImGui::SliderInt("##ManualBackpackCapacity",
                           &config.manualBackpackCapacity, capSliderMin,
                           capSliderMax)) {
        // When capacity is reduced, immediately clamp quota so it never exceeds new capacity
        if (config.backpackLimitEnabled) {
          int newMaxVal = calcMaxValForCap(config.manualBackpackCapacity);
          if (config.companyQuota > newMaxVal) {
            config.companyQuota = std::max(50, newMaxVal);
          }
        }
      }
      endChangedWidgetBorder(capacityChanged);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Максимальна вантажопідйомність робітника (від 10 до %d кг).\n"
                          "Для поточної квоти (%d грн) потрібно щонайменше %d кг.",
                          capSliderMax, config.companyQuota, minCapNeededForQuota);
      }

      int plannedRouteWeight = 0;
      for (const auto &wp : engine.getPlan().visitOrder) {
        if (wp.itemId >= 0) {
          plannedRouteWeight += wp.itemWeight;
        }
      }
      int fitCapTarget = std::clamp(plannedRouteWeight, capSliderMin, capSliderMax);

      // 5. Auto-fit Backpack Capacity Button
      if (ImGui::Button(" [+] Підігнати місткість під поточний маршрут",
                        ImVec2(-1, std::max(26.0f * config.uiScale,
                                            ImGui::GetFrameHeight())))) {
        config.manualBackpackCapacity = fitCapTarget;
        if (config.backpackLimitEnabled) {
          int newMaxVal = calcMaxValForCap(config.manualBackpackCapacity);
          if (config.companyQuota > newMaxVal) {
            config.companyQuota = std::max(50, newMaxVal);
          }
        }
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Автоматично встановити місткість рюкзака, достатню для повного\n"
            "збору запланованого брухту на поточному маршруті (%d кг).",
            plannedRouteWeight);
      }

      ImGui::Spacing();
      ImGui::Text("Алгоритм рюкзака:");
      const char *knapsackNames[] = {"0/1 Динамічне програмування (Точний)",
                                     "Жадібний за питомою цінністю (V/W)"};
      bool knapsackAlgoChanged =
          (state.knapsackAlgoIndex != state.appliedKnapsackAlgoIndex);
      beginChangedWidgetBorder(knapsackAlgoChanged);
      ImGui::SetNextItemWidth(-1.0f);
      ImGui::Combo("##KnapsackAlgo", &state.knapsackAlgoIndex, knapsackNames,
                   IM_ARRAYSIZE(knapsackNames));
      endChangedWidgetBorder(knapsackAlgoChanged);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "0/1 ДП - знаходить точний глобальний оптимум. Жадібний - швидка "
            "евристика за співвідношенням ціна/вага.");
      }

      ImGui::Spacing();
      ImGui::Text("Пошук шляху:");
      const char *pathfinderNames[] = {
          "A* (Октагональна евристика, 8 напрямків)",
          "Алгоритм Дейкстри (Найменша енергія)",
          "Пошук у ширину (BFS, найменше клітинок)"};
      bool pathAlgoChanged =
          (state.pathfinderAlgoIndex != state.appliedPathfinderAlgoIndex);
      beginChangedWidgetBorder(pathAlgoChanged);
      ImGui::SetNextItemWidth(-1.0f);
      ImGui::Combo("##PathfinderAlgo", &state.pathfinderAlgoIndex,
                   pathfinderNames, IM_ARRAYSIZE(pathfinderNames));
      endChangedWidgetBorder(pathAlgoChanged);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "A* - спрямований евристичний пошук з октагональною відстанню. "
            "Дейкстра - пошук з точними вагами. BFS - хвильовий пошук.");
      }

      ImGui::Spacing();

      bool energyLimitChanged =
          (config.energyLimitEnabled != state.appliedEnergyLimitEnabled);
      beginChangedWidgetBorder(energyLimitChanged);
      bool unlimitedEnergy = !config.energyLimitEnabled;
      if (ImGui::Checkbox("Необмежена енергія", &unlimitedEnergy)) {
        config.energyLimitEnabled = !unlimitedEnergy;
      }
      endChangedWidgetBorder(energyLimitChanged);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Якщо увімкнено - алгоритм розраховується без ліміту енергії.");
      }

      bool backpackLimitChanged =
          (config.backpackLimitEnabled != state.appliedBackpackLimitEnabled);
      beginChangedWidgetBorder(backpackLimitChanged);
      bool unlimitedBackpack = !config.backpackLimitEnabled;
      if (ImGui::Checkbox("Необмежений рюкзак", &unlimitedBackpack)) {
        config.backpackLimitEnabled = !unlimitedBackpack;
      }
      endChangedWidgetBorder(backpackLimitChanged);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Якщо увімкнено - алгоритм розраховується без ліміту місця в рюкзаку.");
      }

      ImGui::Spacing();

      bool routeModeChanged = (config.routeMode != state.appliedRouteMode);
      int routeModeInt =
          (config.routeMode == core::RouteMode::PointAToPointB) ? 0 : 1;
      ImGui::Text("Режим евакуації:");
      beginChangedWidgetBorder(routeModeChanged);
      if (ImGui::RadioButton("Точка A -> B (Вихід)", &routeModeInt, 0)) {
        config.routeMode = core::RouteMode::PointAToPointB;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Маршрут через відібраний брухт до окремого шлюзу "
                          "виходу (точка B).");
      }

      ImGui::SameLine();
      if (ImGui::RadioButton("A -> A (База)", &routeModeInt, 1)) {
        config.routeMode = core::RouteMode::RoundTrip;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Круговий маршрут: старт зі шлюзу (A), збір "
                          "цінностей та повернення назад до шлюзу (A).");
      }
      endChangedWidgetBorder(routeModeChanged);

      bool planNeedsRecalculate =
          (state.knapsackAlgoIndex != state.appliedKnapsackAlgoIndex ||
           state.pathfinderAlgoIndex != state.appliedPathfinderAlgoIndex ||
           config.manualBackpackCapacity != state.appliedBackpackCapacity ||
           config.companyQuota != state.appliedCompanyQuota ||
           config.initialEnergy != state.appliedInitialEnergy ||
           config.routeMode != state.appliedRouteMode || energyLimitChanged ||
           backpackLimitChanged);

      if (planNeedsRecalculate) {
        ImGui::Spacing();
        std::string changedText = "Змінено:";
        if (state.knapsackAlgoIndex != state.appliedKnapsackAlgoIndex)
          changedText += " рюкзак;";
        if (state.pathfinderAlgoIndex != state.appliedPathfinderAlgoIndex)
          changedText += " пошук шляху;";
        if (config.companyQuota != state.appliedCompanyQuota)
          changedText += " квота;";
        if (config.manualBackpackCapacity != state.appliedBackpackCapacity)
          changedText += " місткість;";
        if (config.initialEnergy != state.appliedInitialEnergy)
          changedText += " енергія;";
        if (config.routeMode != state.appliedRouteMode)
          changedText += " режим евакуації;";
        if (energyLimitChanged)
          changedText += config.energyLimitEnabled ? " витрата енергії;"
                                                   : " необмежена енергія;";
        if (backpackLimitChanged)
          changedText += config.backpackLimitEnabled ? " ліміт рюкзака;"
                                                     : " необмежений рюкзак;";
        ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.20f, 1.0f), "%s",
                           changedText.c_str());

        float pulse =
            0.5f + 0.5f * std::sin(static_cast<float>(ImGui::GetTime()) * 6.0f);
        ImVec4 btnColor = lerpColor(ImVec4(0.85f, 0.28f, 0.16f, 0.95f),
                                    ImVec4(1.00f, 0.52f, 0.18f, 1.00f), pulse);
        ImVec4 btnHover = ImVec4(1.00f, 0.65f, 0.25f, 1.00f);
        ImVec4 btnActive = ImVec4(1.00f, 0.40f, 0.12f, 1.00f);

        ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, btnHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, btnActive);
        ImGui::PushStyleColor(ImGuiCol_Border,
                              ImVec4(1.00f, 0.88f, 0.35f, 1.00f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

        if (ImGui::Button(" ПЕРЕРАХУВАТИ ОПТИМАЛЬНИЙ ПЛАН",
                          ImVec2(-1, std::max(34.0f * config.uiScale,
                                              ImGui::GetFrameHeight())))) {
          state.appliedKnapsackAlgoIndex = state.knapsackAlgoIndex;
          state.appliedPathfinderAlgoIndex = state.pathfinderAlgoIndex;
          state.appliedBackpackCapacity = config.manualBackpackCapacity;
          state.appliedCompanyQuota = config.companyQuota;
          state.appliedInitialEnergy = config.initialEnergy;
          state.appliedRouteMode = config.routeMode;
          state.appliedEnergyLimitEnabled = config.energyLimitEnabled;
          state.appliedBackpackLimitEnabled = config.backpackLimitEnabled;
          state.triggerRecalculate = true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Заново розрахувати рюкзак та TSP-маршрут для "
                            "нових обраних параметрів.");
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.2f, 1.0f),
                           "[~] Очікує перерахунку (параметри не застосовано)");
      } else {
        if (ImGui::Button(" Перерахувати оптимальний план",
                          ImVec2(-1, std::max(30.0f * config.uiScale,
                                              ImGui::GetFrameHeight())))) {
          state.triggerRecalculate = true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Заново розрахувати рюкзак та TSP-маршрут для "
                            "поточних параметрів карти.");
        }

        const auto &curPlan = engine.getPlan();
        if (curPlan.routeValid) {
          int targets = curPlan.visitOrder.size() > 2
                            ? static_cast<int>(curPlan.visitOrder.size()) - 2
                            : 0;
          ImGui::TextColored(
              ImVec4(0.2f, 1.0f, 0.4f, 1.0f),
              "План актуальний: %d предм. | %d кроків | %d од. енергії",
              targets, curPlan.totalSteps, curPlan.totalEnergyCost);
        }
      }

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      ImGui::TextColored(ImVec4(0.40f, 0.85f, 1.0f, 1.0f), "JSON Конфігурація та Звітність:");

      float jsonBtnW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
      float jsonBtnH = std::max(28.0f * config.uiScale, ImGui::GetFrameHeight());

      if (ImGui::Button(" ↓ Завантажити input_config.json", ImVec2(jsonBtnW, jsonBtnH))) {
        state.triggerImportJson = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Завантажити та застосувати параметри карти, робітника\n"
                          "та обраних алгоритмів із файлу 'input_config.json'.");
      }

      ImGui::SameLine();
      if (ImGui::Button(" ↓ Зберегти конфіг у JSON", ImVec2(jsonBtnW, jsonBtnH))) {
        state.triggerSaveConfigJson = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Зберегти поточні налаштування місії, розміри карти, сід,\n"
                          "квоту та алгоритми у файл 'input_config.json'.");
      }

      ImGui::Spacing();
      if (ImGui::Button(" ↓ Експортувати report_route.json", ImVec2(-1, jsonBtnH))) {
        state.triggerExportReportJson = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Експортувати повний структурований звіт оптимізації рюкзака,\n"
                          "маршруту TSP, координат траєкторії та бенчмарків у 'report_route.json'.");
      }

      ImGui::Spacing();
    }

    ImGui::Dummy(ImVec2(0.0f, 12.0f * config.uiScale));

    // 4. Simulation playback controls
    if (renderSectionHeader("4.  Керування симуляцією", config.uiScale, true)) {
      ImGui::Spacing();
      const auto &agent = engine.getAgent();

      if (engine.getExtraCollectedCount() > 0) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f),
                           "[+] Попутно зібрано: +%zu од. брухту",
                           engine.getExtraCollectedCount());
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(
              "На шляху агента лежав брухт, який помістився у рюкзак,\n"
              "тому агент зібрав більше, ніж планувалося спочатку.");
        }
      }

      bool atEvacOrSuccess =
          engine.isAtEvacuationCell() ||
          agent.getStatus() == simulation::AgentStatus::Success;
      bool canStepBack =
          engine.getPlan().routeValid && (!engine.isManualMode() || atEvacOrSuccess);
      bool canControl = engine.getPlan().routeValid && !engine.isManualMode();
      const char *disabledReason =
          engine.isManualMode()
              ? "Режим: Ручне керування. Автоматичне керування недоступне."
              : "Маршрут недоступний або не розрахований.";

      float btnW = (ImGui::GetContentRegionAvail().x - 18) / 4.0f;
      float btnH = std::max(30.0f * config.uiScale, ImGui::GetFrameHeight());

      if (!canStepBack) {
        ImGui::BeginDisabled(true);
      }
      if (ImGui::Button(" [ < ]", ImVec2(btnW, btnH))) {
        if (!engine.isPaused()) {
          engine.setPaused(true);
        }
        engine.stepBackward();
        if (config.cameraFollowAgent) {
          state.requestCenterCameraOnAgent = true;
        }
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        showStyledTooltip(config.uiScale, "%s",
                          canStepBack
                              ? "Крок назад [Shift+Space]: повернутися на 1 "
                                "крок симуляції назад."
                              : disabledReason);
      }
      if (!canStepBack) {
        ImGui::EndDisabled();
      }

      if (!canControl) {
        ImGui::BeginDisabled(true);
      }

      ImGui::SameLine();
      const char *playPauseText =
          engine.isPaused() ? " [    ]##PlayPause" : " [ || ]##PlayPause";
      if (ImGui::Button(playPauseText, ImVec2(btnW, btnH))) {
        engine.togglePlayPause();
        if (!engine.isPaused()) {
          config.cameraFollowAgent = true;
          state.requestCenterCameraOnAgent = true;
        }
      }
      if (engine.isPaused()) {
        ImVec2 rMin = ImGui::GetItemRectMin();
        ImVec2 rMax = ImGui::GetItemRectMax();
        float leftPrefixW = ImGui::CalcTextSize(" [").x;
        float rightSuffixW = ImGui::CalcTextSize("]").x;
        float cx = (rMin.x + rMax.x + leftPrefixW - rightSuffixW) * 0.5f;
        float cy = (rMin.y + rMax.y) * 0.5f;
        float triHalfH = 5.2f * config.uiScale;
        ImU32 iconCol = ImGui::GetColorU32(
            canControl ? ImGuiCol_Text : ImGuiCol_TextDisabled);
        ImGui::GetWindowDrawList()->AddTriangleFilled(
            ImVec2(cx - 4.2f * config.uiScale, cy - triHalfH),
            ImVec2(cx + 5.3f * config.uiScale, cy),
            ImVec2(cx - 4.2f * config.uiScale, cy + triHalfH), iconCol);
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        showStyledTooltip(config.uiScale, "%s",
                          canControl
                              ? (engine.isPaused()
                                     ? "Старт: запустити автоматичний рух робітника."
                                     : "Пауза: призупинити рух робітника.")
                              : disabledReason);
      }

      ImGui::SameLine();
      if (ImGui::Button(" [ > ]", ImVec2(btnW, btnH))) {
        if (!engine.isPaused()) {
          engine.setPaused(true);
        }
        engine.stepForward();
        if (config.cameraFollowAgent) {
          state.requestCenterCameraOnAgent = true;
        }
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        showStyledTooltip(
            config.uiScale, "%s",
            canControl
                ? "Крок вперед [Space]: виконати 1 одиночний крок по маршруту."
                : disabledReason);
      }

      ImGui::SameLine();
      if (ImGui::Button(" [ >> ]", ImVec2(btnW, btnH))) {
        engine.instantFinish();
        state.requestFitMazeInView = true;
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        showStyledTooltip(
            config.uiScale, "%s",
            canControl ? "Завершити миттєво: пройти весь залишок маршруту до "
                         "кінця та віддалити огляд на весь лабіринт."
                       : disabledReason);
      }

      if (!canControl) {
        ImGui::EndDisabled();
        ImGui::TextColored(
            engine.isManualMode() ? ImVec4(1.0f, 0.78f, 0.25f, 1.0f)
                                  : ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
            "%s", disabledReason);
      }

      if (engine.getConfig().energyLimitEnabled &&
          agent.getStatus() == simulation::AgentStatus::Exhausted) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
                           "Шлях вимагає %d од. енергії (було %d)!",
                           engine.getPlan().totalEnergyCost,
                           config.initialEnergy);
        if (ImGui::Button(" [+] Додати енергію (+300 од.) та продовжити",
                          ImVec2(-1, std::max(28.0f * config.uiScale,
                                              ImGui::GetFrameHeight())))) {
          engine.replenishEnergy(300);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Миттєво додати робітнику 300 одиниць енергії та "
                            "продовжити виконання маршруту.");
        }
      }

      float speed = engine.getSpeedMultiplier();
      if (ImGui::SliderFloat("Швидкість ходу", &speed, 0.5f, 4.0f, "%.1fx")) {
        engine.setSpeedMultiplier(speed);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Швидкість автоматичного крокування симуляції (від "
                          "0.5x до 4.0x). Базовий крок: 0.75 с.");
      }

      ImGui::Text("Крок симуляції: %zu / %zu", engine.getCurrentStepIndex(),
                  engine.getTotalSteps());

      ImGui::Spacing();
    }

    ImGui::Dummy(ImVec2(0.0f, 12.0f * config.uiScale));

    // 5. Benchmark analytics
    if (renderSectionHeader("5.  Порівняльний аналіз", config.uiScale, true)) {
      ImGui::Spacing();
      if (ImGui::Button(" Запустити повний аналіз (усі 6)",
                        ImVec2(-1, std::max(30.0f * config.uiScale,
                                            ImGui::GetFrameHeight())))) {
        state.triggerBatchBenchmark = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Запустити точне порівняльне профілювання всіх 6 алгоритмів "
            "(Knapsack DP/Greedy, A*, Dijkstra, BFS, TSP + 2-Opt).");
      }

      if (!benchmarkRows.empty()) {
        if (ImGui::BeginTable("ТаблицяМетрик", 4,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_SizingStretchProp |
                                  ImGuiTableFlags_Resizable)) {
          ImGui::TableSetupColumn("Алгоритм",
                                  ImGuiTableColumnFlags_WidthStretch, 0.44f);
          ImGui::TableSetupColumn("Час (мс)",
                                  ImGuiTableColumnFlags_WidthStretch, 0.13f);
          ImGui::TableSetupColumn("Кроки/Енергія",
                                  ImGuiTableColumnFlags_WidthStretch, 0.29f);
          ImGui::TableSetupColumn("Оптимум %",
                                  ImGuiTableColumnFlags_WidthStretch, 0.14f);
          ImGui::TableHeadersRow();

          for (const auto &row : benchmarkRows) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", row.algorithmName.c_str());

            ImGui::TableNextColumn();
            double timeMs = static_cast<double>(row.timeMicrosec) / 1000.0;
            ImGui::Text("%.3f", timeMs);
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("%.3f мс (%lld мкс)", timeMs, row.timeMicrosec);
            }

            ImGui::TableNextColumn();
            if (row.steps > 0 || row.energyCost > 0) {
              ImGui::Text("%d кроків / %d од", row.steps, row.energyCost);
            } else {
              ImGui::Text("%d кг / %d грн", row.weightKg, row.valueCredits);
            }

            ImGui::TableNextColumn();
            if (row.category == "Маршрутизація TSP") {
              ImGui::Text("%.1f %%*", row.qualityPercent);
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Ефективність 2-Opt відносно жадібного NN "
                                  "(+%.1f%% енергозбереження)",
                                  row.qualityPercent - 100.0);
              }
            } else {
              ImGui::Text("%.1f %%", row.qualityPercent);
            }
          }
          ImGui::EndTable();
        }
      }

      const auto &plan = engine.getPlan();
      if (plan.routeValid) {
        ImGui::Separator();
        ImGui::Text("Оптимізація TSP (2-Opt):");
        ImGui::Text(" - До 2-Opt: %d од. енергії", plan.energyBefore2Opt);
        ImGui::Text(" - Після 2-Opt: %d од. енергії", plan.totalEnergyCost);
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f),
                           " - Скорочення маршруту: +%.2f %%",
                           plan.twoOptGainPercent);
      }

      if (ImGui::Button(" Експорт таблиці у CSV файл",
                        ImVec2(-1, std::max(30.0f * config.uiScale,
                                            ImGui::GetFrameHeight())))) {
        state.triggerExportCsv = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Експортувати результати вимірювання продуктивності "
                          "алгоритмів у файл benchmark_results.csv.");
      }

      ImGui::Spacing();
      if (ImGui::Button(" Експорт повного звіту у JSON файл",
                        ImVec2(-1, std::max(30.0f * config.uiScale,
                                            ImGui::GetFrameHeight())))) {
        state.triggerExportReportJson = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Експортувати результати вимірювання, параметри карти та "
                          "траєкторію робітника у файл report_route.json.");
      }

      if (!state.statusNotice.empty()) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s",
                           state.statusNotice.c_str());
      }
      ImGui::Spacing();
    }

    sidebarScroller_.update(68.0f, config.uiScale);
    ImGui::SetScrollX(0.0f);
    ImGui::End();
  }

  void renderResizeHandle(float windowHeight, float uiScale = 1.0f,
                          bool canShowStartupHint = false) {
    float handleW = 18.0f * uiScale;
    float handleH = 158.0f * uiScale;

    float easeT = sidebarOpenProgress_ * sidebarOpenProgress_ *
                  (3.0f - 2.0f * sidebarOpenProgress_);
    float visualEdge = sidebarWidth_ * easeT;

    // Complete all shape morphing strictly within the first 25% of travel:
    // Once easeT >= 0.25f, handleWaveMorph_ is strictly 0.0f (clean pill
    // shape). All shape, color, and icon transitions finish BEFORE the handle
    // reaches the right side!
    float morphFactor = std::clamp((0.25f - easeT) / 0.25f, 0.0f, 1.0f);
    handleWaveMorph_ = morphFactor * morphFactor * (3.0f - 2.0f * morphFactor);

    // Position handle along the outer edge of the panel so it never overlaps
    // sidebar contents, and slide in cleanly from the left edge on HUD intro
    float hudEase = getHudIntroEase();
    float slideHide = (handleW + 14.0f * uiScale) * (1.0f - hudEase);
    float handleX = std::max(0.0f, visualEdge - 1.0f) - slideHide;
    float handleY = (windowHeight - handleH) * 0.5f;

    float flareH = 34.0f * uiScale * handleWaveMorph_;
    float winY = handleY - flareH;
    float winH = handleH + 2.0f * flareH;
    float winW = handleW;

    if (hudEase <= 0.001f) {
      return;
    }

    ImGui::SetNextWindowPos(ImVec2(handleX, winY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(winW, winH), ImGuiCond_Always);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.0f, 1.0f));

    if (ImGui::Begin("##SidebarGripWindow", nullptr, flags)) {
      ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
      ImGui::InvisibleButton("##GripBtn", ImVec2(winW, winH));

      bool isRawHovered = ImGui::IsItemHovered();
      bool isHovered = false;

      if (isRawHovered) {
        ImVec2 m = ImGui::GetIO().MousePos;
        if (handleWaveMorph_ <= 0.05f) {
          // When pill shape: strict rectangle bounds [handleX, handleX +
          // handleW] x [handleY, handleY + handleH]
          isHovered = (m.x >= handleX && m.x <= handleX + handleW &&
                       m.y >= handleY && m.y <= handleY + handleH);
        } else {
          // When wave shape: check main pill body or inside the cubic bezier
          // flares
          if (m.y >= handleY && m.y <= handleY + handleH) {
            isHovered = (m.x >= handleX && m.x <= handleX + handleW);
          } else if (m.y < handleY && m.y >= winY) {
            float span = flareH + 14.0f * uiScale;
            float t = span > 0.001f ? (m.y - winY) / span : 1.0f;
            t = std::clamp(t, 0.0f, 1.0f);
            float curveX = handleX + handleW * (t * t * (3.0f - 2.0f * t));
            isHovered = (m.x >= handleX && m.x <= curveX);
          } else if (m.y > handleY + handleH && m.y <= winY + winH) {
            float span = flareH + 14.0f * uiScale;
            float t = span > 0.001f ? ((winY + winH) - m.y) / span : 1.0f;
            t = std::clamp(t, 0.0f, 1.0f);
            float curveX = handleX + handleW * (t * t * (3.0f - 2.0f * t));
            isHovered = (m.x >= handleX && m.x <= curveX);
          }
        }
      }

      bool isActive = ImGui::IsItemActive();

      if (isHovered || isActive) {
        if (sidebarCollapsed_) {
          ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
          showStyledTooltip(uiScale,
                            "[Клік / Потягніть вправо / Esc]: Відкрити бічну "
                            "панель керування");
        } else {
          ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
          showStyledTooltip(
              uiScale,
              "[Клік / Esc]: Сховати бічну панель\n[Потягніть мишею]: Змінити "
              "ширину меню (640px .. 55%% екрану)");
        }
      }

      if (isActive) {
        float dx = ImGui::GetIO().MouseDelta.x;
        dragAccumX_ += std::abs(dx);
        if (dragAccumX_ > 4.0f) {
          isDraggingHandle_ = true;
        }

        if (isDraggingHandle_) {
          startupHintDismissed_ = true;
          if (sidebarCollapsed_) {
            sidebarCollapsed_ = false;
            sidebarWidth_ = std::max(MIN_SIDEBAR_WIDTH * uiScale,
                                     savedSidebarBaseWidth_ * uiScale);
          }
          sidebarWidth_ += dx;
          float minW = MIN_SIDEBAR_WIDTH * uiScale;
          float maxW = ImGui::GetIO().DisplaySize.x * 0.55f;
          sidebarWidth_ = std::clamp(sidebarWidth_, minW, maxW);
          savedSidebarBaseWidth_ =
              (uiScale > 0.001f) ? (sidebarWidth_ / uiScale) : sidebarWidth_;
        }
      }

      if (ImGui::IsItemDeactivated()) {
        if (!isDraggingHandle_) {
          sidebarCollapsed_ = !sidebarCollapsed_;
          if (!sidebarCollapsed_) {
            startupHintDismissed_ = true;
            sidebarWidth_ = std::max(MIN_SIDEBAR_WIDTH * uiScale,
                                     savedSidebarBaseWidth_ * uiScale);
          }
        }
        dragAccumX_ = 0.0f;
        isDraggingHandle_ = false;
      }

      ImDrawList *drawList = ImGui::GetWindowDrawList();

      // Smooth color transitions between dock and floating handle,
      // completing fully within the first 25% of movement
      float colorT = std::clamp(easeT / 0.25f, 0.0f, 1.0f);
      colorT = colorT * colorT * (3.0f - 2.0f * colorT);
      uint8_t baseR = static_cast<uint8_t>(65 - 19 * colorT);
      uint8_t baseG = static_cast<uint8_t>(26);
      uint8_t baseB = static_cast<uint8_t>(35 - 3 * colorT);
      uint8_t bdrR = static_cast<uint8_t>(245 - 135 * colorT);
      uint8_t bdrG = static_cast<uint8_t>(120 - 75 * colorT);
      uint8_t bdrB = static_cast<uint8_t>(40 + 18 * colorT);

      ImU32 normalBg = IM_COL32(baseR, baseG, baseB,
                                static_cast<uint8_t>(235 - 15 * colorT));
      ImU32 normalBorder =
          IM_COL32(bdrR, bdrG, bdrB, static_cast<uint8_t>(240 - 20 * colorT));

      ImU32 bgColor = isActive    ? IM_COL32(255, 140, 20, 245)
                      : isHovered ? IM_COL32(235, 48, 68, 240)
                                  : normalBg;

      ImU32 borderColor =
          (isHovered || isActive) ? IM_COL32(255, 255, 255, 255) : normalBorder;

      float topY = handleY;
      float botY = handleY + handleH;
      float cornerInset = 14.0f * uiScale * handleWaveMorph_;
      float curFlare = 34.0f * uiScale * handleWaveMorph_;

      if (handleWaveMorph_ > 0.01f) {
        // Flowing wave shape merging smoothly into the left wall (x = handleX)
        ImVec2 pTopWall(handleX, topY - curFlare);
        ImVec2 pTopHandle(handleX + handleW, topY + cornerInset);
        ImVec2 pTopCp1(handleX, topY - curFlare * 0.25f);
        ImVec2 pTopCp2(handleX + handleW, topY);

        ImVec2 pBotHandle(handleX + handleW, botY - cornerInset);
        ImVec2 pBotWall(handleX, botY + curFlare);
        ImVec2 pBotCp1(handleX + handleW, botY);
        ImVec2 pBotCp2(handleX, botY + curFlare * 0.25f);

        // Build the flowing curve path along the outer border
        drawList->PathClear();
        drawList->PathLineTo(pTopWall);
        drawList->PathBezierCubicCurveTo(pTopCp1, pTopCp2, pTopHandle, 24);
        drawList->PathLineTo(pBotHandle);
        drawList->PathBezierCubicCurveTo(pBotCp1, pBotCp2, pBotWall, 24);

        // Water-tight monotone quad strip fill: strictly bounded between
        // handleX and each curve point
        int nPts = drawList->_Path.Size;
        if (nPts >= 2) {
          const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
          drawList->PrimReserve((nPts - 1) * 6, nPts * 2);
          ImDrawIdx vtxBase = static_cast<ImDrawIdx>(drawList->_VtxCurrentIdx);

          for (int i = 0; i < nPts; ++i) {
            ImVec2 rPt = drawList->_Path[i];
            ImVec2 lPt(handleX, rPt.y);
            drawList->PrimWriteVtx(lPt, uv, bgColor);
            drawList->PrimWriteVtx(rPt, uv, bgColor);
          }

          for (int i = 0; i < nPts - 1; ++i) {
            ImDrawIdx i0 = vtxBase + static_cast<ImDrawIdx>(i * 2);
            ImDrawIdx i1 = i0 + 1;
            ImDrawIdx i2 = i0 + 2;
            ImDrawIdx i3 = i0 + 3;
            drawList->PrimWriteIdx(i0);
            drawList->PrimWriteIdx(i1);
            drawList->PrimWriteIdx(i3);
            drawList->PrimWriteIdx(i0);
            drawList->PrimWriteIdx(i3);
            drawList->PrimWriteIdx(i2);
          }
        }

        // Outer wave border stroke (consumes and clears drawList->_Path)
        drawList->PathStroke(borderColor, false, 1.8f);
      } else {
        // Rounded grip pill
        ImVec2 pMin(handleX, handleY);
        ImVec2 pMax(handleX + handleW, handleY + handleH);
        drawList->AddRectFilled(pMin, pMax, bgColor, 4.0f);
        drawList->AddRect(pMin, pMax, borderColor, 4.0f, 0, 1.5f);
      }

      // Draw chevron and grip ridges
      ImU32 iconColor = (isHovered || isActive) ? IM_COL32(20, 20, 20, 240)
                        : (easeT < 0.25f)       ? IM_COL32(255, 210, 120, 255)
                                                : IM_COL32(195, 120, 135, 220);

      float midX = handleX + handleW * 0.5f;
      float midY = handleY + handleH * 0.5f;
      float arrowSize = 4.5f * uiScale;

      if (easeT < 0.25f) {
        // Right-pointing chevron '>'
        drawList->AddLine(
            ImVec2(midX - arrowSize * 0.7f, midY - arrowSize * 1.4f),
            ImVec2(midX + arrowSize * 0.7f, midY), iconColor, 2.0f);
        drawList->AddLine(
            ImVec2(midX + arrowSize * 0.7f, midY),
            ImVec2(midX - arrowSize * 0.7f, midY + arrowSize * 1.4f), iconColor,
            2.0f);
      } else {
        // Left-pointing chevron '<'
        drawList->AddLine(
            ImVec2(midX + arrowSize * 0.7f, midY - arrowSize * 1.4f),
            ImVec2(midX - arrowSize * 0.7f, midY), iconColor, 2.0f);
        drawList->AddLine(
            ImVec2(midX - arrowSize * 0.7f, midY),
            ImVec2(midX + arrowSize * 0.7f, midY + arrowSize * 1.4f), iconColor,
            2.0f);
      }

      // Horizontal grip ridges above and below chevron
      float lineInset = 4.5f * uiScale;
      for (float off : {-46.0f * uiScale, -34.0f * uiScale, -22.0f * uiScale,
                        22.0f * uiScale, 34.0f * uiScale, 46.0f * uiScale}) {
        drawList->AddLine(ImVec2(handleX + lineInset, midY + off),
                          ImVec2(handleX + handleW - lineInset, midY + off),
                          iconColor, 1.5f);
      }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);

    // Initial orientation hint popup displayed to the right of the sidebar
    // toggle handle once HUD intro slide-in finishes
    if (hudIntroProgress_ >= 0.999f && canShowStartupHint &&
        !startupHintDismissed_ && sidebarCollapsed_) {
      float dt = ImGui::GetIO().DeltaTime;
      startupHintTimer_ += dt;
      float animT = std::clamp(startupHintTimer_ / 0.35f, 0.0f, 1.0f);
      float pulse = 0.5f + 0.5f * std::sin(startupHintTimer_ * 3.5f);

      float slideOffset = (1.0f - animT) * 16.0f;
      float hintX = handleX + handleW + 14.0f * uiScale + slideOffset;
      float hintY = handleY + (handleH * 0.5f) - 78.0f * uiScale;

      ImGui::SetNextWindowPos(ImVec2(hintX, hintY), ImGuiCond_Always);
      ImGui::SetNextWindowBgAlpha(0.94f * animT);

      ImGuiWindowFlags hintFlags =
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                          ImVec2(14.0f * uiScale, 11.0f * uiScale));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.6f);
      ImGui::PushStyleColor(ImGuiCol_WindowBg,
                            ImVec4(0.09f, 0.08f, 0.11f, 0.95f));
      ImGui::PushStyleColor(
          ImGuiCol_Border,
          ImVec4(0.95f, 0.70f + 0.15f * pulse, 0.20f, 0.90f * animT));

      if (ImGui::Begin("##FirstTimeSidebarGuide", nullptr, hintFlags)) {
        ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.25f, 1.0f), "[!] Вперше тут?");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.65f, 0.70f, 0.78f, 1.0f), "[Підказка]");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextUnformatted("Натисніть ");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(0.2f, 0.95f, 1.0f, 1.0f), "Esc");
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(" або на цю кнопку ліворуч,");
        ImGui::TextUnformatted("щоб відкрити бічну панель керування:");
        ImGui::TextDisabled("- Вибір алгоритмів пошуку шляху та TSP");
        ImGui::TextDisabled("- Генерація лабіринтів та аналіз");
        ImGui::TextDisabled("- Параметри швидкості та запас енергії");
        ImGui::TextDisabled("- Налаштування UI, тем оформлення та масштабу");
        ImGui::Spacing();

        if (ImGui::Button("Зрозуміло", ImVec2(-1.0f, 26.0f * uiScale))) {
          startupHintDismissed_ = true;
        }

        // Pointer triangle pointing left directly to the handle
        ImDrawList *hintDrawList = ImGui::GetWindowDrawList();
        ImVec2 pTip(hintX - 7.0f, handleY + handleH * 0.5f);
        ImVec2 pTop(hintX + 1.0f, handleY + handleH * 0.5f - 8.0f * uiScale);
        ImVec2 pBot(hintX + 1.0f, handleY + handleH * 0.5f + 8.0f * uiScale);
        hintDrawList->AddTriangleFilled(
            pTip, pTop, pBot,
            ImColor(
                ImVec4(0.95f, 0.70f + 0.15f * pulse, 0.20f, 0.90f * animT)));
      }
      ImGui::End();
      ImGui::PopStyleColor(2);
      ImGui::PopStyleVar(3);
    }
  }

  void renderTopCenteredItemTooltip(const core::Item &it,
                                    const sf::RenderWindow &window,
                                    const sf::View &view, float windowWidth,
                                    float windowHeight,
                                    bool isOpportunistic = false) {
    // Top-center world coordinate of the item's cell
    sf::Vector2f cellTopCenterWorld{
        (it.position.x + 0.5f) * ProceduralRenderer::TILE_SIZE,
        it.position.y * ProceduralRenderer::TILE_SIZE};

    sf::Vector2i screenPos = window.mapCoordsToPixel(cellTopCenterWorld, view);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.09f, 0.08f, 0.09f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.52f, 0.18f, 0.26f, 0.85f));

    ImGuiWindowFlags tipFlags =
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    float posX =
        std::clamp(static_cast<float>(screenPos.x), 16.0f, windowWidth - 16.0f);
    float posY = std::clamp(static_cast<float>(screenPos.y) - 8.0f, 16.0f,
                            windowHeight - 16.0f);

    // Pivot (0.5f, 1.0f) anchors the horizontal center and bottom of the
    // tooltip exactly above the top of the cell
    ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Always,
                            ImVec2(0.5f, 1.0f));

    if (ImGui::Begin("##TacticalSpotlightTooltip", nullptr, tipFlags)) {
      ImGui::PushTextWrapPos(std::min(460.0f, windowWidth - 32.0f));

      if (it.category == core::ItemCategory::LightScrap) {
        ImGui::TextColored(ImVec4(0.20f, 0.41f, 1.00f, 1.0f),
                           "Дрібний брухт: %s", it.name.c_str());
      } else if (it.category == core::ItemCategory::MediumScrap) {
        ImGui::TextColored(ImVec4(1.00f, 1.00f, 0.00f, 1.0f),
                           "Побутовий брухт: %s", it.name.c_str());
      } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Важкий брухт: %s",
                           it.name.c_str());
      }
      ImGui::Separator();
      ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.55f, 1.0f),
                         "Статус: Вже підібрано робітником у рюкзак");
      if (isOpportunistic) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f),
                           "Збір: Попутний брухт (лежав на шляху)");
      }
      ImGui::Text("Категорія: %s",
                  core::itemCategoryToString(it.category).c_str());
      ImGui::Text("Вага: %d кг", it.weight);
      ImGui::Text("Цінність: %d грн", it.value);
      ImGui::Text("Питома цінність (V/W): %.2f грн/кг", it.getValueDensity());
      ImGui::Text("Координати: [%d, %d]", it.position.x, it.position.y);

      ImGui::PopTextWrapPos();
      ImGui::End();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
  }

  void renderMinimapLegend(core::Config &config, const map::Maze &maze,
                           const simulation::SimulationEngine &engine,
                           CameraController &camera, sf::RenderWindow &window,
                           const ProceduralRenderer &renderer,
                           float windowWidth, float windowHeight) {
    hoveredCollectedItemId_ = -1;
    hoveredCatalogItemName_.clear();
    hoveredCatalogCategory_ = -1;

    float dt = ImGui::GetIO().DeltaTime;
    const float LEGEND_ANIM_DURATION = 0.24f; // Responsive 240ms transition matching sidebar
    if (config.showLegend) {
      legendAnimT_ = std::min(1.0f, legendAnimT_ + dt / LEGEND_ANIM_DURATION);
    } else {
      legendAnimT_ = std::max(0.0f, legendAnimT_ - dt / LEGEND_ANIM_DURATION);
    }

    // Cubic smoothstep easing: 3*t^2 - 2*t^3
    float easeT = legendAnimT_ * legendAnimT_ * (3.0f - 2.0f * legendAnimT_);
    float windowAlpha = std::clamp(easeT, 0.0f, 1.0f);

    float legendW =
        355.0f * config.uiScale; // Compact width with balanced column spacing
    float padX = 10.0f * config.uiScale;
    float padY = 8.0f * config.uiScale;
    float commonBtnW = legendW;
    float commonBtnH =
        std::max(34.0f * config.uiScale,
                 ImGui::GetFrameHeight() + 6.0f); // Taller button up-and-down

    float hudEase = getHudIntroEase();
    float btnX = windowWidth - legendW - Minimap::PADDING;
    float targetBtnY = Minimap::PADDING;
    float curBtnY =
        -commonBtnH - 6.0f + (targetBtnY + commonBtnH + 6.0f) * hudEase;

    // 1. Single unified persistent toggle button sliding in from top edge;
    // when settled, extend invisible window bounds to top/right screen edges so hovering/clicking
    // the outer 2px margin never leaks to camera drag or changes cursor.
    bool introDone = (hudEase >= 0.999f);
    float winPosY = introDone ? 0.0f : curBtnY;
    float winSizeW = introDone ? (commonBtnW + Minimap::PADDING) : commonBtnW;
    float winSizeH = introDone ? (targetBtnY + commonBtnH) : commonBtnH;

    ImGui::SetNextWindowPos(ImVec2(btnX, winPosY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(winSizeW, winSizeH), ImGuiCond_Always);

    ImGuiWindowFlags btnFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (ImGui::Begin("##MinimapLegendUnifiedToggle", nullptr, btnFlags)) {
      if (introDone) {
        ImGui::SetCursorPos(ImVec2(0.0f, targetBtnY));
      }
      const char *btnLabel =
          config.showLegend ? "[M] Сховати легенду" : "[M] Показати легенду";
      if (ImGui::Button(btnLabel, ImVec2(commonBtnW, commonBtnH))) {
        config.showLegend = !config.showLegend;
      }
      if (ImGui::IsItemHovered()) {
        showStyledTooltip(config.uiScale, "%s",
                          config.showLegend
                              ? "Сховати детальну легенду позначок мапи [M]."
                              : "Показати детальну легенду позначок мапи [M].");
      }
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    float btnY = targetBtnY;

    // 2. Details window gracefully sliding & fading vertically under the toggle button
    if (windowAlpha > 0.001f) {
      float frameH = ui::Minimap::getFrameHeight(config.minimapScale);
      float mapControlsBtnH =
          std::max(28.0f * config.uiScale, ImGui::GetFrameHeight());
      float mapControlsY =
          windowHeight - frameH - Minimap::PADDING - mapControlsBtnH - 6.0f;
      float targetLegendY = btnY + commonBtnH + 6.0f;
      float maxLegendH = std::max(120.0f, mapControlsY - 6.0f - targetLegendY);

      bool isQuotaMet = (engine.getAgent().getBackpack().getCurrentValue() >= config.companyQuota);
      static bool s_lastQuotaMet = false;
      if (s_lastQuotaMet != isQuotaMet) {
        s_lastQuotaMet = isQuotaMet;
        collapsedLegendH_ = 0.0f;
      }

      if (std::abs(lastUiScale_ - config.uiScale) > 0.001f) {
        lastUiScale_ = config.uiScale;
        collapsedLegendH_ = 0.0f;
      }

      float baseLegendH = ImGui::GetTextLineHeightWithSpacing() * 2.0f +
                          9.0f * (std::max(18.0f, ImGui::GetTextLineHeight()) +
                                  ImGui::GetStyle().ItemSpacing.y + 6.0f) +
                          ImGui::GetFrameHeightWithSpacing() + padY * 2.0f +
                          (isQuotaMet ? (ImGui::GetTextLineHeightWithSpacing() + 38.0f * config.uiScale) : 28.0f * config.uiScale);

      float collapsedH =
          (collapsedLegendH_ > 50.0f) ? collapsedLegendH_ : baseLegendH;
      collapsedH = std::min(collapsedH, maxLegendH);

      float subAnimSpeed = 5.2f; // Smooth ~0.19s transition
      if (isCatalogOpen_ && !isCatalogClosing_) {
        catalogOpenProgress_ =
            std::min(1.0f, catalogOpenProgress_ + dt * subAnimSpeed);
      } else {
        catalogOpenProgress_ =
            std::max(0.0f, catalogOpenProgress_ - dt * subAnimSpeed);
        if (catalogOpenProgress_ <= 0.0f) {
          isCatalogOpen_ = false;
          isCatalogClosing_ = false;
        }
      }

      if (isCollectedOpen_ && !isCollectedClosing_) {
        collectedOpenProgress_ =
            std::min(1.0f, collectedOpenProgress_ + dt * subAnimSpeed);
      } else {
        collectedOpenProgress_ =
            std::max(0.0f, collectedOpenProgress_ - dt * subAnimSpeed);
        if (collectedOpenProgress_ <= 0.0f) {
          isCollectedOpen_ = false;
          isCollectedClosing_ = false;
        }
      }

      float easeCatalog = catalogOpenProgress_ * catalogOpenProgress_ *
                          (3.0f - 2.0f * catalogOpenProgress_);
      float easeCollected = collectedOpenProgress_ * collectedOpenProgress_ *
                            (3.0f - 2.0f * collectedOpenProgress_);

      float targetCatalogH = maxLegendH;
      float targetCollectedH = std::min(maxLegendH, collapsedH + measuredCollectedContentH_);

      float catalogAddedH = (targetCatalogH - collapsedH) * easeCatalog;
      float collectedAddedH = (targetCollectedH - collapsedH) * easeCollected;
      float legendH = collapsedH + std::max(catalogAddedH, collectedAddedH);
      legendH = std::min(legendH, maxLegendH);

      float legendX = btnX;
      float legendY = targetLegendY - (1.0f - easeT) * (legendH + 10.0f);

      ImGui::SetNextWindowPos(ImVec2(legendX, legendY), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(legendW, legendH), ImGuiCond_Always);

      ImGuiWindowFlags flags =
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_NoScrollWithMouse;

      ImGui::PushStyleColor(ImGuiCol_WindowBg,
                            ImVec4(0.08f, 0.07f, 0.08f, 0.70f));
      ImGui::PushStyleColor(ImGuiCol_Border,
                            ImVec4(0.48f, 0.16f, 0.24f, 0.85f));
      ImGui::PushStyleColor(ImGuiCol_ScrollbarBg,
                            ImVec4(0.05f, 0.04f, 0.06f, 0.45f));
      ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab,
                            ImVec4(0.40f, 0.20f, 0.28f, 0.75f));
      ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered,
                            ImVec4(0.60f, 0.28f, 0.38f, 0.90f));
      ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive,
                            ImVec4(0.85f, 0.38f, 0.48f, 1.00f));

      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, windowAlpha);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padX, padY));
      ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 10.0f * config.uiScale);
      ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 4.0f);

      if (ImGui::Begin("MinimapLegendWindow", nullptr, flags)) {
        ImGui::GetWindowDrawList()->PushClipRect(
            ImVec2(btnX - 10.0f, targetLegendY - 2.0f),
            ImVec2(btnX + legendW + 10.0f, windowHeight), true);
        ImGui::SetScrollX(0.0f);
        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f),
                           "Легенда позначок:");
        ImGui::Separator();

        auto colA = [windowAlpha](uint8_t r, uint8_t g, uint8_t b,
                                  uint8_t baseA = 255) -> ImU32 {
          uint8_t a = static_cast<uint8_t>(
              std::clamp(baseA * windowAlpha, 0.0f, 255.0f));
          return IM_COL32(r, g, b, a);
        };

        auto drawLegendItem = [scale = config.uiScale](const char *label,
                                                     const char *tooltip,
                                                     auto iconFunc) {
          ImVec2 p = ImGui::GetCursorScreenPos();
          float iconSize = 18.0f;
          ImDrawList *drawList = ImGui::GetWindowDrawList();
          iconFunc(drawList, p, iconSize);

          ImGui::Dummy(ImVec2(iconSize, iconSize));
          ImGui::SameLine(0, 8.0f);
          ImGui::TextUnformatted(label);

          if (ImGui::IsItemHovered() ||
              ImGui::IsMouseHoveringRect(
                  p, ImVec2(p.x + iconSize, p.y + iconSize))) {
            showStyledTooltip(scale, "%s", tooltip);
          }
          ImGui::Spacing();
        };

        drawLegendItem(
            "Робітник (Агент)",
            "Автономний робітник у захисному костюмі Lethal Company",
            [&colA, &renderer](ImDrawList *dl, ImVec2 p, float s) {
              ImVec2 center(p.x + s * 0.5f, p.y + s * 0.5f);
              dl->AddCircleFilled(center, s * 0.5f, colA(30, 24, 18, 220));
              dl->AddCircle(center, s * 0.5f, colA(255, 190, 40, 240), 0, 1.4f);
              if (renderer.hasAgentTexture()) {
                const auto &tex = renderer.getAgentTexture();
                auto glHandle = tex.getNativeHandle();
                ImTextureID texID{};
                std::memcpy(&texID, &glHandle, sizeof(glHandle));
                float pad = 1.0f;
                dl->AddImage(texID, ImVec2(p.x + pad, p.y + pad),
                             ImVec2(p.x + s - pad, p.y + s - pad),
                             ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                             colA(255, 255, 255, 255));
              } else {
                dl->AddCircleFilled(center, 5.0f, colA(255, 200, 40, 255));
                dl->AddCircle(center, 5.0f, colA(255, 255, 255, 240), 0, 1.5f);
              }
            });

        drawLegendItem(
            "Шлюз (Старт)",
            "Початкова точка, звідки робітник починає місію (люк шлюзу з "
            "маяком)",
            [&colA](ImDrawList *dl, ImVec2 p, float s) {
              dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s),
                                colA(28, 26, 32, 255), 2.0f);
              dl->AddRectFilled(ImVec2(p.x + 2, p.y + 2),
                                ImVec2(p.x + s - 2, p.y + s - 2),
                                colA(240, 135, 18, 255), 2.0f);
              dl->AddRect(ImVec2(p.x + 2, p.y + 2),
                          ImVec2(p.x + s - 2, p.y + s - 2),
                          colA(255, 210, 90, 255), 2.0f, 0, 1.2f);
              dl->AddCircle(ImVec2(p.x + s * 0.5f, p.y + s * 0.5f), s * 0.22f,
                            colA(255, 240, 140, 255), 0, 1.2f);
              // 4 corner beacons
              dl->AddRectFilled(ImVec2(p.x + 2, p.y + 2),
                                ImVec2(p.x + 4.5f, p.y + 4.5f),
                                colA(255, 220, 60, 255));
              dl->AddRectFilled(ImVec2(p.x + s - 4.5f, p.y + 2),
                                ImVec2(p.x + s - 2, p.y + 4.5f),
                                colA(255, 220, 60, 255));
              dl->AddRectFilled(ImVec2(p.x + 2, p.y + s - 4.5f),
                                ImVec2(p.x + 4.5f, p.y + s - 2),
                                colA(255, 220, 60, 255));
              dl->AddRectFilled(ImVec2(p.x + s - 4.5f, p.y + s - 4.5f),
                                ImVec2(p.x + s - 2, p.y + s - 2),
                                colA(255, 220, 60, 255));
            });

        // 3. Exit (Evacuation) - actual maze cell appearance
        if (config.routeMode == core::RouteMode::RoundTrip) {
          drawLegendItem("Вихід (Недоступний)",
                         "У режимі 'A -> A (База)' вихід заблоковано. "
                         "Евакуація лише через вхід.",
                         [&colA](ImDrawList *dl, ImVec2 p, float s) {
                           dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s),
                                             colA(32, 34, 40, 255), 2.0f);
                           dl->AddRectFilled(ImVec2(p.x + 2, p.y + 2),
                                             ImVec2(p.x + s - 2, p.y + s - 2),
                                             colA(52, 56, 66, 255), 2.0f);
                           dl->AddRect(ImVec2(p.x + 2, p.y + 2),
                                       ImVec2(p.x + s - 2, p.y + s - 2),
                                       colA(80, 85, 96, 255), 2.0f, 0, 1.2f);
                           dl->AddLine(ImVec2(p.x + 3.5f, p.y + 3.5f),
                                       ImVec2(p.x + s - 3.5f, p.y + s - 3.5f),
                                       colA(225, 55, 55, 240), 2.0f);
                           dl->AddLine(ImVec2(p.x + s - 3.5f, p.y + 3.5f),
                                       ImVec2(p.x + 3.5f, p.y + s - 3.5f),
                                       colA(225, 55, 55, 240), 2.0f);
                         });
        } else {
          drawLegendItem(
              "Вихід (Евакуація)",
              "Шлюз евакуації, куди робітник прямує після збору брухту "
              "(смарагдова платформа)",
              [&colA](ImDrawList *dl, ImVec2 p, float s) {
                dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s),
                                  colA(20, 32, 26, 255), 2.0f);
                dl->AddRectFilled(ImVec2(p.x + 2, p.y + 2),
                                  ImVec2(p.x + s - 2, p.y + s - 2),
                                  colA(40, 185, 100, 255), 2.0f);
                dl->AddRect(ImVec2(p.x + 2, p.y + 2),
                            ImVec2(p.x + s - 2, p.y + s - 2),
                            colA(140, 255, 185, 255), 2.0f, 0, 1.2f);
                dl->AddCircle(ImVec2(p.x + s * 0.5f, p.y + s * 0.5f), s * 0.22f,
                              colA(180, 255, 210, 255), 0, 1.2f);
                // 4 corner emerald beacons
                dl->AddRectFilled(ImVec2(p.x + 2, p.y + 2),
                                  ImVec2(p.x + 4.5f, p.y + 4.5f),
                                  colA(80, 255, 160, 255));
                dl->AddRectFilled(ImVec2(p.x + s - 4.5f, p.y + 2),
                                  ImVec2(p.x + s - 2, p.y + 4.5f),
                                  colA(80, 255, 160, 255));
                dl->AddRectFilled(ImVec2(p.x + 2, p.y + s - 4.5f),
                                  ImVec2(p.x + 4.5f, p.y + s - 2),
                                  colA(80, 255, 160, 255));
                dl->AddRectFilled(ImVec2(p.x + s - 4.5f, p.y + s - 4.5f),
                                  ImVec2(p.x + s - 2, p.y + s - 2),
                                  colA(80, 255, 160, 255));
              });
        }

        // 4. Mud / Swamp / Water hazard tile (original style with round
        // bubbles)
        drawLegendItem(
            "Багнюка / Болото / Вода",
            "Важкопрохідна ділянка з бульбашками: витрата енергії +25 прямо, "
            "+35 по діагоналі",
            [&colA](ImDrawList *dl, ImVec2 p, float s) {
              dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s),
                                colA(32, 22, 14, 255), 2.0f);
              dl->AddRectFilled(ImVec2(p.x + 2, p.y + 2),
                                ImVec2(p.x + s - 2, p.y + s - 2),
                                colA(44, 32, 18, 255), 2.0f);
              dl->AddRect(ImVec2(p.x + 4.5f, p.y + 4.5f),
                          ImVec2(p.x + s - 4.5f, p.y + s - 4.5f),
                          colA(88, 68, 44, 220), 1.0f);
              // Mud bubbles
              dl->AddCircleFilled(ImVec2(p.x + s * 0.35f, p.y + s * 0.40f),
                                  2.2f, colA(115, 88, 54, 255));
              dl->AddCircle(ImVec2(p.x + s * 0.35f, p.y + s * 0.40f), 2.2f,
                            colA(152, 122, 80, 240), 0, 0.8f);
              dl->AddCircleFilled(ImVec2(p.x + s * 0.68f, p.y + s * 0.62f),
                                  1.6f, colA(115, 88, 54, 255));
              // Corner safety rivets
              dl->AddRectFilled(ImVec2(p.x + 1, p.y + 1),
                                ImVec2(p.x + 3, p.y + 3),
                                colA(120, 95, 55, 200));
              dl->AddRectFilled(ImVec2(p.x + s - 3, p.y + 1),
                                ImVec2(p.x + s - 1, p.y + 3),
                                colA(120, 95, 55, 200));
              dl->AddRectFilled(ImVec2(p.x + 1, p.y + s - 3),
                                ImVec2(p.x + 3, p.y + s - 1),
                                colA(120, 95, 55, 200));
              dl->AddRectFilled(ImVec2(p.x + s - 3, p.y + s - 3),
                                ImVec2(p.x + s - 1, p.y + s - 1),
                                colA(120, 95, 55, 200));
            });

        // 5. Light, medium, and heavy scrap items
        drawLegendItem(
            "Дрібний брухт (1-3 кг)",
            "Одноручний брухт Lethal Company (каченя, рибка, пульт, лупа)",
            [&colA](ImDrawList *dl, ImVec2 p, float s) {
              ImVec2 center(p.x + s * 0.5f, p.y + s * 0.5f);
              dl->AddCircleFilled(center, 5.0f, colA(50, 105, 255, 255));
              dl->AddCircle(center, 5.0f, colA(255, 255, 255, 240), 0, 1.8f);
            });

        drawLegendItem(
            "Побутовий брухт (4-8 кг)",
            "Середній побутовий брухт (годинник, бумбокс, фен, картина)",
            [&colA](ImDrawList *dl, ImVec2 p, float s) {
              ImVec2 center(p.x + s * 0.5f, p.y + s * 0.5f);
              dl->AddCircleFilled(center, 6.0f, colA(255, 255, 0, 255));
              dl->AddCircle(center, 6.0f, colA(255, 255, 255, 240), 0, 1.8f);
            });

        drawLegendItem(
            "Важкий брухт (10-18 кг)",
            "Дворучні масивні механізми (апарат живлення, двигун, золото, "
            "реактор)",
            [&colA](ImDrawList *dl, ImVec2 p, float s) {
              ImVec2 center(p.x + s * 0.5f, p.y + s * 0.5f);
              dl->AddCircleFilled(center, 7.0f, colA(255, 0, 255, 255));
              dl->AddCircle(center, 7.0f, colA(255, 255, 255, 240), 0, 1.8f);
            });

        // 6. Walked cell (Trail) - clean maze floor square without inner dots
        drawLegendItem("Пройдена клітинка (Слід)",
                       "Пройдений шлях робітника: темно-блакитна підлога з "
                       "неоновим контуром",
                       [&colA](ImDrawList *dl, ImVec2 p, float s) {
                         dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s),
                                           colA(10, 48, 65, 235), 2.0f);
                         dl->AddRect(p, ImVec2(p.x + s, p.y + s),
                                     colA(0, 230, 245, 240), 2.0f, 0, 1.5f);
                       });

        // 7. Route (TSP Trajectory) - laser line without waypoint node circle
        drawLegendItem("Маршрут (Траєкторія TSP)",
                       "Розрахована оптимальна лінія пересування робітника",
                       [&colA](ImDrawList *dl, ImVec2 p, float s) {
                         dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s),
                                           colA(22, 24, 30, 220), 2.0f);
                         // Outer neon glow
                         dl->AddLine(ImVec2(p.x + 2, p.y + s - 3),
                                     ImVec2(p.x + s - 2, p.y + 3),
                                     colA(0, 200, 240, 130), 4.2f);
                         // Main laser line
                         dl->AddLine(ImVec2(p.x + 2, p.y + s - 3),
                                     ImVec2(p.x + s - 2, p.y + 3),
                                     colA(0, 255, 240, 255), 2.0f);
                         // White core beam
                         dl->AddLine(ImVec2(p.x + 3, p.y + s - 4),
                                     ImVec2(p.x + s - 3, p.y + 4),
                                     colA(230, 255, 255, 240), 1.0f);
                       });

        ImGui::Spacing();
        float btnLocalY =
            ImGui::GetCursorPosY() - ImGui::GetStyle().WindowPadding.y;

        ImGui::PushStyleColor(ImGuiCol_Button,
                              (isCatalogOpen_ || isCatalogClosing_)
                                  ? ImVec4(0.28f, 0.16f, 0.22f, 0.90f)
                                  : ImVec4(0.16f, 0.14f, 0.18f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.42f, 0.22f, 0.30f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.55f, 0.28f, 0.38f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Border,
                              (isCatalogOpen_ || isCatalogClosing_)
                                  ? ImVec4(0.95f, 0.65f, 0.25f, 0.90f)
                                  : ImVec4(0.48f, 0.30f, 0.38f, 0.70f));

        char catalogBtnText[64];
        std::snprintf(catalogBtnText, sizeof(catalogBtnText),
                      "Каталог брухту (%zu найменувань)###CatalogBtn",
                      core::getLethalScrapTemplates().size());

        if (ImGui::Button(catalogBtnText,
                          ImVec2(-1.0f, std::max(28.0f * config.uiScale,
                                                 ImGui::GetFrameHeight())))) {
          if (!isCatalogOpen_ || isCatalogClosing_) {
            isCatalogOpen_ = true;
            isCatalogClosing_ = false;
            if (isCollectedOpen_) {
              isCollectedClosing_ = true;
              collectedScrollOnCloseStart_ = ImGui::GetScrollY();
            }
            catalogAutoScrollActive_ = true;
            catalogAutoScrollStart_ = ImGui::GetScrollY();
            catalogAutoScrollTarget_ = std::max(0.0f, btnLocalY - 4.0f);
            catalogAutoScrollTime_ = 0.0f;
          } else {
            isCatalogClosing_ = true;
            catalogScrollOnCloseStart_ = ImGui::GetScrollY();
          }
        }
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(
              config.uiScale, "%s",
              (isCatalogOpen_ && !isCatalogClosing_)
                  ? "Згорнути каталог найменувань брухту та повернутися "
                    "до легенди."
                  : "Розгорнути повний каталог з 60 найменувань брухту "
                    "Lethal Company.");
        }
        ImGui::PopStyleColor(4);

        if (isCatalogOpen_ || catalogOpenProgress_ > 0.001f) {
          ImGui::Spacing();
          const auto &catalog = core::getLethalScrapTemplates();
          std::unordered_set<std::string> mazeItemNames;
          for (const auto &mItem : maze.getItems()) {
            mazeItemNames.insert(mItem.name);
          }

          ImGui::PushStyleVar(
              ImGuiStyleVar_CellPadding,
              ImVec2(10.0f * config.uiScale, 4.0f * config.uiScale));

          if (ImGui::BeginTable("ScrapCatalogHeaderTable", 2,
                                ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("  Найменування",
                                    ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Вага / Ціна",
                                    ImGuiTableColumnFlags_WidthFixed,
                                    92.0f * config.uiScale);

            ImGui::PushStyleColor(ImGuiCol_TableHeaderBg,
                                  ImVec4(0.18f, 0.12f, 0.16f, 0.95f));
            ImGui::TableHeadersRow();
            ImGui::PopStyleColor();
            ImGui::EndTable();
          }

          core::ItemCategory lastCategory =
              static_cast<core::ItemCategory>(-1);
          bool catTableOpen = false;

          for (size_t i = 0; i < catalog.size(); ++i) {
            const auto &it = catalog[i];

            // Group category divider drawn outside any table so right column cannot clip or cover it
            if (it.category != lastCategory) {
              if (catTableOpen) {
                ImGui::EndTable();
                catTableOpen = false;
              }
              lastCategory = it.category;

              ImGui::Dummy(
                  ImVec2(0.0f, (i > 0 ? 10.0f : 4.0f) * config.uiScale));

              const char *catHeader =
                  (it.category == core::ItemCategory::LightScrap)
                      ? "Легкий брухт (1-3 кг)"
                  : (it.category == core::ItemCategory::MediumScrap)
                      ? "Побутовий брухт (4-8 кг)"
                      : "Важкий брухт (10-18 кг)";
              ImVec4 headerCol =
                  (it.category == core::ItemCategory::LightScrap)
                      ? ImVec4(0.20f, 0.41f, 1.00f, 1.0f)
                  : (it.category == core::ItemCategory::MediumScrap)
                      ? ImVec4(1.00f, 1.00f, 0.00f, 1.0f)
                      : ImVec4(1.0f, 0.0f, 1.0f, 1.0f);

              float topPad = 8.0f * config.uiScale;
              float botPad = 8.0f * config.uiScale;
              float bannerH = topPad + ImGui::GetTextLineHeight() + botPad;
              float availW = std::max(40.0f, ImGui::GetContentRegionAvail().x);

              ImVec2 bannerMin = ImGui::GetCursorScreenPos();
              ImVec2 bannerMax(bannerMin.x + availW, bannerMin.y + bannerH);

              char bannerId[32];
              std::snprintf(bannerId, sizeof(bannerId), "##CatBanner_%d",
                            static_cast<int>(it.category));
              ImGui::InvisibleButton(bannerId, ImVec2(availW, bannerH));
              bool isCatHovered = ImGui::IsItemHovered();
              if (isCatHovered) {
                hoveredCatalogCategory_ = static_cast<int>(it.category);
              }

              uint8_t bannerAlpha = static_cast<uint8_t>(
                  std::clamp((isCatHovered ? 68.0f : 28.0f) * windowAlpha,
                             0.0f, 255.0f));
              ImU32 rowBg = (it.category == core::ItemCategory::LightScrap)
                                ? IM_COL32(50, 105, 255, bannerAlpha)
                            : (it.category == core::ItemCategory::MediumScrap)
                                ? IM_COL32(255, 255, 0, bannerAlpha)
                                : IM_COL32(255, 0, 255, bannerAlpha);

              ImDrawList *dl = ImGui::GetWindowDrawList();
              dl->AddRectFilled(bannerMin, bannerMax, rowBg,
                                4.0f * config.uiScale);
              if (isCatHovered) {
                uint8_t bAlpha = static_cast<uint8_t>(
                    std::clamp(170.0f * windowAlpha, 0.0f, 255.0f));
                ImU32 bCol = (it.category == core::ItemCategory::LightScrap)
                                 ? IM_COL32(50, 105, 255, bAlpha)
                             : (it.category == core::ItemCategory::MediumScrap)
                                 ? IM_COL32(255, 255, 0, bAlpha)
                                 : IM_COL32(255, 0, 255, bAlpha);
                dl->AddRect(bannerMin, bannerMax, bCol, 4.0f * config.uiScale,
                            0, 1.0f);
              }

              float textW = ImGui::CalcTextSize(catHeader).x;
              float textX =
                  bannerMin.x + std::max(0.0f, (availW - textW) * 0.5f);
              float textY = bannerMin.y + topPad;
              ImVec4 txtCol = headerCol;
              txtCol.w *= windowAlpha;
              dl->AddText(ImVec2(textX, textY),
                          ImGui::ColorConvertFloat4ToU32(txtCol), catHeader);

              if (isCatHovered) {
                showStyledTooltip(
                    config.uiScale,
                    "%s\nВиділено всі предмети цієї категорії в комплексі.",
                    catHeader);
                if (lastHoveredCatalogCategory_ != hoveredCatalogCategory_) {
                  lastHoveredCatalogCategory_ = hoveredCatalogCategory_;
                  fitMazeToVisibleArea(camera, maze, config, engine,
                                       windowWidth, windowHeight, 0.02f);
                }
              }

              char catTableId[32];
              std::snprintf(catTableId, sizeof(catTableId), "ScrapCatTable_%d",
                            static_cast<int>(it.category));
              catTableOpen = ImGui::BeginTable(
                  catTableId, 2,
                  ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp);
              if (catTableOpen) {
                ImGui::TableSetupColumn("  Найменування",
                                        ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Вага / Ціна",
                                        ImGuiTableColumnFlags_WidthFixed,
                                        92.0f * config.uiScale);
              }
            }

            if (!catTableOpen)
              continue;

            ImVec4 col = (it.category == core::ItemCategory::LightScrap)
                             ? ImVec4(0.20f, 0.41f, 1.00f, 1.0f)
                         : (it.category == core::ItemCategory::MediumScrap)
                             ? ImVec4(1.00f, 1.00f, 0.00f, 1.0f)
                             : ImVec4(1.0f, 0.0f, 1.0f, 1.0f);

            bool isPresentInMaze = (mazeItemNames.count(it.name) > 0);
            bool isRowHovered = false;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(col, "%zu. %s", i + 1, it.name.c_str());

            if (isPresentInMaze) {
              ImVec2 pMin = ImGui::GetItemRectMin();
              ImVec2 pMax = ImGui::GetItemRectMax();
              if (ImGui::IsRectVisible(pMin, pMax)) {
                ImDrawList *dl = ImGui::GetWindowDrawList();
                ImVec4 lineCol = col;
                lineCol.w = std::clamp(windowAlpha, 0.0f, 1.0f);
                dl->AddLine(ImVec2(pMin.x, pMax.y - 1.0f),
                            ImVec2(pMax.x, pMax.y - 1.0f),
                            ImGui::ColorConvertFloat4ToU32(lineCol), 1.4f);
              }
            }

            if (ImGui::IsItemHovered()) {
              isRowHovered = true;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("%d кг, %d грн", it.baseWeight, it.baseValue);
            if (ImGui::IsItemHovered()) {
              isRowHovered = true;
            }

            if (isRowHovered) {
              hoveredCatalogCategory_ = -1;
              hoveredCatalogItemName_ = it.name;
              if (isPresentInMaze) {
                showStyledTooltip(
                    config.uiScale,
                    "%zu. %s [В поточному комплексі]\nВага: %d "
                    "кг | Вартість: %d грн",
                    i + 1, it.name.c_str(), it.baseWeight, it.baseValue);
              } else {
                showStyledTooltip(config.uiScale,
                                  "%zu. %s\nВага: %d кг | Вартість: %d грн",
                                  i + 1, it.name.c_str(), it.baseWeight,
                                  it.baseValue);
              }
              if (lastHoveredCatalogItemName_ != it.name) {
                lastHoveredCatalogItemName_ = it.name;
                fitMazeToVisibleArea(camera, maze, config, engine, windowWidth,
                                     windowHeight, 0.02f);
              }
            }
          }
          if (catTableOpen) {
            ImGui::EndTable();
          }
          ImGui::PopStyleVar();

          ImGui::Spacing();
          if (ImGui::Button("Вгору до легенди###ScrollUpLegendBtn",
                            ImVec2(-1.0f, 26.0f * config.uiScale))) {
            isCatalogClosing_ = true;
            catalogScrollOnCloseStart_ = ImGui::GetScrollY();
          }
          if (ImGui::IsItemHovered()) {
            showStyledTooltip(config.uiScale,
                              "Повернутися до початкового списку легенди.");
          }
        }

        // 2. Collected scrap list button below catalog
        ImGui::Spacing();
        const auto &collectedIds = engine.getCollectedItemIds();
        size_t collectedCount = collectedIds.size();
        const auto &curPlan = engine.getPlan();
        size_t targetItemsCount = 0;
        if (curPlan.routeValid && curPlan.visitOrder.size() > 2) {
          targetItemsCount = curPlan.visitOrder.size() - 2;
        } else {
          targetItemsCount = maze.getItems().size();
        }

        bool hasExtraScrap = (collectedCount > targetItemsCount);
        size_t extraCount =
            hasExtraScrap ? (collectedCount - targetItemsCount) : 0;

        int curCollectedVal = engine.getAgent().getBackpack().getCurrentValue();
        int quota = config.companyQuota;
        bool isQuotaMet = (curCollectedVal >= quota);

        char collectedBtnText[128];
        if (isQuotaMet) {
          if (hasExtraScrap) {
            std::snprintf(collectedBtnText, sizeof(collectedBtnText),
                          "(%d/%d грн | %zu од. [+%zu])###CollectedBtn",
                          curCollectedVal, quota, collectedCount, extraCount);
          } else {
            std::snprintf(collectedBtnText, sizeof(collectedBtnText),
                          "(%d/%d грн | %zu/%zu од.)###CollectedBtn",
                          curCollectedVal, quota, collectedCount,
                          targetItemsCount);
          }
        } else {
          if (hasExtraScrap) {
            std::snprintf(collectedBtnText, sizeof(collectedBtnText),
                          "Квота: %d / %d грн (%zu од. [+%zu])###CollectedBtn",
                          curCollectedVal, quota, collectedCount, extraCount);
          } else {
            std::snprintf(collectedBtnText, sizeof(collectedBtnText),
                          "Квота: %d / %d грн (%zu/%zu од.)###CollectedBtn",
                          curCollectedVal, quota, collectedCount,
                          targetItemsCount);
          }
        }

        ImVec4 btnNormalCol =
            isQuotaMet ? ImVec4(0.10f, 0.22f, 0.14f, 0.88f)
                       : (hasExtraScrap ? ImVec4(0.24f, 0.20f, 0.10f, 0.90f)
                                        : ImVec4(0.14f, 0.18f, 0.16f, 0.85f));
        ImVec4 btnOpenCol =
            isQuotaMet ? ImVec4(0.14f, 0.30f, 0.18f, 0.95f)
                       : (hasExtraScrap ? ImVec4(0.32f, 0.26f, 0.12f, 0.95f)
                                        : ImVec4(0.18f, 0.28f, 0.22f, 0.90f));
        ImVec4 borderCol =
            isQuotaMet
                ? ImVec4(0.25f, 0.95f, 0.50f, 0.95f)
                : (hasExtraScrap ? ImVec4(0.95f, 0.75f, 0.20f, 0.95f)
                                 : ((isCollectedOpen_ || isCollectedClosing_)
                                        ? ImVec4(0.35f, 0.95f, 0.55f, 0.90f)
                                        : ImVec4(0.25f, 0.50f, 0.35f, 0.70f)));

        ImGui::PushStyleColor(ImGuiCol_Button,
                              (isCollectedOpen_ || isCollectedClosing_)
                                  ? btnOpenCol
                                  : btnNormalCol);
        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            isQuotaMet ? ImVec4(0.16f, 0.35f, 0.22f, 0.95f)
                       : (hasExtraScrap ? ImVec4(0.40f, 0.32f, 0.16f, 0.95f)
                                        : ImVec4(0.24f, 0.38f, 0.28f, 0.95f)));
        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive,
            isQuotaMet ? ImVec4(0.22f, 0.45f, 0.28f, 1.00f)
                       : (hasExtraScrap ? ImVec4(0.50f, 0.40f, 0.20f, 1.00f)
                                        : ImVec4(0.30f, 0.48f, 0.35f, 1.00f)));
        ImGui::PushStyleColor(ImGuiCol_Border, borderCol);

        if (ImGui::Button(collectedBtnText,
                          ImVec2(-1.0f, std::max(28.0f * config.uiScale,
                                                 ImGui::GetFrameHeight())))) {
          if (!isCollectedOpen_ || isCollectedClosing_) {
            isCollectedOpen_ = true;
            isCollectedClosing_ = false;
            if (isCatalogOpen_) {
              isCatalogClosing_ = true;
              catalogScrollOnCloseStart_ = ImGui::GetScrollY();
            }
          } else {
            isCollectedClosing_ = true;
            collectedScrollOnCloseStart_ = ImGui::GetScrollY();
          }
        }
        if (ImGui::IsItemHovered()) {
          if (isQuotaMet) {
            showStyledTooltip(config.uiScale,
                              "Мінімальну квоту Компанії успішно виконано!\n"
                              "Зібрано %d грн із необхідних %d грн.",
                              curCollectedVal, quota);
          } else if (hasExtraScrap) {
            showStyledTooltip(
                config.uiScale,
                "На шляху агента лежав брухт, який помістився в рюкзак,\n"
                "тому агент зібрав на %zu од. більше, ніж планувалося!",
                extraCount);
          } else {
            showStyledTooltip(
                config.uiScale, "%s",
                (isCollectedOpen_ && !isCollectedClosing_)
                    ? "Згорнути список зібраного робітником брухту."
                    : "Розгорнути повний список зібраного на мапі брухту.");
          }
        }
        ImGui::PopStyleColor(4);

        if (isQuotaMet) {
          ImGui::Spacing();
          const char *quotaSuccessText = "Мінімальну квоту виконано!";
          float textW = ImGui::CalcTextSize(quotaSuccessText).x;
          float availW = ImGui::GetContentRegionAvail().x;
          if (availW > textW) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availW - textW) * 0.5f);
          }
          ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.55f, 1.0f), "%s", quotaSuccessText);
          ImGui::Dummy(ImVec2(0.0f, 6.0f * config.uiScale));
        }

        if (isCollectedOpen_ || collectedOpenProgress_ > 0.001f) {
          ImGui::Spacing();
          float collectedEase = collectedOpenProgress_ *
                                collectedOpenProgress_ *
                                (3.0f - 2.0f * collectedOpenProgress_);
          float clipBoxH =
              std::max(10.0f, measuredCollectedContentH_ * collectedEase);
          ImGui::BeginChild(
              "##CollectedScrapAnimBox", ImVec2(0.0f, clipBoxH), false,
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
          float childStartY = ImGui::GetCursorPosY();

          // Quota status notification banner at the top of the expanded card
          if (isQuotaMet) {
            int surplusVal = curCollectedVal - quota;
            ImGui::PushStyleColor(ImGuiCol_Border,
                                  ImVec4(0.25f, 0.95f, 0.50f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                                  ImVec4(0.10f, 0.25f, 0.16f, 0.92f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleVar(
                ImGuiStyleVar_WindowPadding,
                ImVec2(8.0f * config.uiScale, 6.0f * config.uiScale));
            ImGui::BeginChild("##QuotaSuccessNotice", ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_Border |
                                  ImGuiChildFlags_AutoResizeY);
            ImGui::TextColored(ImVec4(0.40f, 1.0f, 0.60f, 1.0f),
                               "[OK] МІНІМАЛЬНУ КВОТУ ВИКОНАНО!");
            ImGui::TextWrapped(
                "Зібрано %d грн із необхідних %d грн (+%d грн перевиконання). "
                "Евакуація гарантує 100%% успіх!",
                curCollectedVal, quota, surplusVal);
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::Spacing();
          } else {
            int remainingVal = quota - curCollectedVal;
            ImGui::PushStyleColor(ImGuiCol_Border,
                                  ImVec4(1.0f, 0.75f, 0.20f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                                  ImVec4(0.24f, 0.18f, 0.08f, 0.90f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleVar(
                ImGuiStyleVar_WindowPadding,
                ImVec2(8.0f * config.uiScale, 6.0f * config.uiScale));
            ImGui::BeginChild("##QuotaProgressNotice", ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_Border |
                                  ImGuiChildFlags_AutoResizeY);
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f),
                               "[*] ПРОГРЕС КВОТИ: %d / %d грн",
                               curCollectedVal, quota);
            ImGui::TextWrapped("До виконання вимоги Компанії залишилося "
                               "підібрати брухту на %d грн.",
                               remainingVal);
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::Spacing();
          }

          if (hasExtraScrap) {
            ImGui::PushStyleColor(ImGuiCol_Border,
                                  ImVec4(0.95f, 0.75f, 0.20f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                                  ImVec4(0.18f, 0.14f, 0.08f, 0.85f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleVar(
                ImGuiStyleVar_WindowPadding,
                ImVec2(8.0f * config.uiScale, 6.0f * config.uiScale));
            ImGui::BeginChild("##OpportunisticScrapNotice", ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_Border |
                                  ImGuiChildFlags_AutoResizeY);
            ImGui::TextColored(
                ImVec4(1.0f, 0.85f, 0.30f, 1.0f),
                "[!] Понадплановий збір (+%zu од.):", extraCount);
            ImGui::TextWrapped(
                "На шляху агента лежав брухт, який помістився в рюкзак, "
                "тому агент зібрав більше ніж потрібно було!");
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::Spacing();
          }

          if (collectedIds.empty()) {
            ImGui::TextDisabled("Робітник ще не зібрав жодного брухту.");
          } else {
            if (ImGui::BeginTable("CollectedScrapTable", 2,
                                  ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_SizingStretchProp)) {
              ImGui::TableSetupColumn("Зібраний предмет",
                                      ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableSetupColumn("Вага / Ціна",
                                      ImGuiTableColumnFlags_WidthFixed,
                                      92.0f * config.uiScale);

              ImGui::PushStyleColor(ImGuiCol_TableHeaderBg,
                                    ImVec4(0.12f, 0.20f, 0.16f, 0.95f));
              ImGui::TableHeadersRow();
              ImGui::PopStyleColor();

              for (size_t i = 0; i < collectedIds.size(); ++i) {
                int itemId = collectedIds[i];
                auto optItem = maze.getItemById(itemId);
                if (!optItem.has_value())
                  continue;
                const auto &it = *optItem;
                bool isPlanned = engine.isItemPlanned(it.id);

                ImVec4 col = (it.category == core::ItemCategory::LightScrap)
                                 ? ImVec4(0.20f, 0.41f, 1.00f, 1.0f)
                             : (it.category == core::ItemCategory::MediumScrap)
                                 ? ImVec4(1.00f, 1.00f, 0.00f, 1.0f)
                                 : ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(col, "%zu. %s", i + 1, it.name.c_str());
                if (!isPlanned) {
                  ImGui::SameLine();
                  ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.25f, 1.0f),
                                     "[+Попутний]");
                }

                bool isRowHovered = ImGui::IsItemHovered();
                ImGui::TableSetColumnIndex(1);
                ImGui::TextDisabled("%d кг, %d грн", it.weight, it.value);
                if (ImGui::IsItemHovered()) {
                  isRowHovered = true;
                }

                if (isRowHovered) {
                  hoveredCollectedItemId_ = it.id;
                  float itemWorldX =
                      (it.position.x + 0.5f) * ProceduralRenderer::TILE_SIZE;
                  float itemWorldY =
                      (it.position.y + 0.5f) * ProceduralRenderer::TILE_SIZE;
                  camera.panTo(itemWorldX, itemWorldY, 8.5f);
                  camera.zoomTo(0.50f, 8.5f); // 2.0x magnification

                  renderTopCenteredItemTooltip(it, window, camera.getView(),
                                               windowWidth, windowHeight,
                                               !isPlanned);
                }
              }
              ImGui::EndTable();
            }
          }

          ImGui::Spacing();
          if (ImGui::Button("Вгору до легенди###ScrollUpCollectedBtn",
                            ImVec2(-1.0f, 26.0f * config.uiScale))) {
            isCollectedClosing_ = true;
            collectedScrollOnCloseStart_ = ImGui::GetScrollY();
          }
          if (ImGui::IsItemHovered()) {
            showStyledTooltip(config.uiScale,
                              "Повернутися до початкового списку легенди.");
          }

          measuredCollectedContentH_ =
              std::max(40.0f, ImGui::GetCursorPosY() - childStartY + 12.0f);
          ImGui::EndChild();
        }

        if (hoveredCatalogItemName_.empty()) {
          lastHoveredCatalogItemName_.clear();
        }
        if (hoveredCatalogCategory_ < 0) {
          lastHoveredCatalogCategory_ = -1;
        }

        // Measure collapsed height only when both catalog and collected lists
        // are completely closed
        if (!isCatalogOpen_ && catalogOpenProgress_ <= 0.0f &&
            !isCollectedOpen_ && collectedOpenProgress_ <= 0.0f) {
          float measuredH = std::ceil(ImGui::GetCursorPosY() + padY + 10.0f * config.uiScale);
          if (measuredH > 50.0f) {
            collapsedLegendH_ = measuredH;
          }
        }

        // Smooth scroll processing & lock horizontal shift
        if (isCatalogClosing_) {
          float easeSub = catalogOpenProgress_ * catalogOpenProgress_ *
                          (3.0f - 2.0f * catalogOpenProgress_);
          ImGui::SetScrollY(catalogScrollOnCloseStart_ * easeSub);
        } else if (isCollectedClosing_) {
          float easeSub = collectedOpenProgress_ * collectedOpenProgress_ *
                          (3.0f - 2.0f * collectedOpenProgress_);
          ImGui::SetScrollY(collectedScrollOnCloseStart_ * easeSub);
        } else if (catalogAutoScrollActive_) {
          if (std::abs(ImGui::GetIO().MouseWheel) > 0.001f &&
              ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
            catalogAutoScrollActive_ = false;
            legendScroller_.targetScrollY = ImGui::GetScrollY();
            legendScroller_.smoothScrollY = ImGui::GetScrollY();
            legendScroller_.update(68.0f, config.uiScale);
          } else {
            float dtScroll = ImGui::GetIO().DeltaTime;
            catalogAutoScrollTime_ += dtScroll;
            float t = std::clamp(catalogAutoScrollTime_ / 0.25f, 0.0f, 1.0f);
            float ease = 1.0f - std::pow(1.0f - t, 3.0f);
            float curY =
                catalogAutoScrollStart_ +
                (catalogAutoScrollTarget_ - catalogAutoScrollStart_) * ease;

            ImGui::SetScrollY(curY);
            legendScroller_.smoothScrollY = curY;
            legendScroller_.targetScrollY = catalogAutoScrollTarget_;

            if (t >= 1.0f) {
              catalogAutoScrollActive_ = false;
            }
          }
          ImGui::SetScrollX(0.0f);
        } else if (collectedAutoScrollActive_) {
          if (std::abs(ImGui::GetIO().MouseWheel) > 0.001f &&
              ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
            collectedAutoScrollActive_ = false;
            legendScroller_.targetScrollY = ImGui::GetScrollY();
            legendScroller_.smoothScrollY = ImGui::GetScrollY();
            legendScroller_.update(68.0f, config.uiScale);
          } else {
            float dtScroll = ImGui::GetIO().DeltaTime;
            collectedAutoScrollTime_ += dtScroll;
            float t = std::clamp(collectedAutoScrollTime_ / 0.25f, 0.0f, 1.0f);
            float ease = 1.0f - std::pow(1.0f - t, 3.0f);
            float curY =
                collectedAutoScrollStart_ +
                (collectedAutoScrollTarget_ - collectedAutoScrollStart_) * ease;

            ImGui::SetScrollY(curY);
            legendScroller_.smoothScrollY = curY;
            legendScroller_.targetScrollY = collectedAutoScrollTarget_;

            if (t >= 1.0f) {
              collectedAutoScrollActive_ = false;
            }
          }
          ImGui::SetScrollX(0.0f);
        } else {
          legendScroller_.update(68.0f, config.uiScale);
          ImGui::SetScrollX(0.0f);
        }
        ImGui::GetWindowDrawList()->PopClipRect();
      }
      ImGui::End();

      ImGui::PopStyleVar(5);
      ImGui::PopStyleColor(6);
    }
  }

  void renderMapTopControls(UIState &state, core::Config &config,
                            float windowWidth, float windowHeight) {
    float attachEase = getHudAttachEase();
    if (attachEase <= 0.001f) {
      return;
    }

    float frameW = ui::Minimap::getFrameWidth(config.minimapScale);
    float frameH = ui::Minimap::getFrameHeight(config.minimapScale);
    float btnW = std::min(frameW, 280.0f);
    float btnH = std::max(28.0f * config.uiScale, ImGui::GetFrameHeight());
    float posX =
        windowWidth - frameW - Minimap::PADDING + (frameW - btnW) * 0.5f;
    float frameTopY = windowHeight - frameH - Minimap::PADDING;
    float monitorVisualTopY =
        frameTopY + Minimap::getVisualLeftInset(config.minimapScale);
    float targetPosY = frameTopY - btnH - 6.0f;
    float startHiddenY = monitorVisualTopY + 2.0f;
    float curPosY = startHiddenY + (targetPosY - startHiddenY) * attachEase;

    ImGui::SetNextWindowPos(ImVec2(posX, curPosY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(btnW, btnH), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##MapTopControlsWindow", nullptr, flags)) {
      bool clipToMonitorTop = (attachEase < 0.999f);
      if (clipToMonitorTop) {
        ImGui::PushClipRect(ImVec2(0.0f, 0.0f),
                            ImVec2(windowWidth, monitorVisualTopY), true);
      }

      bool followActive = config.cameraFollowAgent;
      if (followActive) {
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.48f, 0.16f, 0.24f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_Border,
                              ImVec4(0.95f, 0.50f, 0.20f, 1.00f));
      }

      if (state.isTransitionActive) {
        ImGui::BeginDisabled(true);
      }

      if (ImGui::Button("[C] Центрувати на агенті", ImVec2(btnW, btnH))) {
        config.cameraFollowAgent = !config.cameraFollowAgent;
        if (config.cameraFollowAgent) {
          state.requestCenterCameraOnAgent = true;
        } else {
          state.requestCenterCameraOnMaze = true;
        }
      }

      if (state.isTransitionActive) {
        ImGui::EndDisabled();
      }

      if (followActive) {
        ImGui::PopStyleColor(2);
      }

      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        if (state.isTransitionActive) {
          showStyledTooltip(config.uiScale,
                            "Центрування недоступне під час генерації та "
                            "трасування маршруту.");
        } else {
          showStyledTooltip(
              config.uiScale, "%s",
              config.cameraFollowAgent
                  ? "Вимкнути стеження камери за агентом [C]."
                  : "Центрувати та увімкнути стеження камери за агентом [C].");
        }
      }

      if (clipToMonitorTop) {
        ImGui::PopClipRect();
      }
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
  }

  struct AgentStatusDisplay {
    std::string text;
    ImVec4 color;
  };

  static AgentStatusDisplay
  getAgentStatusDisplay(const simulation::SimulationEngine &engine,
                        const map::Maze &maze,
                        const ProceduralRenderer &renderer,
                        bool isManualMode) {
    const auto &agent = engine.getAgent();
    const auto &pos = agent.getPosition();

    // 1. Evacuation reached
    bool isExitDisabled =
        (engine.getConfig().routeMode == core::RouteMode::RoundTrip);
    const auto &evacTarget =
        isExitDisabled ? maze.getStartPoint() : maze.getExitPoint();
    if ((agent.getStatus() == simulation::AgentStatus::Success) ||
        (pos == evacTarget && agent.getStepsTaken() > 0)) {
      return {"Евакуація...", ImVec4(0.20f, 1.0f, 0.35f, 1.0f)};
    }

    // 2. Start point before departure / parachuting
    if (renderer.isWaitingForDrop() || renderer.isParachuting() ||
        (pos == maze.getStartPoint() && agent.getStepsTaken() == 0)) {
      return {"Десантування...", ImVec4(1.0f, 0.60f, 0.15f, 1.0f)};
    }

    // 3. Exhausted
    if (engine.getConfig().energyLimitEnabled &&
        (agent.isExhausted() ||
         agent.getStatus() == simulation::AgentStatus::Exhausted)) {
      return {"Енергія вичерпана!", ImVec4(0.92f, 0.05f, 0.05f, 1.0f)};
    }

    // 4. Scrap collection on current cell
    if (maze.isValid(pos) && maze.at(pos).hasResource()) {
      int rId = maze.at(pos).getResourceId();
      const auto &collected = engine.getCollectedItemIds();
      auto optItem = maze.getItemById(rId);
      if (std::find(collected.begin(), collected.end(), rId) !=
          collected.end()) {
        if (optItem.has_value()) {
          ImVec4 catCol = (optItem->category == core::ItemCategory::LightScrap)
                              ? ImVec4(0.20f, 0.41f, 1.00f, 1.0f)
                          : (optItem->category == core::ItemCategory::MediumScrap)
                              ? ImVec4(1.00f, 1.00f, 0.00f, 1.0f)
                              : ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
          return {"Збір брухту: " + optItem->name, catCol};
        }
      } else if (optItem.has_value() &&
                 engine.getConfig().backpackLimitEnabled) {
        int freeKg = std::max(0, agent.getBackpack().getMaxCapacity() -
                                     agent.getBackpack().getCurrentWeight());
        if (optItem->weight > freeKg) {
          std::string msg =
              (freeKg == 0)
                  ? ("Рюкзак повний (" + std::to_string(optItem->weight) +
                     " кг)")
                  : ("Замало місця (" + std::to_string(optItem->weight) +
                     " > " + std::to_string(freeKg) + " кг)");
          return {msg, ImVec4(1.0f, 0.36f, 0.24f, 1.0f)};
        }
      }
    }

    // 5. Manual control vs Autonomous
    if (isManualMode) {
      return {"Ручне збирання брухту", ImVec4(1.0f, 0.82f, 0.25f, 1.0f)};
    } else {
      return {"Автономне збирання брухту", ImVec4(0.68f, 0.94f, 1.00f, 1.0f)};
    }
  }

  void renderMissionStatusPanel(UIState &state, core::Config &config,
                                const map::Maze &maze,
                                const simulation::SimulationEngine &engine,
                                const ProceduralRenderer &renderer,
                                float windowWidth, float windowHeight) {
    float dt = ImGui::GetIO().DeltaTime;
    const float PANEL_ANIM_DUR = 0.26f;
    if (missionPanelCollapsed_) {
      missionPanelOpenProgress_ =
          std::max(0.0f, missionPanelOpenProgress_ - dt / PANEL_ANIM_DUR);
    } else {
      missionPanelOpenProgress_ =
          std::min(1.0f, missionPanelOpenProgress_ + dt / PANEL_ANIM_DUR);
    }
    float easeT = missionPanelOpenProgress_ * missionPanelOpenProgress_ *
                  (3.0f - 2.0f * missionPanelOpenProgress_);

    bool showManualCard =
        engine.isManualMode() && !engine.isAtEvacuationCell() &&
        engine.getAgent().getStatus() != simulation::AgentStatus::Success;
    const float CARD_ANIM_DUR = 0.20f;
    if (showManualCard) {
      manualBannerOpenProgress_ =
          std::min(1.0f, manualBannerOpenProgress_ + dt / CARD_ANIM_DUR);
    } else {
      manualBannerOpenProgress_ =
          std::max(0.0f, manualBannerOpenProgress_ - dt / CARD_ANIM_DUR);
    }
    float cardEaseT = manualBannerOpenProgress_ * manualBannerOpenProgress_ *
                      (3.0f - 2.0f * manualBannerOpenProgress_);

    float mmFrameW = Minimap::getFrameWidth(config.minimapScale);
    float monitorLeftX = std::round(
        windowWidth - mmFrameW - Minimap::PADDING +
        Minimap::getVisualLeftInset(config.minimapScale));

    float panelW = std::round(345.0f * config.uiScale);
    float panelH = std::round(154.0f * config.uiScale);

    float cardPadX = 10.0f * config.uiScale;
    float cardPadY = 8.0f * config.uiScale;
    float cardBtnH =
        std::max(28.0f * config.uiScale, ImGui::GetFrameHeight() + 4.0f);
    const char *returnBtnLabel = "Повернутись до останнього автоматичного кроку";
    float btnTextW = ImGui::CalcTextSize(returnBtnLabel).x;
    float cardW = std::max(
        panelW, std::round(btnTextW + cardPadX * 2.0f + 22.0f * config.uiScale));
    float cardTextLineH = ImGui::GetTextLineHeight();
    float cardTextGapY = std::round(3.0f * config.uiScale);
    float cardBtnGapY = std::round(6.0f * config.uiScale);
    float cardTextH = cardTextLineH * 2.0f + cardTextGapY;
    float cardH =
        std::round(cardPadY * 2.0f + cardTextH + cardBtnGapY + cardBtnH);
    float cardGap = std::round(8.0f * config.uiScale);
    float cardBottomMargin = std::round(Minimap::PADDING + 18.0f * config.uiScale);
    float cardY = std::round(windowHeight - cardBottomMargin - cardH);

    // Raise "Стан робітника" panel higher so the return-to-auto-step card fits underneath it
    float bottomMargin = cardBottomMargin + cardH + cardGap;
    float panelY = std::round(windowHeight - bottomMargin - panelH);
    lastMissionPanelTopY_ = (easeT > 0.001f) ? panelY : -1.0f;

    // Slide the mission panel (with its handle already open) out from under the monitor after monitor rises
    float attachEase = getHudAttachEase();
    if (attachEase <= 0.001f) {
      lastMissionPanelTopY_ = -1.0f;
      return;
    }

    float handleW = std::round(18.0f * config.uiScale);
    float handleH = std::round(76.0f * config.uiScale);
    float introShiftX =
        std::round((panelW * easeT + handleW) * (1.0f - attachEase));
    float panelX = std::round(monitorLeftX - panelW * easeT) + introShiftX;
    float handleX = panelX - handleW;
    float handleY = std::round(panelY + (panelH - handleH) * 0.5f);

    // 1. Mission indicators panel (clipped strictly at the monitor's left edge so it slides UNDER the monitor)
    if (easeT > 0.001f) {
      ImGui::SetNextWindowPos(ImVec2(panelX, panelY), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_Always);

      ImGuiWindowFlags pFlags =
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(
          ImGuiStyleVar_WindowPadding,
          ImVec2(10.0f * config.uiScale, 8.0f * config.uiScale));

      if (ImGui::Begin("##MissionStatusPanelWindow", nullptr, pFlags)) {
        float clipX = std::floor(monitorLeftX);
        ImGui::PushClipRect(ImVec2(0.0f, 0.0f), ImVec2(clipX, windowHeight), true);

        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImU32 panelBgCol = IM_COL32(23, 20, 26, 240);
        ImU32 panelBorderCol = IM_COL32(122, 41, 61, 230);
        float roundRadius = 8.0f * config.uiScale;

        // Smoothly round only the left corners (top-left and bottom-left).
        // The right side meets the monitor flush and flat with zero gaps!
        dl->AddRectFilled(ImVec2(panelX, panelY),
                          ImVec2(panelX + panelW, panelY + panelH),
                          panelBgCol, roundRadius, ImDrawFlags_RoundCornersLeft);
        dl->AddRect(ImVec2(panelX, panelY),
                    ImVec2(panelX + panelW, panelY + panelH),
                    panelBorderCol, roundRadius, ImDrawFlags_RoundCornersLeft, 1.0f);
        auto statusDisp = getAgentStatusDisplay(engine, maze, renderer,
                                               engine.isManualMode());

        ImGui::Text("Стан робітника:");
        ImGui::SameLine();
        ImGui::TextColored(statusDisp.color, "%s", statusDisp.text.c_str());

        float barH =
            std::max(20.0f * config.uiScale, ImGui::GetFontSize() + 5.0f);
        const auto &agent = engine.getAgent();

        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              ImVec4(0.20f, 0.15f, 0.03f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                              ImVec4(0.64f, 0.48f, 0.04f, 0.95f));
        if (engine.getConfig().energyLimitEnabled) {
          float energyFraction =
              static_cast<float>(agent.getEnergyPercentage()) / 100.0f;
          char energyBuf[64];
          std::snprintf(energyBuf, sizeof(energyBuf), "Енергія: %d / %d од.",
                        agent.getCurrentEnergy(), agent.getMaxEnergy());
          ImGui::ProgressBar(energyFraction, ImVec2(-1, barH), energyBuf);
        } else {
          ImGui::ProgressBar(1.0f, ImVec2(-1, barH), "Енергія: Необмежена");
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(config.uiScale,
                            "Поточний рівень заряду енергії робітника.");
        }

        ImGui::Spacing();

        if (engine.getConfig().backpackLimitEnabled) {
          float bpFraction =
              static_cast<float>(agent.getBackpack().getUtilizationPercent()) /
              100.0f;
          char bpBuf[64];
          std::snprintf(bpBuf, sizeof(bpBuf), "Вантаж: %d / %d кг",
                        agent.getBackpack().getCurrentWeight(),
                        agent.getBackpack().getMaxCapacity());
          ImGui::ProgressBar(bpFraction, ImVec2(-1, barH), bpBuf);
        } else {
          ImGui::ProgressBar(1.0f, ImVec2(-1, barH), "Вантаж: Необмежений");
        }
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(config.uiScale, "Заповненість рюкзака робітника.");
        }

        ImGui::Spacing();

        int curValue = agent.getBackpack().getCurrentValue();
        int targetQuota = config.companyQuota;
        float quotaFraction =
            std::clamp(static_cast<float>(curValue) /
                           static_cast<float>(std::max(1, targetQuota)),
                       0.0f, 1.0f);
        char quotaBuf[64];
        if (curValue >= targetQuota) {
          std::snprintf(quotaBuf, sizeof(quotaBuf),
                        "Квота: %d / %d грн [ВИКОНАНО!]", curValue, targetQuota);
          ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                ImVec4(0.08f, 0.22f, 0.11f, 0.80f));
          ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                                ImVec4(0.10f, 0.46f, 0.18f, 0.95f));
        } else {
          std::snprintf(quotaBuf, sizeof(quotaBuf), "Квота: %d / %d грн (%d%%)",
                        curValue, targetQuota,
                        static_cast<int>(quotaFraction * 100.0f));
          ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                ImVec4(0.24f, 0.08f, 0.12f, 0.65f));
          ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                                ImVec4(1.00f, 0.72f, 0.18f, 1.0f));
        }
        ImGui::ProgressBar(quotaFraction, ImVec2(-1, barH), quotaBuf);
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(config.uiScale,
                            "Прогрес виконання квоти Компанії.");
        }

        ImGui::PopClipRect();
      }
      ImGui::End();
      ImGui::PopStyleVar(3);
    }

    // 1b. Manual mode notice & return-to-auto-step button right underneath the panel
    if (cardEaseT > 0.001f) {
      float cardX = std::round(monitorLeftX - cardW * cardEaseT);
      ImGui::SetNextWindowPos(ImVec2(cardX, cardY), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(cardW, cardH), ImGuiCond_Always);

      ImGuiWindowFlags cFlags =
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                          ImVec2(cardPadX, cardPadY));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.0f, 1.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                          ImVec2(0.0f, cardTextGapY));

      if (ImGui::Begin("##ManualControlCardWindow", nullptr, cFlags)) {
        float clipX = std::floor(monitorLeftX);
        ImGui::PushClipRect(ImVec2(0.0f, 0.0f), ImVec2(clipX, windowHeight), true);

        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImU32 cardBgCol = IM_COL32(28, 22, 24, 242);
        ImU32 cardBorderCol = IM_COL32(180, 110, 35, 235);
        float roundRadius = 8.0f * config.uiScale;

        dl->AddRectFilled(ImVec2(cardX, cardY),
                          ImVec2(cardX + cardW, cardY + cardH),
                          cardBgCol, roundRadius, ImDrawFlags_RoundCornersLeft);
        dl->AddRect(ImVec2(cardX, cardY),
                    ImVec2(cardX + cardW, cardY + cardH),
                    cardBorderCol, roundRadius, ImDrawFlags_RoundCornersLeft, 1.0f);

        ImGui::TextColored(ImVec4(1.0f, 0.80f, 0.28f, 1.0f),
                           "Режим: Ручне керування.");
        ImGui::TextColored(ImVec4(0.95f, 0.62f, 0.45f, 1.0f),
                           "Автоматичне керування недоступне.");

        ImGui::SetCursorPosY(cardPadY + cardTextH + cardBtnGapY);

        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.68f, 0.34f, 0.10f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.88f, 0.46f, 0.16f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.98f, 0.56f, 0.20f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Border,
                              ImVec4(1.00f, 0.78f, 0.30f, 0.95f));

        if (ImGui::Button(returnBtnLabel, ImVec2(-1, cardBtnH))) {
          state.requestReturnToLastAutoStep = true;
        }
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(
              config.uiScale,
              "Повернути робітника на останню клітинку автоматичного маршруту\n"
              "та відновити керування через [Space] / [Shift+Space] / "
              "[Ctrl+Space].");
        }
        ImGui::PopStyleColor(4);

        ImGui::PopClipRect();
      }
      ImGui::End();
      ImGui::PopStyleVar(5);
    }

    // 2. Toggle button on left edge (moves out together with panel after monitor finishes rising)
    float morphFactor = std::clamp((0.25f - easeT) / 0.25f, 0.0f, 1.0f);
    float missionWaveMorph = morphFactor * morphFactor * (3.0f - 2.0f * morphFactor);

    float curFlare = 28.0f * config.uiScale * missionWaveMorph;
    float winPad = 4.0f;
    float winY = handleY - curFlare;
    float winH = handleH + 2.0f * curFlare;
    float winW = handleW + 8.0f; // Extra width to allow overlap into panel without window clipping

    ImGui::SetNextWindowPos(ImVec2(handleX - winPad, winY - winPad), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(winW + winPad * 2.0f, winH + winPad * 2.0f), ImGuiCond_Always);

    ImGuiWindowFlags hFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##MissionPanelGripWindow", nullptr, hFlags)) {
      ImGui::PushClipRect(ImVec2(0.0f, 0.0f),
                          ImVec2(std::floor(monitorLeftX), windowHeight), true);
      ImGui::SetCursorPos(ImVec2(winPad, winPad));
      if (ImGui::InvisibleButton("##MissionGripBtn",
                                 ImVec2(winW, winH))) {
        missionPanelCollapsed_ = !missionPanelCollapsed_;
      }

      bool isRawHovered = ImGui::IsItemHovered();
      bool isHovered = false;

      if (isRawHovered) {
        ImVec2 m = ImGui::GetIO().MousePos;
        if (missionWaveMorph <= 0.05f) {
          isHovered = (m.x >= handleX - 2.0f && m.x <= panelX + 2.0f &&
                       m.y >= handleY - 2.0f && m.y <= handleY + handleH + 2.0f);
        } else {
          float wallX = panelX;
          if (m.y >= handleY && m.y <= handleY + handleH) {
            isHovered = (m.x >= handleX - 2.0f && m.x <= wallX + 2.0f);
          } else if (m.y < handleY && m.y >= winY) {
            float span = curFlare + 10.0f * config.uiScale * missionWaveMorph;
            float t = span > 0.001f ? (m.y - winY) / span : 1.0f;
            t = std::clamp(t, 0.0f, 1.0f);
            float curveX = wallX - handleW * (t * t * (3.0f - 2.0f * t));
            isHovered = (m.x >= curveX - 2.0f && m.x <= wallX + 2.0f);
          } else if (m.y > handleY + handleH && m.y <= winY + winH) {
            float span = curFlare + 10.0f * config.uiScale * missionWaveMorph;
            float t = span > 0.001f ? ((winY + winH) - m.y) / span : 1.0f;
            t = std::clamp(t, 0.0f, 1.0f);
            float curveX = wallX - handleW * (t * t * (3.0f - 2.0f * t));
            isHovered = (m.x >= curveX - 2.0f && m.x <= wallX + 2.0f);
          }
        }
      }

      bool isActive = ImGui::IsItemActive();

      if (isHovered || isActive) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        showStyledTooltip(config.uiScale, "%s",
                          missionPanelCollapsed_
                              ? "[Клік]: Показати показники місії"
                              : "[Клік]: Сховати показники місії");
      }

      ImDrawList *dl = ImGui::GetWindowDrawList();

      // Smooth color transitions between dock and floating handle matching main sidebar
      float colorT = std::clamp(easeT / 0.25f, 0.0f, 1.0f);
      colorT = colorT * colorT * (3.0f - 2.0f * colorT);
      uint8_t baseR = static_cast<uint8_t>(65 - 19 * colorT);
      uint8_t baseG = static_cast<uint8_t>(26);
      uint8_t baseB = static_cast<uint8_t>(35 - 3 * colorT);
      uint8_t bdrR = static_cast<uint8_t>(245 - 135 * colorT);
      uint8_t bdrG = static_cast<uint8_t>(120 - 75 * colorT);
      uint8_t bdrB = static_cast<uint8_t>(40 + 18 * colorT);

      ImU32 normalBg = IM_COL32(baseR, baseG, baseB,
                                static_cast<uint8_t>(235 - 15 * colorT));
      ImU32 normalBorder =
          IM_COL32(bdrR, bdrG, bdrB, static_cast<uint8_t>(240 - 20 * colorT));

      ImU32 bgCol = isActive
                        ? IM_COL32(255, 140, 20, 245)
                        : isHovered ? IM_COL32(235, 48, 68, 240)
                                    : normalBg;
      ImU32 bdrCol = (isHovered || isActive) ? IM_COL32(255, 255, 255, 255)
                                             : normalBorder;

      float wallX = panelX;
      float topY = handleY;
      float botY = handleY + handleH;
      float cornerInset = 10.0f * config.uiScale * missionWaveMorph;

      if (missionWaveMorph > 0.01f) {
        // Flowing wave shape merging smoothly into the monitor casing (x = wallX)
        ImVec2 pTopWall(wallX, topY - curFlare);
        ImVec2 pTopHandle(handleX, topY + cornerInset);
        ImVec2 pTopCp1(wallX, topY - curFlare * 0.25f);
        ImVec2 pTopCp2(handleX, topY);

        ImVec2 pBotHandle(handleX, botY - cornerInset);
        ImVec2 pBotWall(wallX, botY + curFlare);
        ImVec2 pBotCp1(handleX, botY);
        ImVec2 pBotCp2(wallX, botY + curFlare * 0.25f);

        dl->PathClear();
        dl->PathLineTo(pTopWall);
        dl->PathBezierCubicCurveTo(pTopCp1, pTopCp2, pTopHandle, 24);
        dl->PathLineTo(pBotHandle);
        dl->PathBezierCubicCurveTo(pBotCp1, pBotCp2, pBotWall, 24);

        int nPts = dl->_Path.Size;
        if (nPts >= 2) {
          const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
          dl->PrimReserve((nPts - 1) * 6, nPts * 2);
          ImDrawIdx vtxBase = static_cast<ImDrawIdx>(dl->_VtxCurrentIdx);

          for (int i = 0; i < nPts; ++i) {
            ImVec2 lPt = dl->_Path[i];
            ImVec2 rPt(wallX, lPt.y);
            dl->PrimWriteVtx(lPt, uv, bgCol);
            dl->PrimWriteVtx(rPt, uv, bgCol);
          }

          for (int i = 0; i < nPts - 1; ++i) {
            ImDrawIdx i0 = vtxBase + static_cast<ImDrawIdx>(i * 2);
            ImDrawIdx i1 = i0 + 1;
            ImDrawIdx i2 = i0 + 2;
            ImDrawIdx i3 = i0 + 3;
            dl->PrimWriteIdx(i0);
            dl->PrimWriteIdx(i1);
            dl->PrimWriteIdx(i3);
            dl->PrimWriteIdx(i0);
            dl->PrimWriteIdx(i3);
            dl->PrimWriteIdx(i2);
          }
        }

        // Draw flowing outer border stroke (left edge)
        dl->PathStroke(bdrCol, false, 1.8f);

        // Also draw border on the right edge along wallX when hovered or active:
        dl->AddLine(ImVec2(wallX, topY - curFlare), ImVec2(wallX, botY + curFlare), bdrCol, 1.2f);
      } else {
        // Rounded grip handle attached flush to panel without any seam or gap
        float handleRounding = 6.0f * config.uiScale;
        // Extend 2px into panelX so there is ZERO gap/hole between handle and panel!
        dl->AddRectFilled(ImVec2(handleX, handleY),
                          ImVec2(panelX + 2.0f, handleY + handleH), bgCol,
                          handleRounding, ImDrawFlags_RoundCornersLeft);

        // Entire outer border around the handle (complete crisp white perimeter on hover)
        dl->AddRect(ImVec2(handleX, handleY),
                    ImVec2(handleX + handleW, handleY + handleH), bdrCol,
                    handleRounding, ImDrawFlags_RoundCornersLeft, 1.2f);
      }

      // Chevron arrow: if collapsed, point left '<'; if open, point right '>'
      ImU32 iconColor = (isHovered || isActive) ? IM_COL32(20, 20, 20, 240)
                        : (easeT < 0.25f)       ? IM_COL32(255, 210, 120, 255)
                                                : IM_COL32(195, 120, 135, 220);
      float midX = handleX + handleW * 0.5f;
      float midY = handleY + handleH * 0.5f;
      float arrowSize = 4.0f * config.uiScale;

      if (missionPanelCollapsed_) {
        // Pointing Left '<' (expand to left)
        dl->AddLine(ImVec2(midX + arrowSize * 0.6f, midY - arrowSize * 1.3f),
                    ImVec2(midX - arrowSize * 0.6f, midY), iconColor, 2.0f);
        dl->AddLine(ImVec2(midX - arrowSize * 0.6f, midY),
                    ImVec2(midX + arrowSize * 0.6f, midY + arrowSize * 1.3f),
                    iconColor, 2.0f);
      } else {
        // Pointing Right '>' (collapse into monitor)
        dl->AddLine(ImVec2(midX - arrowSize * 0.6f, midY - arrowSize * 1.3f),
                    ImVec2(midX + arrowSize * 0.6f, midY), iconColor, 2.0f);
        dl->AddLine(ImVec2(midX + arrowSize * 0.6f, midY),
                    ImVec2(midX - arrowSize * 0.6f, midY + arrowSize * 1.3f),
                    iconColor, 2.0f);
      }

      // Tactical grip ridges above and below chevron
      float lineInset = 3.5f * config.uiScale;
      for (float off : {-20.0f * config.uiScale, -12.0f * config.uiScale,
                        12.0f * config.uiScale, 20.0f * config.uiScale}) {
        dl->AddLine(ImVec2(handleX + lineInset, midY + off),
                    ImVec2(handleX + handleW - lineInset, midY + off),
                    iconColor, 1.2f);
      }
      ImGui::PopClipRect();
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
  }

  void renderWorldTooltip(
      const map::Maze &maze, const simulation::SimulationEngine &engine,
      const sf::Vector2f &worldPos, bool mouseOverUI,
      const sf::Vector2i &mousePixel = {0, 0}, float windowWidth = 1920.0f,
      float windowHeight = 1080.0f, bool isExitDisabled = false,
      float scanRadius = -1.0f, const sf::Vector2f &scanOrigin = {0.0f, 0.0f},
      bool isSonarScanning = false, float routeProgress = 1.0f,
      bool isFadingOut = false) {
    if (mouseOverUI || isFadingOut)
      return;

    // Ensure mouse cursor is within active window bounds and ImGui has valid
    // position
    if (mousePixel.x < 0 || mousePixel.x >= static_cast<int>(windowWidth) ||
        mousePixel.y < 0 || mousePixel.y >= static_cast<int>(windowHeight)) {
      return;
    }

    const auto &io = ImGui::GetIO();
    if (io.MousePos.x < 0.0f || io.MousePos.y < 0.0f) {
      return;
    }

    int tileX = static_cast<int>(
        std::floor(worldPos.x / ProceduralRenderer::TILE_SIZE));
    int tileY = static_cast<int>(
        std::floor(worldPos.y / ProceduralRenderer::TILE_SIZE));

    core::Point startPt = maze.getStartPoint();
    sf::Vector2i inDir{0, 0};
    const auto &plan = engine.getPlan();
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
    sf::Vector2f startArrowCenter{
        (cellBeforeStart.x + 0.5f) * ProceduralRenderer::TILE_SIZE,
        (cellBeforeStart.y + 0.5f) * ProceduralRenderer::TILE_SIZE};

    sf::Vector2i cellAfterExit{0, 0};
    sf::Vector2f exitArrowCenter{0.0f, 0.0f};
    bool hasExitArrow = !isExitDisabled;
    if (hasExitArrow) {
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
      cellAfterExit = {exitPt.x + outDir.x, exitPt.y + outDir.y};
      exitArrowCenter = {
          (cellAfterExit.x + 0.5f) * ProceduralRenderer::TILE_SIZE,
          (cellAfterExit.y + 0.5f) * ProceduralRenderer::TILE_SIZE};
    }

    core::Point cellPoint{tileX, tileY};

    bool isHoveringStartArrow =
        (std::hypot(worldPos.x - startArrowCenter.x,
                    worldPos.y - startArrowCenter.y) <=
         ProceduralRenderer::TILE_SIZE * 0.70f) ||
        (cellPoint.x == cellBeforeStart.x && cellPoint.y == cellBeforeStart.y);

    bool isHoveringExitArrow =
        hasExitArrow &&
        ((std::hypot(worldPos.x - exitArrowCenter.x,
                     worldPos.y - exitArrowCenter.y) <=
          ProceduralRenderer::TILE_SIZE * 0.70f) ||
         (cellPoint.x == cellAfterExit.x && cellPoint.y == cellAfterExit.y));

    if (!isHoveringStartArrow && !isHoveringExitArrow &&
        !maze.isValid(tileX, tileY)) {
      return;
    }

    // While sonar scanning animation is active, do not show tooltip for cells
    // or arrows that have not been revealed yet by the sonar wave wavefront!
    if (isSonarScanning && scanRadius >= 0.0f) {
      float checkX = 0.0f;
      float checkY = 0.0f;
      if (isHoveringStartArrow) {
        checkX = startArrowCenter.x;
        checkY = startArrowCenter.y;
      } else if (isHoveringExitArrow) {
        checkX = exitArrowCenter.x;
        checkY = exitArrowCenter.y;
      } else {
        checkX = (tileX + 0.5f) * ProceduralRenderer::TILE_SIZE;
        checkY = (tileY + 0.5f) * ProceduralRenderer::TILE_SIZE;
      }
      float distToOrigin =
          std::hypot(checkX - scanOrigin.x, checkY - scanOrigin.y);
      if (distToOrigin > scanRadius + 14.0f) {
        return; // Unscanned: not yet reached by sonar wavefront
      }
    }

    const map::Cell *pCell =
        maze.isValid(cellPoint) ? &maze.at(cellPoint) : nullptr;
    const auto &agent = engine.getAgent();

    // Check if mouse is hovering near/on the Agent
    bool isHoveringAgent = false;
    if (maze.isValid(cellPoint) && agent.getPosition() == cellPoint) {
      isHoveringAgent = true;
    } else {
      float agentWorldX =
          agent.getPosition().x * ProceduralRenderer::TILE_SIZE +
          ProceduralRenderer::TILE_SIZE * 0.5f;
      float agentWorldY =
          agent.getPosition().y * ProceduralRenderer::TILE_SIZE +
          ProceduralRenderer::TILE_SIZE * 0.5f;
      float dist =
          std::hypot(worldPos.x - agentWorldX, worldPos.y - agentWorldY);
      if (dist <= ProceduralRenderer::TILE_SIZE * 0.55f) {
        isHoveringAgent = true;
      }
    }

    // Check if hovering over scrap item
    std::optional<core::Item> itemHere;
    bool isCollected = false;
    if (pCell && pCell->hasResource()) {
      itemHere = maze.getItemById(pCell->getResourceId());
      if (itemHere.has_value()) {
        const auto &collected = engine.getCollectedItemIds();
        isCollected = (std::find(collected.begin(), collected.end(),
                                 itemHere->id) != collected.end());
      }
    }

    // Check if mouse is directly hovering the drawn route line (within line
    // thickness + glow radius)
    bool isHoveringRouteLine = false;
    if (pCell && !plan.fullPath.empty() && routeProgress > 0.001f) {
      size_t maxSeg = plan.fullPath.size() - 1;
      if (routeProgress < 0.999f) {
        maxSeg = static_cast<size_t>(
            std::clamp(routeProgress * static_cast<float>(plan.fullPath.size()),
                       0.0f, static_cast<float>(plan.fullPath.size() - 1)));
      }
      for (size_t i = 0; i < maxSeg; ++i) {
        const auto &pA = plan.fullPath[i];
        const auto &pB = plan.fullPath[i + 1];
        if (pA == cellPoint || pB == cellPoint) {
          sf::Vector2f aWorld((pA.x + 0.5f) * ProceduralRenderer::TILE_SIZE,
                              (pA.y + 0.5f) * ProceduralRenderer::TILE_SIZE);
          sf::Vector2f bWorld((pB.x + 0.5f) * ProceduralRenderer::TILE_SIZE,
                              (pB.y + 0.5f) * ProceduralRenderer::TILE_SIZE);
          sf::Vector2f ab = bWorld - aWorld;
          float l2 = ab.x * ab.x + ab.y * ab.y;
          float dist = 999.0f;
          if (l2 < 1e-4f) {
            dist = std::hypot(worldPos.x - aWorld.x, worldPos.y - aWorld.y);
          } else {
            float t = std::clamp(((worldPos.x - aWorld.x) * ab.x +
                                  (worldPos.y - aWorld.y) * ab.y) /
                                     l2,
                                 0.0f, 1.0f);
            sf::Vector2f proj = aWorld + t * ab;
            dist = std::hypot(worldPos.x - proj.x, worldPos.y - proj.y);
          }
          if (dist <= 7.0f) {
            isHoveringRouteLine = true;
            break;
          }
        }
      }
    }

    // Wall hover filtering:
    // 1. Enclosed interior walls (surrounded by corridors/rooms) -> always show
    // tooltip for all cells.
    // 2. Exterior walls (facing the void) -> only show tooltip within 2 cells
    // of an exterior corridor/room.
    // 3. Gateway cells (before start or after exit) -> never show wall tooltip.
    if (!isHoveringStartArrow && !isHoveringExitArrow && !isHoveringAgent &&
        pCell && pCell->getType() == core::CellType::Wall) {
      if ((cellPoint.x == cellBeforeStart.x &&
           cellPoint.y == cellBeforeStart.y) ||
          (cellPoint.x == cellAfterExit.x && cellPoint.y == cellAfterExit.y)) {
        return;
      }
      bool isExterior = maze.isExteriorWall(cellPoint.x, cellPoint.y);
      if (isExterior) {
        bool isWithin2Cells = false;
        for (int dy = -2; dy <= 2 && !isWithin2Cells; ++dy) {
          for (int dx = -2; dx <= 2 && !isWithin2Cells; ++dx) {
            if (dx == 0 && dy == 0)
              continue;
            int nx = cellPoint.x + dx;
            int ny = cellPoint.y + dy;
            if (maze.isValid(nx, ny) && maze.at(nx, ny).isPassable()) {
              isWithin2Cells = true;
            }
          }
        }
        if (!isWithin2Cells) {
          return;
        }
      }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.09f, 0.08f, 0.09f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.52f, 0.18f, 0.26f, 0.85f));

    ImGuiWindowFlags tipFlags =
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    static ImVec2 s_lastTooltipSize(380.0f, 95.0f);

    float mouseX = static_cast<float>(mousePixel.x);
    float mouseY = static_cast<float>(mousePixel.y);

    float pivotX = 0.0f;
    float posX = mouseX + 28.0f;
    if (posX + s_lastTooltipSize.x > windowWidth - 12.0f ||
        mouseX > windowWidth - 220.0f) {
      posX = mouseX - 16.0f;
      pivotX = 1.0f;
    }

    float pivotY = 0.0f;
    float posY = mouseY + 24.0f;
    if (posY + s_lastTooltipSize.y > windowHeight - 12.0f ||
        mouseY > windowHeight - 180.0f) {
      posY = mouseY - 16.0f;
      pivotY = 1.0f;
    }

    posX = std::clamp(posX, 10.0f, windowWidth - 10.0f);
    posY = std::clamp(posY, 10.0f, windowHeight - 10.0f);

    ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Always,
                            ImVec2(pivotX, pivotY));

    if (ImGui::Begin("##TacticalWorldTooltip", nullptr, tipFlags)) {
      ImGui::PushTextWrapPos(std::min(460.0f, windowWidth - 32.0f));

      if (isHoveringStartArrow) {
        ImGui::TextColored(ImVec4(0.95f, 0.55f, 0.1f, 1.0f),
                           "Шлюз (Старт) — Вхідна стрілка");
        ImGui::Separator();
        ImGui::TextUnformatted(
            "Вектор заходу робітника у комплекс через початковий шлюз");
        ImGui::Text("Координати шлюзу: [%d, %d]", startPt.x, startPt.y);
      } else if (isHoveringExitArrow) {
        ImGui::TextColored(ImVec4(0.18f, 0.80f, 0.44f, 1.0f),
                           "Вихід (Евакуація) — Стрілка евакуації");
        ImGui::Separator();
        ImGui::TextUnformatted(
            "Вектор виходу та евакуації робітника з комплексу");
        ImGui::Text("Координати шлюзу: [%d, %d]", maze.getExitPoint().x,
                    maze.getExitPoint().y);
      } else if (isHoveringAgent) {
        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "Робітник (Агент)");
        ImGui::Separator();
        ImGui::TextUnformatted(
            "Поточна позиція автономного робітника (GPS тактичний маяк)");
        ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
        ImGui::Text("Стан: %s", agent.getStatusString().c_str());
        ImGui::Text("Запас енергії: %d / %d од.", agent.getCurrentEnergy(),
                    agent.getMaxEnergy());
        ImGui::Text("Вантаж: %d / %d кг (%d грн)",
                    agent.getBackpack().getCurrentWeight(),
                    agent.getBackpack().getMaxCapacity(),
                    agent.getBackpack().getCurrentValue());
      } else if (itemHere.has_value() && !isCollected) {
        if (itemHere->category == core::ItemCategory::LightScrap) {
          ImGui::TextColored(ImVec4(0.20f, 0.41f, 1.00f, 1.0f),
                             "Дрібний брухт: %s", itemHere->name.c_str());
        } else if (itemHere->category == core::ItemCategory::MediumScrap) {
          ImGui::TextColored(ImVec4(1.00f, 1.00f, 0.00f, 1.0f),
                             "Побутовий брухт: %s", itemHere->name.c_str());
        } else {
          ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
                             "Важкий брухт: %s", itemHere->name.c_str());
        }
        ImGui::Separator();
        ImGui::Text("Категорія: %s",
                    core::itemCategoryToString(itemHere->category).c_str());
        ImGui::Text("Вага: %d кг", itemHere->weight);
        ImGui::Text("Цінність: %d грн", itemHere->value);
        ImGui::Text("Питома цінність (V/W): %.2f грн/кг",
                    itemHere->getValueDensity());
        ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
      } else if (isHoveringRouteLine) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.9f, 1.0f),
                           "Лінія маршруту (TSP)");
        ImGui::Separator();
        ImGui::TextUnformatted(
            "Розрахована лінія оптимального шляху робітника");
        ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
      } else if (itemHere.has_value() && isCollected) {
        if (itemHere->category == core::ItemCategory::LightScrap) {
          ImGui::TextColored(ImVec4(0.20f, 0.41f, 1.00f, 1.0f),
                             "Дрібний брухт: %s", itemHere->name.c_str());
        } else if (itemHere->category == core::ItemCategory::MediumScrap) {
          ImGui::TextColored(ImVec4(1.00f, 1.00f, 0.00f, 1.0f),
                             "Побутовий брухт: %s", itemHere->name.c_str());
        } else {
          ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
                             "Важкий брухт: %s", itemHere->name.c_str());
        }
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.55f, 1.0f),
                           "Статус: Вже підібрано робітником у рюкзак");
        ImGui::Text("Категорія: %s",
                    core::itemCategoryToString(itemHere->category).c_str());
        ImGui::Text("Вага: %d кг", itemHere->weight);
        ImGui::Text("Цінність: %d грн", itemHere->value);
        ImGui::Text("Питома цінність (V/W): %.2f грн/кг",
                    itemHere->getValueDensity());
        ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
      } else if (pCell && pCell->getType() == core::CellType::Start) {
        ImGui::TextColored(ImVec4(0.95f, 0.55f, 0.1f, 1.0f), "Шлюз (Старт)");
        ImGui::Separator();
        ImGui::TextUnformatted(
            "Початкова точка, звідки робітник починає місію");
        ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
      } else if (pCell && pCell->getType() == core::CellType::Exit) {
        if (isExitDisabled) {
          ImGui::TextColored(ImVec4(0.85f, 0.35f, 0.35f, 1.0f),
                             "Вихід (Недоступний / Заблокований)");
          ImGui::Separator();
          ImGui::TextUnformatted(
              "Режим евакуації: 'A -> A (База)'. Шлюз законсервовано, вихід "
              "лише через початковий шлюз (A).");
          ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
        } else {
          ImGui::TextColored(ImVec4(0.18f, 0.80f, 0.44f, 1.0f),
                             "Вихід (Евакуація)");
          ImGui::Separator();
          ImGui::TextUnformatted(
              "Шлюз евакуації, куди робітник прямує після збору брухту");
          ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
        }
      } else if (pCell && pCell->getType() == core::CellType::Mud) {
        ImGui::TextColored(ImVec4(0.85f, 0.65f, 0.35f, 1.0f),
                           "Багнюка / Болото / Вода");
        ImGui::Separator();
        ImGui::TextUnformatted("Важкопрохідна ділянка: витрата енергії +25 "
                               "прямо, +35 по діагоналі");
        ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
        if (pCell->isWalked()) {
          ImGui::TextColored(ImVec4(0.35f, 0.88f, 0.80f, 1.0f),
                             "Пройдений слід: Блакитно-коричнева пройдена "
                             "болотяна клітинка");
        }
      } else if (pCell && pCell->isWalked()) {
        ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.95f, 1.0f), "Пройдений слід");
        ImGui::Separator();
        ImGui::TextUnformatted("Клітинка, яку робітник уже пройшов");
        ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
      } else if (pCell && pCell->getType() == core::CellType::Wall) {
        if ((cellPoint.x == cellBeforeStart.x &&
             cellPoint.y == cellBeforeStart.y) ||
            (cellPoint.x == cellAfterExit.x &&
             cellPoint.y == cellAfterExit.y)) {
          return;
        }
        ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), "Стіна комплексу");
        ImGui::Separator();
        ImGui::TextUnformatted("Непрохідна металева перешкода лабіринту");
        ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
      } else if (pCell) {
        const map::Room *inRoom = nullptr;
        for (const auto &r : maze.getRooms()) {
          if (r.contains(cellPoint)) {
            inRoom = &r;
            break;
          }
        }
        if (inRoom != nullptr) {
          ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f),
                             "Кімната комплексу");
          ImGui::Separator();
          ImGui::Text("Сектор кімнати (#%d)", inRoom->id + 1);
          ImGui::TextUnformatted(
              "Вільний прохід комплексу (витрата: 10 прямо, 14 по діагоналі)");
          ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
        } else {
          ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f),
                             "Коридор комплексу");
          ImGui::Separator();
          ImGui::TextUnformatted(
              "Вільний коридор комплексу (витрата: 10 прямо, 14 по діагоналі)");
          ImGui::Text("Координати: [%d, %d]", cellPoint.x, cellPoint.y);
        }
      }

      ImGui::PopTextWrapPos();
      s_lastTooltipSize = ImGui::GetWindowSize();
      ImGui::End();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
  }

  void renderMinimapZoomBadge(float cameraZoom, float windowWidth,
                              float windowHeight, float minimapScale,
                              float /*uiScale*/, float alpha) {
    if (alpha <= 0.01f)
      return;

    // Magnification matching the sidebar camera settings exactly
    float currentMagnification = 1.0f / std::max(0.01f, cameraZoom);
    char zoomBuf[32];
    std::snprintf(zoomBuf, sizeof(zoomBuf), "%.2fx", currentMagnification);

    auto l = ui::Minimap::calculateLayout(windowWidth, windowHeight,
                                          minimapScale, 100.0f, 100.0f);

    float a = std::clamp(alpha, 0.0f, 1.0f);

    // Calculate text sizes so badge comfortably wraps contents
    ImVec2 valSz = ImGui::CalcTextSize(zoomBuf);
    float padX = 7.0f * l.s;

    float badgeW = std::max(48.0f * l.s, valSz.x + padX * 2.0f);
    float badgeH = std::max(20.0f * l.s, valSz.y + 7.0f * l.s);

    // Positioned safely below the rounded corner bezel (cornerR = 26.0f * l.s)
    // l.screenY + 32.0f * l.s places the badge cleanly on the vertical straight
    // edge of the screen
    float badgeX = l.screenX + l.screenW - badgeW - 12.0f * l.s;
    float badgeY = l.screenY + 32.0f * l.s;

    ImGui::SetNextWindowPos(ImVec2(badgeX, badgeY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(badgeW, badgeH), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##MinimapZoomBadge", nullptr, flags)) {
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      ImVec2 pMin(badgeX, badgeY);
      ImVec2 pMax(badgeX + badgeW, badgeY + badgeH);

      ImU32 bgCol = IM_COL32(10, 16, 24, static_cast<int>(210.0f * a));
      ImU32 borderCol = IM_COL32(0, 230, 245, static_cast<int>(180.0f * a));
      ImU32 textValCol = IM_COL32(235, 255, 255, static_cast<int>(255.0f * a));
      ImU32 gaugeBgCol = IM_COL32(0, 45, 55, static_cast<int>(180.0f * a));
      ImU32 gaugeFillCol = IM_COL32(0, 245, 255, static_cast<int>(240.0f * a));
      ImU32 markCol = IM_COL32(255, 220, 60, static_cast<int>(200.0f * a));

      // Tactical rounded container
      float roundR = 3.5f * l.s;
      drawList->AddRectFilled(pMin, pMax, bgCol, roundR);
      drawList->AddRect(pMin, pMax, borderCol, roundR, 0, 1.0f);

      // Centered magnification text (e.g. "0.80x")
      float gaugeMarginX = 6.0f * l.s;
      float gaugeH = 2.0f * l.s;
      float gaugeY = badgeY + badgeH - gaugeH - 2.5f * l.s;
      float gaugeW = badgeW - gaugeMarginX * 2.0f;

      float startTextX = badgeX + (badgeW - valSz.x) * 0.5f;
      float textY = badgeY + (badgeH - gaugeH - 2.5f * l.s - valSz.y) * 0.5f;

      drawList->AddText(ImVec2(startTextX, textY), textValCol, zoomBuf);

      // Mini tactical gauge at bottom matching sidebar slider range [0.20x
      // .. 4.50x]
      float normT = std::clamp((currentMagnification - 0.20f) / (4.50f - 0.20f),
                               0.0f, 1.0f);
      drawList->AddRectFilled(
          ImVec2(badgeX + gaugeMarginX, gaugeY),
          ImVec2(badgeX + gaugeMarginX + gaugeW, gaugeY + gaugeH), gaugeBgCol,
          1.0f);
      if (normT > 0.01f) {
        drawList->AddRectFilled(
            ImVec2(badgeX + gaugeMarginX, gaugeY),
            ImVec2(badgeX + gaugeMarginX + gaugeW * normT, gaugeY + gaugeH),
            gaugeFillCol, 1.0f);
      }
      // 1.00x normal scale tick mark on the gauge
      float norm1x = (1.00f - 0.20f) / (4.50f - 0.20f);
      float markX = badgeX + gaugeMarginX + gaugeW * norm1x;
      drawList->AddLine(ImVec2(markX, gaugeY - 1.0f),
                        ImVec2(markX, gaugeY + gaugeH + 1.0f), markCol, 1.0f);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
  }

private:
  struct SmoothScroller {
    float targetScrollY = 0.0f;
    float smoothScrollY = 0.0f;
    bool initialized = false;

    void reset() {
      targetScrollY = 0.0f;
      smoothScrollY = 0.0f;
      initialized = false;
    }

    void update(float scrollStepPerUnit, float uiScale) {
      ImGui::SetScrollX(0.0f);

      float curScroll = ImGui::GetScrollY();
      float maxScroll = ImGui::GetScrollMaxY();

      if (!initialized) {
        targetScrollY = curScroll;
        smoothScrollY = curScroll;
        initialized = true;
      }

      targetScrollY = std::clamp(targetScrollY, 0.0f, maxScroll);

      float wheel = 0.0f;
      if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
        wheel = ImGui::GetIO().MouseWheel;
      }

      bool isUserDraggingScrollbar =
          (std::abs(curScroll - smoothScrollY) > 0.5f &&
           std::abs(wheel) < 0.001f && ImGui::GetIO().MouseDown[0]);

      if (isUserDraggingScrollbar) {
        targetScrollY = curScroll;
        smoothScrollY = curScroll;
      } else {
        if (std::abs(wheel) > 0.001f) {
          targetScrollY -= wheel * scrollStepPerUnit * uiScale;
          targetScrollY = std::clamp(targetScrollY, 0.0f, maxScroll);
        }

        float dt = ImGui::GetIO().DeltaTime;
        float speed = 19.0f;
        float diff = targetScrollY - smoothScrollY;
        if (std::abs(diff) < 0.25f) {
          smoothScrollY = targetScrollY;
        } else {
          smoothScrollY += diff * std::clamp(dt * speed, 0.0f, 1.0f);
        }
        ImGui::SetScrollY(smoothScrollY);
      }
    }
  };

  SmoothScroller sidebarScroller_;
  SmoothScroller legendScroller_;

  float sidebarWidth_ = DEFAULT_SIDEBAR_WIDTH;
  float savedSidebarBaseWidth_ = DEFAULT_SIDEBAR_WIDTH;
  float sidebarOpenProgress_ = 0.0f;
  bool sidebarCollapsed_ = true;
  bool isDraggingHandle_ = false;
  float dragAccumX_ = 0.0f;
  float legendAnimT_ = 0.0f;
  float collapsedLegendH_ = 0.0f;
  float lastUiScale_ = 1.0f;
  bool isCatalogOpen_ = false;
  bool isCatalogClosing_ = false;
  float catalogOpenProgress_ = 0.0f;
  float catalogScrollOnCloseStart_ = 0.0f;
  bool catalogAutoScrollActive_ = false;
  float catalogAutoScrollStart_ = 0.0f;
  float catalogAutoScrollTarget_ = 0.0f;
  float catalogAutoScrollTime_ = 0.0f;
  bool isCollectedOpen_ = false;
  bool isCollectedClosing_ = false;
  float collectedOpenProgress_ = 0.0f;
  float collectedScrollOnCloseStart_ = 0.0f;
  float measuredCollectedContentH_ = 110.0f;
  bool collectedAutoScrollActive_ = false;
  float collectedAutoScrollStart_ = 0.0f;
  float collectedAutoScrollTarget_ = 0.0f;
  float collectedAutoScrollTime_ = 0.0f;
  int hoveredCollectedItemId_ = -1;
  std::string hoveredCatalogItemName_;
  std::string lastHoveredCatalogItemName_;
  int hoveredCatalogCategory_ = -1;
  int lastHoveredCatalogCategory_ = -1;
  float handleWaveMorph_ = 1.0f;
  bool startupHintDismissed_ = false;
  float startupHintTimer_ = 0.0f;
  bool missionPanelCollapsed_ = true;
  float missionPanelOpenProgress_ = 0.0f;
  float manualBannerOpenProgress_ = 0.0f;
  mutable float lastMissionPanelTopY_ = -1.0f;
  bool hudIntroActive_ = false;
  float hudIntroProgress_ = 1.0f;
  float hudAttachProgress_ = 1.0f;
};

} // namespace ui
