#pragma once

#include "CameraController.hpp"
#include "Localization.hpp"
#include "Minimap.hpp"
#include "ProceduralRenderer.hpp"
#include "core/Config.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "io/CsvExporter.hpp"
#include "map/Maze.hpp"
#include "simulation/SimulationEngine.hpp"
#include <algorithm>
#include <cmath>
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
  [[nodiscard]] float getSavedSidebarBaseWidth() const noexcept {
    return savedSidebarBaseWidth_;
  }
  void setSavedSidebarBaseWidth(float w) noexcept {
    savedSidebarBaseWidth_ = w;
    sidebarWidth_ = w;
  }
  [[nodiscard]] float getSidebarOpenProgress() const noexcept {
    return sidebarOpenProgress_;
  }
  void setSidebarOpenProgress(float progress) noexcept {
    sidebarOpenProgress_ = progress;
  }
  void setStartupHintDismissed(bool dismissed) noexcept {
    startupHintDismissed_ = dismissed;
  }
  [[nodiscard]] float getSavedMissionPanelBaseWidth() const noexcept {
    return savedMissionPanelBaseWidth_;
  }
  void setSavedMissionPanelBaseWidth(float w) noexcept {
    savedMissionPanelBaseWidth_ = w;
    missionPanelWidth_ = w;
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

  void startTour() noexcept {
    isTourActive_ = true;
    tourCurrentStep_ = 0;
    lastTourStep_ = -1;
    escHoldTimer_ = 0.0f;
    tourTextAlpha_ = 0.0f;
    tourDimAlpha_ = 0.0f;
    hasValidSpotlight_ = false;
    sidebarScrollTargetY_ = -1.0f;
  }
  void endTour() noexcept {
    isTourActive_ = false;
    escHoldTimer_ = 0.0f;
    hasValidSpotlight_ = false;
    manualBannerOpenProgress_ = 0.0f;
    sidebarScrollTargetY_ = 0.0f;
    sidebarScroller_.targetScrollY = 0.0f;
    sidebarScroller_.smoothScrollY = 0.0f;
    resetSidebarScrollToTop_ = true;
    ignoreEscUntilRelease_ = true;
  }
  [[nodiscard]] bool isTourActive() const noexcept {
    return isTourActive_;
  }
  [[nodiscard]] bool shouldIgnoreEscKey() const noexcept {
    return ignoreEscUntilRelease_;
  }
  void onEscKeyReleased() noexcept {
    ignoreEscUntilRelease_ = false;
  }
  [[nodiscard]] int getTourCurrentStep() const noexcept {
    return tourCurrentStep_;
  }

  void setMissionPanelOpenProgress(float progress) noexcept {
    missionPanelOpenProgress_ = progress;
  }

  [[nodiscard]] bool isCatalogOpen() const noexcept { return isCatalogOpen_; }
  void setCatalogOpen(bool open) noexcept {
    isCatalogOpen_ = open;
    catalogOpenProgress_ = open ? 1.0f : 0.0f;
    isCatalogClosing_ = false;
  }

  [[nodiscard]] bool isCollectedOpen() const noexcept { return isCollectedOpen_; }
  void setCollectedOpen(bool open) noexcept {
    isCollectedOpen_ = open;
    collectedOpenProgress_ = open ? 1.0f : 0.0f;
    isCollectedClosing_ = false;
  }

  void setLegendAnimT(float t) noexcept {
    legendAnimT_ = t;
  }
  [[nodiscard]] float getLegendAnimT() const noexcept {
    return legendAnimT_;
  }

  [[nodiscard]] float getSidebarScrollY() const noexcept {
    return sidebarScroller_.smoothScrollY;
  }
  void setSidebarScrollY(float y) noexcept {
    sidebarScroller_.targetScrollY = y;
    sidebarScroller_.smoothScrollY = y;
    sidebarScrollTargetY_ = y;
  }

  [[nodiscard]] std::vector<bool> getSectionsOpen() const {
    return {sectionsOpen_[0], sectionsOpen_[1], sectionsOpen_[2], sectionsOpen_[3], sectionsOpen_[4]};
  }
  void setSectionsOpen(const std::vector<bool> &states) {
    for (size_t i = 0; i < 5 && i < states.size(); ++i) {
      sectionsOpen_[i] = states[i];
    }
    hasLoadedSectionStates_ = true;
  }

  void setBoldFont(ImFont *font) noexcept { boldFont_ = font; }
  [[nodiscard]] ImFont *getBoldFont() const noexcept { return boldFont_; }

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
      float /*mx*/, float /*my*/, const core::Config & /*config*/,
      float /*windowWidth*/, float /*windowHeight*/) const noexcept {
    return false;
  }

  [[nodiscard]] int getHoveredCollectedItemId() const noexcept {
    if (isTourActive_) return -1;
    return hoveredCollectedItemId_;
  }
  [[nodiscard]] const std::string &getHoveredCatalogItemName() const noexcept {
    static const std::string emptyCatalogItem;
    if (isTourActive_) return emptyCatalogItem;
    return hoveredCatalogItemName_;
  }
  [[nodiscard]] int getHoveredCatalogCategory() const noexcept {
    if (isTourActive_) return -1;
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

  static void applyUIScaleAndFontGlobal(const core::Config &config) {
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
      appliedScale = config.uiScale;
    }
  }

  bool renderSectionHeader(int sectionIdx, const char *label, float uiScale,
                           bool defaultOpen = true) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                        ImVec2(12.0f * uiScale, 8.5f * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f * uiScale);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f); // Clean single-pass border rendering

    // Deep high-contrast dark maroon background to make the blue border pop
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.36f, 0.11f, 0.19f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                          ImVec4(0.50f, 0.15f, 0.25f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                          ImVec4(0.62f, 0.18f, 0.30f, 1.00f));
    // Crisp white-cyan text
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.96f, 0.99f, 1.00f, 1.00f));

    if (isTourActive_) {
      ImGui::SetNextItemOpen(true);
    } else if (hasLoadedSectionStates_) {
      if (sectionIdx >= 0 && sectionIdx < 5) {
        ImGui::SetNextItemOpen(sectionsOpen_[sectionIdx]);
      }
      if (sectionIdx == 4) {
        hasLoadedSectionStates_ = false;
      }
    }

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (defaultOpen && !hasLoadedSectionStates_) {
      flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    bool open = ImGui::CollapsingHeader(label, flags);
    if (sectionIdx >= 0 && sectionIdx < 5) {
      sectionsOpen_[sectionIdx] = open;
    }

    ImVec2 pMin = ImGui::GetItemRectMin();
    ImVec2 pMax = ImGui::GetItemRectMax();
    ImDrawList *drawList = ImGui::GetWindowDrawList();

    bool isHovered = ImGui::IsItemHovered();

    // 1. Crisp electric cyan/blue outer border cleanly enclosing the entire header
    ImU32 borderCol =
        isHovered ? IM_COL32(90, 235, 255, 255) : IM_COL32(0, 210, 255, 240);
    drawList->AddRect(pMin, pMax, borderCol, 4.0f * uiScale, 0, 1.5f * uiScale);

    // 2. High-visibility glowing cyan accent stripe running completely from top to bottom
    ImU32 stripeCol =
        isHovered ? IM_COL32(140, 248, 255, 255) : IM_COL32(0, 225, 255, 255);
    drawList->AddRectFilled(
        ImVec2(pMin.x, pMin.y),
        ImVec2(pMin.x + 4.0f * uiScale, pMax.y),
        stripeCol, 4.0f * uiScale, ImDrawFlags_RoundCornersLeft);

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(3);

    return open;
  }

  void render(core::Config &config, map::Maze &maze,
              simulation::SimulationEngine &engine,
              const std::vector<io::BenchmarkRow> &benchmarkRows,
              float windowHeight, UIState &state) {
    // Handle dynamic UI scaling with crisp text (oversampled TTF)
    applyUIScaleAndFontGlobal(config);
    if (std::abs(config.uiScale - lastUiScale_) > 0.001f) {
      sidebarWidth_ = savedSidebarBaseWidth_ * config.uiScale;
      missionPanelWidth_ = savedMissionPanelBaseWidth_ * config.uiScale;
      lastUiScale_ = config.uiScale;
    }

    float minSidebarW = MIN_SIDEBAR_WIDTH * config.uiScale;
    float maxSidebarWidth = ImGui::GetIO().DisplaySize.x * 0.55f;
    sidebarWidth_ = std::clamp(sidebarWidth_, minSidebarW, maxSidebarWidth);

    float minPanelW = std::round(235.0f * config.uiScale);
    float maxPanelW = std::round(415.0f * config.uiScale);
    missionPanelWidth_ = std::clamp(missionPanelWidth_, minPanelW, maxPanelW);

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
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("Керування та аналітика##Sidebar", nullptr, flags);
    ImGui::SetScrollX(0.0f);

    if (resetSidebarScrollToTop_) {
      resetSidebarScrollToTop_ = false;
      sidebarScroller_.targetScrollY = 0.0f;
      sidebarScroller_.smoothScrollY = 0.0f;
      ImGui::SetScrollY(0.0f);
    } else if (isTourActive_ && sidebarScrollTargetY_ >= 0.0f) {
      sidebarScroller_.targetScrollY = sidebarScrollTargetY_;
    }

    if (state.sliderAutoAdjustTimer > 0.0f) {
      state.sliderAutoAdjustTimer -= ImGui::GetIO().DeltaTime;
    }

    // 1. Display and camera settings
    float b6Start = ImGui::GetCursorScreenPos().y;
    float b6Local = ImGui::GetCursorPosY();
    if (renderSectionHeader(0, LOC("1.  Відображення та огляд", "1.  View & Camera"), config.uiScale,
                            true)) {
      ImGui::Spacing();

      // Zoom Scale mixer slider
      float currentMagnification = 1.0f / std::max(0.01f, state.cameraZoom);
      float userZoom = currentMagnification;
      if (ImGui::SliderFloat(LOC("Масштаб огляду (Zoom)", "View Zoom Scale"), &userZoom, 0.20f, 4.50f,
                             "%.2fx")) {
        userZoom = std::clamp(userZoom, 0.20f, 4.50f);
        state.cameraZoom = 1.0f / userZoom;
        state.targetCameraZoom = state.cameraZoom;
        state.cameraZoomChanged = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "%s", LOC("Мікшер масштабу огляду камери:\n"
                      "Вліво (0.2x - 0.9x): віддалити для ширшого огляду лабіринту.\n"
                      "Вправо (1.1x - 4.5x): наблизити для детального огляду клітинок.\n"
                      "Також можна наближати/віддаляти скролом коліщатка миші.",
                      "Camera zoom mixer:\n"
                      "Left (0.2x - 0.9x): zoom out for wider maze overview.\n"
                      "Right (1.1x - 4.5x): zoom in for detailed cell inspection.\n"
                      "You can also zoom using mouse wheel scroll."));
      }

      // Quick preset buttons for zoom & view centering
      float availZoomWidth = ImGui::GetContentRegionAvail().x;
      float btnSpacing = ImGui::GetStyle().ItemSpacing.x;
      float zoomBtnW =
          std::max(10.0f, (availZoomWidth - 3.0f * btnSpacing) / 4.0f);
      float zoomBtnH =
          std::max(26.0f * config.uiScale, ImGui::GetFrameHeight());

      if (ImGui::Button(LOC("[-] Віддалити", "[-] Zoom Out"), ImVec2(zoomBtnW, zoomBtnH))) {
        float baseZoom = (state.targetCameraZoom > 0.01f)
                             ? state.targetCameraZoom
                             : state.cameraZoom;
        float currentTargetMag = 1.0f / std::max(0.01f, baseZoom);
        float nextMag = std::clamp(currentTargetMag * 0.80f, 0.20f, 4.50f);
        state.targetCameraZoom = 1.0f / nextMag;
        state.requestSmoothZoom = true;
      }
      if (ImGui::IsItemHovered()) {
        showStyledTooltip(
            config.uiScale,
            "%s", LOC("Плавно віддалити огляд камери (зменшити масштаб на 20%).",
                      "Smoothly zoom out camera view (decrease zoom by 20%)."));
      }

      ImGui::SameLine();
      if (ImGui::Button(LOC("[+] Наблизити", "[+] Zoom In"), ImVec2(zoomBtnW, zoomBtnH))) {
        float baseZoom = (state.targetCameraZoom > 0.01f)
                             ? state.targetCameraZoom
                             : state.cameraZoom;
        float currentTargetMag = 1.0f / std::max(0.01f, baseZoom);
        float nextMag = std::clamp(currentTargetMag * 1.25f, 0.20f, 4.50f);
        state.targetCameraZoom = 1.0f / nextMag;
        state.requestSmoothZoom = true;
      }
      if (ImGui::IsItemHovered()) {
        showStyledTooltip(
            config.uiScale,
            "%s", LOC("Плавно наблизити огляд камери (збільшити масштаб на 25%).",
                      "Smoothly zoom in camera view (increase zoom by 25%)."));
      }

      ImGui::SameLine();
      if (ImGui::Button(LOC("Весь лабіринт", "Fit Maze"), ImVec2(zoomBtnW, zoomBtnH))) {
        state.requestFitMazeInView = true;
      }
      if (ImGui::IsItemHovered()) {
        showStyledTooltip(
            config.uiScale,
            "%s", LOC("Огляд усього лабіринту: плавно підібрати масштаб так, щоб "
                      "увесь лабіринт вмістився в екрані.",
                      "Fit whole maze in view: smoothly adjust scale so "
                      "the entire labyrinth fits on screen."));
      }

      ImGui::SameLine();
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
      if (ImGui::Button(LOC("Центрувати", "Center"), ImVec2(zoomBtnW, zoomBtnH))) {
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
                            "%s", LOC("Центрування недоступне під час генерації та "
                                      "трасування маршруту.",
                                      "Centering unavailable during generation and routing."));
        } else {
          showStyledTooltip(
              config.uiScale, "%s",
              config.cameraFollowAgent
                  ? LOC("[C] Вимкнути постійне стеження камери за робітником.",
                        "[C] Disable camera following worker.")
                  : LOC("[C] Центрувати огляд та увімкнути стеження за робітником.",
                        "[C] Center view and follow worker."));
        }
      }

      tourBlockMin_[6] = ImVec2(2.0f, b6Start - 4.0f * config.uiScale);
      tourBlockMax_[6] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 4.0f * config.uiScale);
      tourBlockLocalY_[6] = b6Local;
      tourBlockH_[6] = ImGui::GetCursorPosY() - b6Local;

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      float b7Start = ImGui::GetCursorScreenPos().y;
      float b7Local = ImGui::GetCursorPosY();

      // Display checkbox: Grid
      ImGui::Checkbox(LOC("Показувати сітку клітинок", "Show Grid Overlay"), &config.showGrid);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", LOC("Відображати тонку координатно-тактичну сітку між "
                                    "клітинками лабіринту.",
                                    "Display tactical grid overlay between labyrinth cells."));
      }

      ImGui::Checkbox(LOC("Показувати оптимальний маршрут", "Show Optimal Route"),
                      &config.showOptimalRoute);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", LOC("Увімкнути або вимкнути відображення розрахованої "
                                    "оптимальної траєкторії маршруту.",
                                    "Toggle display of the calculated optimal trajectory path."));
      }

      ImGui::Checkbox(LOC("Показувати пройдені клітинки", "Show Visited Cells"),
                      &config.showWalkedCells);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", LOC("Відображати або приховувати підсвічування "
                                    "пройдених робітником клітинок.",
                                    "Show or hide trail highlights for cells visited by worker."));
      }

      // VSync
      if (ImGui::Checkbox(LOC("Вертикальна синхронізація (VSync)", "Vertical Sync (VSync)"),
                          &config.vsyncEnabled)) {
        state.vsyncChanged = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "%s", LOC("Усуває розриви кадрів ('драбинку') та ривки при плавному русі "
                      "камери.\n"
                      "Синхронізує частоту кадрів із герцовкою вашого монітора.",
                      "Eliminates screen tearing and stutter during smooth camera motion.\n"
                      "Synchronizes frame rate with monitor refresh rate."));
      }

      // UI Scale
      ImGui::SliderFloat(LOC("Масштаб UI", "UI Scale"), &config.uiScale, 0.75f, 1.50f, "%.2fx");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", LOC("Масштабування розміру інтерфейсу, тексту та кнопок "
                                    "(від 0.75x до 1.50x).",
                                    "Scale UI elements, text, and buttons (0.75x to 1.50x)."));
      }

      // Minimap Scale slider
      ImGui::SliderFloat(LOC("Розмір міні-карти", "Minimap Size"), &config.minimapScale, 0.70f,
                         1.30f, "%.2fx");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", LOC("Масштабування розміру тактичної "
                                    "міні-карти (від 0.70x до 1.30x).",
                                    "Scale tactical CRT minimap (0.70x to 1.30x)."));
      }

      tourBlockMin_[7] = ImVec2(2.0f, b7Start - 4.0f * config.uiScale);
      tourBlockMax_[7] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 4.0f * config.uiScale);
      tourBlockLocalY_[7] = b7Local;
      tourBlockH_[7] = ImGui::GetCursorPosY() - b7Local;

      ImGui::Spacing();
      float b8Start = ImGui::GetCursorScreenPos().y;
      float b8Local = ImGui::GetCursorPosY();
      ImGui::TextColored(ImVec4(0.85f, 0.75f, 0.50f, 1.0f),
                         "%s", LOC("Гарячі клавіші та керування:", "Hotkeys & Controls:"));
      ImGui::TextDisabled(
          "%s", LOC("- [Esc] - Відкрити / сховати бічну панель керування",
                    "- [Esc] - Open / close sidebar control panel"));
      ImGui::TextDisabled(
          "%s", LOC("- [F11] - увімкнути / вимкнути повний екран",
                    "- [F11] - Toggle fullscreen mode"));
      ImGui::TextDisabled(
          "%s", LOC("- [W, A, S, D] - ручне керування переміщенням робітника",
                    "- [W, A, S, D] - Manual agent movement controls"));
      ImGui::TextDisabled(
          "%s", LOC("- [B] - Показати / сховати показники місії",
                    "- [B] - Toggle mission status HUD"));
      ImGui::TextDisabled(
          "%s", LOC("- [C] - Центрувати / прив'язати камеру до робітника",
                    "- [C] - Center / lock camera onto worker"));
      ImGui::TextDisabled(
          "%s", LOC("- [Z] - Повернутись до останнього автоматичного кроку",
                    "- [Z] - Rewind manual moves to last route step"));
      ImGui::TextDisabled(
          "%s", LOC("- [M] - Показати / сховати легенду позначок",
                    "- [M] - Toggle map legend"));
      ImGui::TextDisabled(
          "%s", LOC("- [Space] - Крок вперед (затиснути для швидкого руху)",
                    "- [Space] - Step forward (hold for fast movement)"));
      ImGui::TextDisabled(
          "%s", LOC("- [Shift+Space] - Крок назад (перемотування)",
                    "- [Shift+Space] - Step backward (rewind)"));
      ImGui::TextDisabled(
          "%s", LOC("- [Ctrl+Space] - Повернутись на старт / Телепорт на фініш",
                    "- [Ctrl+Space] - Rewind to start / Jump to finish"));
      ImGui::TextDisabled(
          "%s", LOC("- [ЛКМ] - панорамування карти",
                    "- [LMB] - Pan labyrinth camera"));
      ImGui::TextDisabled(
          "%s", LOC("- [ЛКМ на міні-карті] - перехід або панорамування",
                    "- [LMB on minimap] - Jump or pan view"));
      ImGui::TextDisabled(
          "%s", LOC("- [Колесо миші] - масштабування в точку курсора",
                    "- [Mouse wheel] - Zoom view to cursor position"));
      ImGui::TextDisabled(
          "%s", LOC("- [Колесо на міні-карті] - масштабування загальної карти",
                    "- [Mouse wheel on minimap] - Zoom minimap overview"));
      ImGui::Spacing();

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.42f, 0.54f, 0.90f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.58f, 0.74f, 1.00f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.70f, 0.88f, 1.00f));
      ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.95f, 1.00f, 1.00f));
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
      ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.5f * config.uiScale);
      if (boldFont_) ImGui::PushFont(boldFont_);
      if (ImGui::Button(LOC("[?]  ІНСТРУКЦІЯ ЗБОРУ БРУХТУ ДЛЯ ЧАЙНИКІВ",
                            "[?]  SCRAP RUNNERS GUIDE FOR BEGINNERS"),
                        ImVec2(-1.0f, 32.0f * config.uiScale))) {
        startTour();
      }
      if (boldFont_) ImGui::PopFont();
      ImGui::PopStyleVar();
      ImGui::PopStyleColor(5);
      if (ImGui::IsItemHovered()) {
        showStyledTooltip(
            config.uiScale,
            "%s", LOC("Інтерактивний покроковий тур для новачків: показує як грати, призначення тактичного радара, показників місії, легенди та всіх інструментів сайдбару.",
                      "Interactive step-by-step beginner tour: shows how to play, explains tactical radar, mission HUD, legend, and all sidebar controls."));
      }
      ImGui::Spacing();
      tourBlockMin_[8] = ImVec2(2.0f, b8Start - 4.0f * config.uiScale);
      tourBlockMax_[8] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 4.0f * config.uiScale);
      tourBlockLocalY_[8] = b8Local;
      tourBlockH_[8] = ImGui::GetCursorPosY() - b8Local;
    }

    ImGui::Dummy(ImVec2(0.0f, 12.0f * config.uiScale));

    // 2. Maze configuration
    float b9Start = ImGui::GetCursorScreenPos().y;
    float b9Local = ImGui::GetCursorPosY();
    if (renderSectionHeader(1, LOC("2.  Налаштування лабіринту BSP", "2.  BSP Maze Settings"), config.uiScale,
                            true)) {
      ImGui::Spacing();

      // Display the seed of the currently generated labyrinth (clean clickable text,
      // hover glow, click-to-copy, without opening a text input window)
      unsigned int currentSeed =
          maze.getSeed() != 0 ? maze.getSeed() : config.randomSeed;
      char seedStr[32];
      snprintf(seedStr, sizeof(seedStr), "%u", currentSeed);

      static float s_seedCopiedFeedbackTime = 0.0f;
      if (s_seedCopiedFeedbackTime > 0.0f) {
        s_seedCopiedFeedbackTime -= ImGui::GetIO().DeltaTime;
      }
      bool isCopied = (s_seedCopiedFeedbackTime > 0.0f);

      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.70f, 1.0f));
      ImGui::TextUnformatted(LOC("Сід поточного лабіринту:", "Current maze seed:"));
      ImGui::PopStyleColor();

      if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetTooltip("Натисніть для копіювання сіду (%s)", seedStr);
      }
      if (ImGui::IsItemClicked()) {
        ImGui::SetClipboardText(seedStr);
        s_seedCopiedFeedbackTime = 2.0f;
      }

      ImGui::SameLine();

      ImVec4 numColor = isCopied ? ImVec4(0.35f, 0.95f, 0.55f, 1.0f)
                                 : ImVec4(0.88f, 0.88f, 0.95f, 1.0f);
      ImGui::TextColored(numColor, "%s", seedStr);

      bool numHovered = ImGui::IsItemHovered();
      if (numHovered) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (isCopied) {
          ImGui::SetTooltip("Скопійовано в буфер обміну: %s", seedStr);
        } else {
          ImGui::SetTooltip("Натисніть для швидкого копіювання сіду: %s", seedStr);
        }
      }
      if (ImGui::IsItemClicked()) {
        ImGui::SetClipboardText(seedStr);
        s_seedCopiedFeedbackTime = 2.0f;
      }

      // Elegant underline strictly below the digits
      ImVec2 rMin = ImGui::GetItemRectMin();
      ImVec2 rMax = ImGui::GetItemRectMax();
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      ImU32 lineCol = isCopied ? IM_COL32(90, 240, 140, 230)
                               : (numHovered ? IM_COL32(0, 215, 255, 230)
                                             : IM_COL32(140, 150, 175, 140));
      drawList->AddLine(ImVec2(rMin.x, rMax.y), ImVec2(rMax.x, rMax.y), lineCol,
                        numHovered ? 1.5f : 1.0f);

      if (isCopied) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.35f, 0.90f, 0.55f, 1.0f), "[Скопійовано!]");
      }

      ImGui::Spacing();

      ImGui::Checkbox(LOC("Власний сід", "Custom seed"), &state.customSeedEnabled);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Увімкнути генерацію лабіринту за фіксованим числовим сідом.\n"
            "Дозволяє точно відтворити будь-яку згенеровану раніше мапу.");
      }

      if (state.customSeedEnabled) {
        ImGui::PushStyleVar(
            ImGuiStyleVar_FramePadding,
            ImVec2(6.0f * config.uiScale, 5.0f * config.uiScale));
        float btnPasteWidth = ImGui::CalcTextSize(" Вставити з буфера").x +
                              16.0f * config.uiScale;
        float btnUseCurrentWidth = ImGui::CalcTextSize(" Поточний").x +
                                   16.0f * config.uiScale;
        float availW = ImGui::GetContentRegionAvail().x;
        float itemSpacingX = ImGui::GetStyle().ItemSpacing.x;
        float inputWidth = std::max(
            100.0f, availW - btnPasteWidth - btnUseCurrentWidth - itemSpacingX * 2.0f);

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

        ImGui::SameLine();
        if (ImGui::Button(" Поточний", ImVec2(btnUseCurrentWidth, 0))) {
          snprintf(state.customSeedInput, sizeof(state.customSeedInput),
                   "%u", currentSeed);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Підставити сід поточного лабіринту (%u).", currentSeed);
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
      ImGui::SliderInt(LOC("Розмір (N x N)", "Size (N x N)"), &config.mapWidth, 20, 100);
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
        snprintf(btnLabel, sizeof(btnLabel), "%s (%dx%d)",
                 LOC(" ЗГЕНЕРУВАТИ ЛАБІРИНТ", " GENERATE MAZE"),
                 config.mapWidth, config.mapWidth);
        if (state.isTransitionActive) {
          ImGui::BeginDisabled();
          ImGui::Button(LOC(" [ СКАНУВАННЯ КОМПЛЕКСУ... ]", " [ SCANNING FACILITY... ]"),
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
                "%s (%dx%d)",
                LOC("Створити новий процедурний лабіринт розміром",
                    "Create a new procedural maze of size"),
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
    }
    tourBlockMin_[10] = ImVec2(2.0f, b9Start - 2.0f * config.uiScale);
    tourBlockMax_[10] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 2.0f * config.uiScale);
    tourBlockLocalY_[10] = b9Local;
    tourBlockH_[10] = ImGui::GetCursorPosY() - b9Local;

    ImGui::Dummy(ImVec2(0.0f, 12.0f * config.uiScale));

    // 3. Knapsack and algorithm selection
    float b10Start = ImGui::GetCursorScreenPos().y;
    float b10Local = ImGui::GetCursorPosY();
    float b16Start = b10Start;
    float b16Local = b10Local;
    if (renderSectionHeader(2, LOC("3.  Керування експедицією", "3.  Expedition Control"), config.uiScale,
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
      ImGui::Text("%s", LOC("Квота Компанії (грн):", "Company Quota ($):"));
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
      ImGui::Text("%s", LOC("Запас енергії:", "Energy Reserve:"));
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
      if (ImGui::Button(LOC(" [+] Підігнати енергію під поточний маршрут", " [+] Match energy to route"),
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
      ImGui::Text("%s", LOC("Місткість рюкзака (кг):", "Backpack Capacity (kg):"));
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
      if (ImGui::Button(LOC(" [+] Підігнати місткість під поточний маршрут", " [+] Match capacity to route"),
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

      tourBlockMin_[11] = ImVec2(2.0f, b10Start - 2.0f * config.uiScale);
      tourBlockMax_[11] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 4.0f * config.uiScale);
      tourBlockLocalY_[11] = b10Local;
      tourBlockH_[11] = ImGui::GetCursorPosY() - b10Local;

      ImGui::Spacing();
      float b11Start = ImGui::GetCursorScreenPos().y;
      float b11Local = ImGui::GetCursorPosY();
      ImGui::Text("%s", LOC("Алгоритм рюкзака:", "Knapsack Algorithm:"));
      const char *knapsackNames[] = {LOC("0/1 Динамічне програмування (Точний)", "0/1 Dynamic Programming (Exact)"),
                                     LOC("Жадібний за питомою цінністю (V/W)", "Greedy by Value Density (V/W)")};
      bool knapsackAlgoChanged =
          (state.knapsackAlgoIndex != state.appliedKnapsackAlgoIndex);
      beginChangedWidgetBorder(knapsackAlgoChanged);
      ImGui::SetNextItemWidth(-1.0f);
      ImGui::Combo("##KnapsackAlgo", &state.knapsackAlgoIndex, knapsackNames,
                   IM_ARRAYSIZE(knapsackNames));
      endChangedWidgetBorder(knapsackAlgoChanged);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s",
            LOC("0/1 ДП - знаходить точний глобальний оптимум. Жадібний - швидка евристика за співвідношенням ціна/вага.",
                "0/1 DP finds global exact optimum. Greedy is a fast heuristic sorting by value/weight."));
      }

      tourBlockMin_[12] = ImVec2(2.0f, b11Start - 4.0f * config.uiScale);
      tourBlockMax_[12] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 4.0f * config.uiScale);
      tourBlockLocalY_[12] = b11Local;
      tourBlockH_[12] = ImGui::GetCursorPosY() - b11Local;

      ImGui::Spacing();
      float b12Start = ImGui::GetCursorScreenPos().y;
      float b12Local = ImGui::GetCursorPosY();
      ImGui::Text("%s", LOC("Пошук шляху:", "Pathfinding:"));
      const char *pathfinderNames[] = {
          LOC("A* (Октагональна евристика, 8 напрямків)", "A* (Octile Heuristic, 8-way)"),
          LOC("Алгоритм Дейкстри (Найменша енергія)", "Dijkstra (Uniform Cost, Min Energy)"),
          LOC("Пошук у ширину (BFS, найменше клітинок)", "Breadth-First Search (BFS, Min Steps)")};
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

      tourBlockMin_[13] = ImVec2(2.0f, b12Start - 4.0f * config.uiScale);
      tourBlockMax_[13] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 4.0f * config.uiScale);
      tourBlockLocalY_[13] = b12Local;
      tourBlockH_[13] = ImGui::GetCursorPosY() - b12Local;

      ImGui::Spacing();
      float b13Start = ImGui::GetCursorScreenPos().y;
      float b13Local = ImGui::GetCursorPosY();

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
      if (ImGui::Checkbox(LOC("Необмежений рюкзак", "Unlimited Backpack"), &unlimitedBackpack)) {
        config.backpackLimitEnabled = !unlimitedBackpack;
      }
      endChangedWidgetBorder(backpackLimitChanged);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s",
            LOC("Якщо увімкнено - алгоритм розраховується без ліміту місця в рюкзаку.",
                "If enabled, algorithms compute without backpack volume limits."));
      }

      tourBlockMin_[14] = ImVec2(2.0f, b13Start - 4.0f * config.uiScale);
      tourBlockMax_[14] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 4.0f * config.uiScale);
      tourBlockLocalY_[14] = b13Local;
      tourBlockH_[14] = ImGui::GetCursorPosY() - b13Local;

      ImGui::Spacing();
      float b14Start = ImGui::GetCursorScreenPos().y;
      float b14Local = ImGui::GetCursorPosY();

      bool routeModeChanged = (config.routeMode != state.appliedRouteMode);
      int routeModeInt =
          (config.routeMode == core::RouteMode::PointAToPointB) ? 0 : 1;
      ImGui::Text("%s", LOC("Режим евакуації:", "Evacuation Mode:"));
      beginChangedWidgetBorder(routeModeChanged);
      if (ImGui::RadioButton(LOC("Точка A -> B (Вихід)", "Point A -> B (Exit)"), &routeModeInt, 0)) {
        config.routeMode = core::RouteMode::PointAToPointB;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s",
            LOC("Маршрут через відібраний брухт до окремого шлюзу виходу (точка B).",
                "Route through selected scrap items to dedicated exit airlock (Point B)."));
      }

      ImGui::SameLine();
      if (ImGui::RadioButton(LOC("A -> A (База)", "A -> A (Base)"), &routeModeInt, 1)) {
        config.routeMode = core::RouteMode::RoundTrip;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s",
            LOC("Круговий маршрут: старт зі шлюзу (A), збір цінностей та повернення назад до шлюзу (A).",
                "Round-trip route: start at airlock (A), collect scrap, and return back to airlock (A)."));
      }
      endChangedWidgetBorder(routeModeChanged);

      tourBlockMin_[15] = ImVec2(2.0f, b14Start - 4.0f * config.uiScale);
      tourBlockMax_[15] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 4.0f * config.uiScale);
      tourBlockLocalY_[15] = b14Local;
      tourBlockH_[15] = ImGui::GetCursorPosY() - b14Local;

      ImGui::Spacing();
      float b15Start = ImGui::GetCursorScreenPos().y;
      float b15Local = ImGui::GetCursorPosY();

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

        if (ImGui::Button(LOC(" ПЕРЕРАХУВАТИ ОПТИМАЛЬНИЙ ПЛАН", " RECALCULATE OPTIMAL PLAN"),
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

        ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.2f, 1.0f), "%s",
                           LOC("[~] Очікує перерахунку (параметри не застосовано)",
                               "[~] Pending recalculation (parameters not applied)"));
      } else {
        if (ImGui::Button(LOC(" Перерахувати оптимальний план", " Recalculate optimal plan"),
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

      tourBlockMin_[16] = ImVec2(2.0f, b15Start - 4.0f * config.uiScale);
      tourBlockMax_[16] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 4.0f * config.uiScale);
      tourBlockLocalY_[16] = b15Local;
      tourBlockH_[16] = ImGui::GetCursorPosY() - b15Local;

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      b16Start = ImGui::GetCursorScreenPos().y;
      b16Local = ImGui::GetCursorPosY();
      ImGui::TextColored(ImVec4(0.40f, 0.85f, 1.0f, 1.0f), "%s",
                         LOC("JSON Конфігурація та Звітність:", "JSON Configuration & Reporting:"));

      float jsonBtnW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
      float jsonBtnH = std::max(28.0f * config.uiScale, ImGui::GetFrameHeight());

      if (ImGui::Button(LOC(" ↓ Завантажити input_config.json", " ↓ Load input_config.json"), ImVec2(jsonBtnW, jsonBtnH))) {
        state.triggerImportJson = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Завантажити та застосувати параметри карти, робітника\n"
                          "та обраних алгоритмів із файлу 'input_config.json'.");
      }

      ImGui::SameLine();
      if (ImGui::Button(LOC(" ↓ Зберегти конфіг у JSON", " ↓ Save config to JSON"), ImVec2(jsonBtnW, jsonBtnH))) {
        state.triggerSaveConfigJson = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Зберегти поточні налаштування місії, розміри карти, сід,\n"
                          "квоту та алгоритми у файл 'input_config.json'.");
      }

      ImGui::Spacing();
      if (ImGui::Button(LOC(" ↓ Експортувати report_route.json", " ↓ Export report_route.json"), ImVec2(-1, jsonBtnH))) {
        state.triggerExportReportJson = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Експортувати повний структурований звіт оптимізації рюкзака,\n"
                          "маршруту TSP, координат траєкторії та бенчмарків у 'report_route.json'.");
      }

      ImGui::Spacing();
    }
    tourBlockMin_[17] = ImVec2(2.0f, b16Start - 4.0f * config.uiScale);
    tourBlockMax_[17] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 2.0f * config.uiScale);
    tourBlockLocalY_[17] = b16Local;
    tourBlockH_[17] = ImGui::GetCursorPosY() - b16Local;

    ImGui::Dummy(ImVec2(0.0f, 12.0f * config.uiScale));

    // 4. Simulation playback controls
    float b17Start = ImGui::GetCursorScreenPos().y;
    float b17Local = ImGui::GetCursorPosY();
    if (renderSectionHeader(3, LOC("4.  Керування симуляцією", "4.  Simulation Control"), config.uiScale, true)) {
      ImGui::Spacing();
      const auto &agent = engine.getAgent();

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

      // Centered step counter above player controls
      char stepBuf[64];
      std::snprintf(stepBuf, sizeof(stepBuf), "%s %zu / %zu",
                    LOC("Крок симуляції:", "Simulation step:"),
                    engine.getCurrentStepIndex(), engine.getTotalSteps());
      float stepTextW = ImGui::CalcTextSize(stepBuf).x;
      float availW = ImGui::GetContentRegionAvail().x;
      if (availW > stepTextW) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availW - stepTextW) * 0.5f);
      }
      ImGui::TextColored(ImVec4(0.85f, 0.92f, 1.0f, 1.0f), "%s", stepBuf);
      ImGui::Spacing();

      float btnItemSpacingX = ImGui::GetStyle().ItemSpacing.x;
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(btnItemSpacingX, btnItemSpacingX));

      float btnW = (ImGui::GetContentRegionAvail().x - 2.0f * btnItemSpacingX) / 3.0f;
      float btnH = std::max(28.0f * config.uiScale, ImGui::GetFrameHeight());

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

      if (!canControl) {
        ImGui::EndDisabled();
      }

      // Two quick navigation buttons immediately after player controls
      float jumpBtnW = (ImGui::GetContentRegionAvail().x - btnItemSpacingX) * 0.5f;
      float jumpBtnH = btnH;

      if (!canStepBack) {
        ImGui::BeginDisabled(true);
      }
      if (ImGui::Button(" [ |< ]", ImVec2(jumpBtnW, jumpBtnH))) {
        if (!engine.isPaused()) {
          engine.setPaused(true);
        }
        engine.resetToStart();
        if (config.cameraFollowAgent) {
          state.requestCenterCameraOnAgent = true;
        }
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        showStyledTooltip(
            config.uiScale, "%s",
            canStepBack
                ? "Повернутися на старт [Ctrl+Space]: скинути прогрес симуляції на початковий шлюз та відновити енергію."
                : disabledReason);
      }
      if (!canStepBack) {
        ImGui::EndDisabled();
      }

      ImGui::SameLine();
      if (!canControl) {
        ImGui::BeginDisabled(true);
      }
      if (ImGui::Button(" [ >| ]", ImVec2(jumpBtnW, jumpBtnH))) {
        engine.instantFinish();
        state.requestFitMazeInView = true;
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        showStyledTooltip(
            config.uiScale, "%s",
            canControl
                ? "Пройти одразу на фініш [Ctrl+Space]: миттєво завершити весь маршрут та показати весь лабіринт."
                : disabledReason);
      }
      if (!canControl) {
        ImGui::EndDisabled();
      }

      if (!canControl) {
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
      if (ImGui::SliderFloat(LOC("Швидкість ходу", "Playback Speed"), &speed, 0.5f, 4.0f, "%.1fx")) {
        engine.setSpeedMultiplier(speed);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Швидкість автоматичного крокування симуляції (від "
                          "0.5x до 4.0x). Базовий крок: 0.75 с.");
      }

      tourBlockMin_[18] = ImVec2(2.0f, b17Start - 2.0f * config.uiScale);
      tourBlockMax_[18] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 2.0f * config.uiScale);
      tourBlockLocalY_[18] = b17Local;
      tourBlockH_[18] = ImGui::GetCursorPosY() - b17Local;

      ImGui::PopStyleVar();
      ImGui::Spacing();
    }

    ImGui::Dummy(ImVec2(0.0f, 12.0f * config.uiScale));

    // 5. Benchmark analytics
    float b19Start = ImGui::GetCursorScreenPos().y;
    float b19Local = ImGui::GetCursorPosY();
    if (renderSectionHeader(4, LOC("5.  Порівняльний аналіз", "5.  Comparative Analysis"), config.uiScale, true)) {
      ImGui::Spacing();
      if (ImGui::Button(LOC(" Запустити повний аналіз (усі 6)", " Run Complete Benchmark (All 6)"),
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
        ImGui::PushStyleColor(ImGuiCol_TableBorderStrong,
                              ImVec4(0.00f, 0.85f, 1.00f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight,
                              ImVec4(0.00f, 0.82f, 1.00f, 0.55f));
        ImGui::PushStyleColor(ImGuiCol_Border,
                              ImVec4(0.00f, 0.85f, 1.00f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg,
                              ImVec4(0.02f, 0.18f, 0.24f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt,
                              ImVec4(0.00f, 0.35f, 0.45f, 0.12f));

        if (ImGui::BeginTable("ТаблицяМетрик", 4,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_SizingStretchProp |
                                  ImGuiTableFlags_Resizable)) {
          ImGui::TableSetupColumn(LOC("Алгоритм", "Algorithm"),
                                  ImGuiTableColumnFlags_WidthStretch, 0.44f);
          ImGui::TableSetupColumn(LOC("Час (мс)", "Time (ms)"),
                                  ImGuiTableColumnFlags_WidthStretch, 0.13f);
          ImGui::TableSetupColumn(LOC("Кроки/Енергія", "Steps/Energy"),
                                  ImGuiTableColumnFlags_WidthStretch, 0.29f);
          ImGui::TableSetupColumn(LOC("Оптимум %", "Optimum %"),
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
        ImGui::PopStyleColor(5);
      }

      const auto &plan = engine.getPlan();
      if (plan.routeValid) {
        ImGui::Spacing();
        ImGui::Text("Оптимізація TSP (2-Opt):");
        ImGui::Text(" - До 2-Opt: %d од. енергії", plan.energyBefore2Opt);
        ImGui::Text(" - Після 2-Opt: %d од. енергії", plan.totalEnergyCost);
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f),
                           " - Скорочення маршруту: +%.2f %%",
                           plan.twoOptGainPercent);
      }

      if (ImGui::Button(LOC(" Експорт таблиці у CSV файл", " Export Table to CSV File"),
                        ImVec2(-1, std::max(30.0f * config.uiScale,
                                            ImGui::GetFrameHeight())))) {
        state.triggerExportCsv = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Експортувати результати вимірювання продуктивності "
                          "алгоритмів у файл benchmark_results.csv.");
      }

      ImGui::Spacing();
      if (ImGui::Button(LOC(" Експорт повного звіту у JSON файл", " Export Full Report to JSON File"),
                        ImVec2(-1, std::max(30.0f * config.uiScale,
                                            ImGui::GetFrameHeight())))) {
        state.triggerExportReportJson = true;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Експортувати результати вимірювання, параметри карти та "
                          "траєкторію робітника у файл report_route.json.");
      }
      ImGui::Spacing();
    }
    tourBlockMin_[19] = ImVec2(2.0f, b19Start - 2.0f * config.uiScale);
    tourBlockMax_[19] = ImVec2(sidebarWidth_ - 6.0f, ImGui::GetCursorScreenPos().y + 2.0f * config.uiScale);
    tourBlockLocalY_[19] = b19Local;
    tourBlockH_[19] = ImGui::GetCursorPosY() - b19Local;

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
                            "[Клік / Esc]: Відкрити бічну панель керування");
        } else {
          ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
          showStyledTooltip(
              uiScale,
              "[Клік / Esc]: Сховати бічну панель\n[Потягніть мишею]: Змінити "
              "ширину меню");
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
      float centerY = handleY + (handleH * 0.5f);

      ImGui::SetNextWindowPos(ImVec2(hintX, centerY), ImGuiCond_Always,
                              ImVec2(0.0f, 0.5f));
      ImGui::SetNextWindowBgAlpha(0.94f * animT);
      ImGui::SetNextWindowSizeConstraints(
          ImVec2(410.0f * uiScale, 0.0f),
          ImVec2(600.0f * uiScale, 900.0f));

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
        ImGui::TextDisabled("- Відображення та огляд");
        ImGui::TextDisabled("- Налаштування лабіринту BSP");
        ImGui::TextDisabled("- Керування експедицією");
        ImGui::TextDisabled("- Керування симуляцією");
        ImGui::TextDisabled("- Порівняльний аналіз");
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.42f, 0.54f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.58f, 0.74f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.70f, 0.88f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.95f, 1.00f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.5f * uiScale);
        if (boldFont_) ImGui::PushFont(boldFont_);
        if (ImGui::Button(LOC("[?]  ІНСТРУКЦІЯ ЗБОРУ БРУХТУ ДЛЯ ЧАЙНИКІВ",
                              "[?]  SCRAP RUNNERS GUIDE FOR BEGINNERS"),
                          ImVec2(-1.0f, 30.0f * uiScale))) {
          startTour();
          startupHintDismissed_ = true;
        }
        if (boldFont_) ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(5);
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(
              uiScale,
              "%s", LOC("Інтерактивний покроковий тур для новачків: показує як грати, призначення тактичного радара, показників місії, легенди та всіх інструментів сайдбару.",
                        "Interactive step-by-step beginner tour: shows how to play, explains tactical radar, mission HUD, legend, and all sidebar controls."));
        }
        ImGui::Spacing();

        if (ImGui::Button(LOC("Зрозуміло", "Got it"), ImVec2(-1.0f, 26.0f * uiScale))) {
          startupHintDismissed_ = true;
        }

        // Pointer triangle pointing left directly to the handle
        ImDrawList *hintDrawList = ImGui::GetWindowDrawList();
        ImVec2 pTip(hintX - 7.0f, centerY);
        ImVec2 pTop(hintX + 1.0f, centerY - 8.0f * uiScale);
        ImVec2 pBot(hintX + 1.0f, centerY + 8.0f * uiScale);
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
    if (isTourActive_)
      return;

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
                           "%s: %s", LOC("Дрібний брухт", "Small Scrap"), it.name.c_str());
      } else if (it.category == core::ItemCategory::MediumScrap) {
        ImGui::TextColored(ImVec4(1.00f, 1.00f, 0.00f, 1.0f),
                           "%s: %s", LOC("Побутовий брухт", "Household Scrap"), it.name.c_str());
      } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
                           "%s: %s", LOC("Дорогоцінний брухт", "Precious Scrap"), it.name.c_str());
      }
      ImGui::Separator();
      if (isOpportunistic) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f),
                           "%s", LOC("Збір: Попутний брухт (лежав на шляху)",
                                     "Pickup: Opportunistic scrap (on route)"));
      }
      ImGui::Text("%s: %s", LOC("Категорія", "Category"),
                  core::itemCategoryToString(it.category).c_str());
      ImGui::Text("%s: %d %s", LOC("Вага", "Weight"), it.weight, LOC("кг", "kg"));
      ImGui::Text("%s: %d %s", LOC("Цінність", "Value"), it.value, LOC("грн", "UAH"));
      ImGui::Text("%s (V/W): %.2f %s", LOC("Питома цінність", "Value Density"),
                  it.getValueDensity(), LOC("грн/кг", "UAH/kg"));
      ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), it.position.x, it.position.y);
      ImGui::Separator();
      ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.55f, 1.0f),
                         "%s", LOC("Вже підібрано робітником у рюкзак", "Already collected into backpack"));

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
    if (isTourActive_ && (tourCurrentStep_ == 0 || tourCurrentStep_ == 1 || tourCurrentStep_ >= 5)) {
      return;
    }
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

    float mmFrameW = Minimap::getFrameWidth(config.minimapScale);
    float legendW = std::max(340.0f * config.uiScale, mmFrameW);
    float padX = 10.0f * config.uiScale;
    float padY = 8.0f * config.uiScale;
    float commonBtnH =
        std::max(34.0f * config.uiScale,
                 ImGui::GetFrameHeight() + 6.0f); // Taller button up-and-down

    float hudEase = getHudIntroEase();
    float btnX = windowWidth - legendW - Minimap::PADDING;
    float targetBtnY = 0.0f;
    float curBtnY = -commonBtnH + commonBtnH * hudEase;

    ImGui::SetNextWindowPos(ImVec2(btnX, curBtnY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(legendW, commonBtnH), ImGuiCond_Always);

    ImGuiWindowFlags btnFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (ImGui::Begin("##MinimapLegendUnifiedToggle", nullptr, btnFlags)) {
      const char *btnLabel =
          config.showLegend ? LOC("[M] Сховати легенду", "[M] Hide Legend")
                            : LOC("[M] Показати легенду", "[M] Show Legend");
      if (ImGui::Button(btnLabel, ImVec2(legendW, commonBtnH))) {
        config.showLegend = !config.showLegend;
      }
      if (ImGui::IsItemHovered()) {
        showStyledTooltip(config.uiScale, "%s",
                          config.showLegend
                              ? LOC("Сховати детальну легенду позначок мапи.", "Hide detailed map legend.")
                              : LOC("Показати детальну легенду позначок мапи.", "Show detailed map legend."));
      }
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    float btnY = targetBtnY;

    // 2. Details window gracefully sliding & fading vertically under the toggle button
    if (windowAlpha > 0.001f) {
      float frameH = ui::Minimap::getFrameHeight(config.minimapScale);
      float missionTopY =
          (lastMissionPanelTopY_ > 0.0f)
              ? lastMissionPanelTopY_
              : (windowHeight - frameH - Minimap::PADDING - 34.0f * config.uiScale);
      float targetLegendY = btnY + commonBtnH + 6.0f;
      float maxLegendH = std::max(100.0f, missionTopY - 6.0f - targetLegendY);

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
                          ImGui::GetFrameHeightWithSpacing() + padY * 2.0f + 4.0f * config.uiScale;

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

      bool legendInteractable = (easeT >= 0.995f && config.showLegend);
      float curWindowH = std::max(1.0f, std::round(legendH * easeT));
      float legendX = btnX;
      // Anchor top edge strictly below toggle button so it never overlaps, dims, or steals hover
      ImGui::SetNextWindowPos(ImVec2(legendX, targetLegendY), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(legendW, curWindowH), ImGuiCond_Always);

      ImGuiWindowFlags flags =
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_NoScrollWithMouse |
          ImGuiWindowFlags_NoBringToFrontOnFocus |
          ImGuiWindowFlags_NoFocusOnAppearing;
      if (!legendInteractable) {
        flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar;
      }

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
      ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.0f, 1.0f));

      if (ImGui::Begin("MinimapLegendWindow", nullptr, flags)) {
        ImGui::GetWindowDrawList()->PushClipRect(
            ImVec2(btnX - 10.0f, targetLegendY - 2.0f),
            ImVec2(btnX + legendW + 10.0f,
                   std::min(windowHeight, targetLegendY + curWindowH + 2.0f)),
            true);
        float revealShiftY = -(1.0f - easeT) * 24.0f * config.uiScale;
        if (!legendInteractable) {
          ImGui::SetCursorPosY(padY + revealShiftY);
        }
        ImGui::SetScrollX(0.0f);
        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f),
                           "%s", LOC("Легенда позначок:", "Map Legend:"));
        ImGui::Separator();

        auto colA = [windowAlpha](uint8_t r, uint8_t g, uint8_t b,
                                  uint8_t baseA = 255) -> ImU32 {
          uint8_t a = static_cast<uint8_t>(
              std::clamp(baseA * windowAlpha, 0.0f, 255.0f));
          return IM_COL32(r, g, b, a);
        };

        auto drawLegendItem = [this, scale = config.uiScale, &colA, legendInteractable](
                                  const char *label, const char *tooltip,
                                  auto iconFunc, int categoryId = -1) {
          ImVec2 p = ImGui::GetCursorScreenPos();
          float iconSize = 18.0f;
          float lineHeight = ImGui::GetTextLineHeight();
          float rowH = std::max(iconSize, lineHeight);
          float availW = ImGui::GetContentRegionAvail().x;
          float padX = 6.0f * scale;
          float padY = 3.0f * scale;

          ImVec2 rowMin(p.x - padX, p.y - padY);
          ImVec2 rowMax(p.x + availW + padX, p.y + rowH + padY);

          ImDrawList *drawList = ImGui::GetWindowDrawList();

          bool isHovered = legendInteractable && !isTourActive_ &&
                           ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
                           ImGui::IsMouseHoveringRect(rowMin, rowMax);

          if (isHovered) {
            drawList->AddRectFilled(rowMin, rowMax, colA(255, 255, 255, 22), 4.0f * scale);
            drawList->AddRect(rowMin, rowMax, colA(0, 220, 255, 75), 4.0f * scale, 0, 1.0f);
            showStyledTooltip(scale, "%s", tooltip);
            if (categoryId >= 0) {
              hoveredCatalogCategory_ = categoryId;
            }
          }

          iconFunc(drawList, p, iconSize);

          ImGui::Dummy(ImVec2(iconSize, iconSize));
          ImGui::SameLine(0, 8.0f);
          ImGui::TextUnformatted(label);
          ImGui::Spacing();
        };

        drawLegendItem(
            LOC("Робітник", "Worker"),
            LOC("Автономний робітник у захисному костюмі Lethal Company",
                "Autonomous worker in Lethal Company hazmat suit"),
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
                dl->AddCircle(center, 5.0f, colA(255, 255, 240, 240), 0, 1.5f);
              }
            },
            10);

        drawLegendItem(
            LOC("Шлюз (Старт)", "Airlock (Start)"),
            LOC("Початкова точка, звідки робітник починає місію (люк шлюзу з маяком)",
                "Starting point where worker begins mission (airlock hatch with beacon)"),
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
            },
            11);

        // 3. Exit (Evacuation) - actual maze cell appearance
        if (config.routeMode == core::RouteMode::RoundTrip) {
          drawLegendItem(LOC("Вихід (Недоступний)", "Exit (Unavailable)"),
                         LOC("У режимі 'A -> A (База)' вихід заблоковано. Евакуація лише через вхід.",
                             "In 'A -> A (Base)' mode the secondary exit is locked. Evacuation only via start."),
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
                         },
                         12);
        } else {
          drawLegendItem(
              LOC("Вихід (Евакуація)", "Exit (Evacuation)"),
              LOC("Шлюз евакуації, куди робітник прямує після збору брухту (смарагдова платформа)",
                  "Evacuation hatch where worker heads after collecting scrap (emerald pad)"),
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
              },
              12);
        }

        // 4. Mud / Swamp / Water hazard tile (styled after labyrinth mud hazards)
        drawLegendItem(
            LOC("Багнюка / Болото / Вода", "Mud / Swamp / Water"),
            LOC("Важкопрохідна ділянка: витрата енергії +25 прямо, +35 по діагоналі",
                "Difficult terrain: energy cost +25 orthogonal, +35 diagonal"),
            [&colA](ImDrawList *dl, ImVec2 p, float s) {
              dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s),
                                colA(58, 44, 28, 255), 2.0f);
              dl->AddRectFilled(ImVec2(p.x + 2, p.y + 2),
                                ImVec2(p.x + s - 2, p.y + s - 2),
                                colA(72, 54, 32, 255), 2.0f);
              dl->AddRect(ImVec2(p.x + 3.5f, p.y + 3.5f),
                          ImVec2(p.x + s - 3.5f, p.y + s - 3.5f),
                          colA(118, 92, 54, 220), 1.0f);
              // Mud square sludge bubbles matching labyrinth style
              dl->AddRectFilled(ImVec2(p.x + s * 0.30f, p.y + s * 0.35f),
                                ImVec2(p.x + s * 0.30f + 3.5f, p.y + s * 0.35f + 3.5f),
                                colA(135, 105, 62, 255));
              dl->AddRect(ImVec2(p.x + s * 0.30f, p.y + s * 0.35f),
                          ImVec2(p.x + s * 0.30f + 3.5f, p.y + s * 0.35f + 3.5f),
                          colA(175, 142, 90, 240), 0.0f, 0, 0.8f);
              dl->AddRectFilled(ImVec2(p.x + s * 0.65f, p.y + s * 0.60f),
                                ImVec2(p.x + s * 0.65f + 2.5f, p.y + s * 0.60f + 2.5f),
                                colA(135, 105, 62, 255));
              // Corner safety rivets
              dl->AddRectFilled(ImVec2(p.x + 1, p.y + 1),
                                ImVec2(p.x + 3, p.y + 3),
                                colA(155, 125, 75, 220));
              dl->AddRectFilled(ImVec2(p.x + s - 3, p.y + 1),
                                ImVec2(p.x + s - 1, p.y + 3),
                                colA(155, 125, 75, 220));
              dl->AddRectFilled(ImVec2(p.x + 1, p.y + s - 3),
                                ImVec2(p.x + 3, p.y + s - 1),
                                colA(155, 125, 75, 220));
              dl->AddRectFilled(ImVec2(p.x + s - 3, p.y + s - 3),
                                ImVec2(p.x + s - 1, p.y + s - 1),
                                colA(155, 125, 75, 220));
            },
            13);

        // 5. Light, medium, and heavy scrap items
        drawLegendItem(
            LOC("Дрібний брухт (1-3 кг)", "Small Scrap (1-3 kg)"),
            LOC("Одноручний брухт Lethal Company (каченя, рибка, пульт, лупа)",
                "One-handed Lethal Company scrap (duck, fish, remote, magnifying glass)"),
            [&colA](ImDrawList *dl, ImVec2 p, float s) {
              ImVec2 center(p.x + s * 0.5f, p.y + s * 0.5f);
              dl->AddCircleFilled(center, 5.0f, colA(50, 105, 255, 255));
              dl->AddCircle(center, 5.0f, colA(255, 255, 255, 240), 0, 1.8f);
            },
            static_cast<int>(core::ItemCategory::LightScrap));

        drawLegendItem(
            LOC("Побутовий брухт (4-8 кг)", "Household Scrap (4-8 kg)"),
            LOC("Середній побутовий брухт (годинник, бумбокс, фен, картина)",
                "Medium household scrap (clock, boombox, hairdryer, painting)"),
            [&colA](ImDrawList *dl, ImVec2 p, float s) {
              ImVec2 center(p.x + s * 0.5f, p.y + s * 0.5f);
              dl->AddCircleFilled(center, 6.0f, colA(255, 255, 0, 255));
              dl->AddCircle(center, 6.0f, colA(255, 255, 255, 240), 0, 1.8f);
            },
            static_cast<int>(core::ItemCategory::MediumScrap));

        drawLegendItem(
            LOC("Дорогоцінний брухт (10-18 кг)", "Precious Scrap (10-18 kg)"),
            LOC("Дворучні масивні механізми (апарат живлення, двигун, золото, реактор)",
                "Two-handed heavy equipment (power apparatus, engine, gold bar, reactor)"),
            [&colA](ImDrawList *dl, ImVec2 p, float s) {
              ImVec2 center(p.x + s * 0.5f, p.y + s * 0.5f);
              dl->AddCircleFilled(center, 7.0f, colA(255, 0, 255, 255));
              dl->AddCircle(center, 7.0f, colA(255, 255, 255, 240), 0, 1.8f);
            },
            static_cast<int>(core::ItemCategory::HeavyScrap));

        // 6. Walked cell (Trail) - clean maze floor square without inner dots
        drawLegendItem(LOC("Пройдена клітинка (Слід)", "Visited Cell (Trail)"),
                       LOC("Пройдений шлях робітника: темно-блакитна підлога з неоновим контуром",
                           "Worker trail: dark-cyan floor tile with neon outline"),
                       [&colA](ImDrawList *dl, ImVec2 p, float s) {
                         dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s),
                                           colA(10, 48, 65, 235), 2.0f);
                         dl->AddRect(p, ImVec2(p.x + s, p.y + s),
                                     colA(0, 230, 245, 240), 2.0f, 0, 1.5f);
                       },
                       14);

        // 7. Route (TSP Trajectory) - laser line without waypoint node circle
        drawLegendItem(LOC("Маршрут (Траєкторія TSP)", "Route (TSP Trajectory)"),
                       LOC("Розрахована оптимальна лінія пересування робітника",
                           "Calculated optimal movement path of the worker"),
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
                       },
                       15);

        ImGui::Spacing();
        float btnLocalY =
            ImGui::GetCursorPosY() - ImGui::GetStyle().WindowPadding.y;
        catalogBtnLocalY_ = btnLocalY;

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

        char catalogBtnText[128];
        std::snprintf(catalogBtnText, sizeof(catalogBtnText),
                      "%s (%zu %s)###CatalogBtn",
                      LOC("Каталог брухту", "Scrap Catalog"),
                      core::getLethalScrapTemplates().size(),
                      LOC("найменувань", "items"));

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
        if (legendInteractable && ImGui::IsItemHovered()) {
          showStyledTooltip(
              config.uiScale, "%s",
              (isCatalogOpen_ && !isCatalogClosing_)
                  ? LOC("Згорнути каталог найменувань брухту та повернутися до легенди.",
                        "Collapse scrap items catalog and return to legend.")
                  : LOC("Розгорнути повний каталог з 60 найменувань брухту Lethal Company.",
                        "Expand full catalog of 60 Lethal Company scrap items."));
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
            ImGui::TableSetupColumn(LOC("  Найменування", "  Item Name"),
                                    ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(LOC("Вага / Ціна", "Weight / Price"),
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
                      ? LOC("Дрібний брухт (1-3 кг)", "Small Scrap (1-3 kg)")
                  : (it.category == core::ItemCategory::MediumScrap)
                      ? LOC("Побутовий брухт (4-8 кг)", "Household Scrap (4-8 kg)")
                      : LOC("Дорогоцінний брухт (10-18 кг)", "Precious Scrap (10-18 kg)");
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
              bool isCatHovered = legendInteractable && !isTourActive_ && ImGui::IsItemHovered();
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
                if (!isTourActive_ && lastHoveredCatalogCategory_ != hoveredCatalogCategory_) {
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

            if (legendInteractable && !isTourActive_ && ImGui::IsItemHovered()) {
              isRowHovered = true;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("%d %s, %d %s", it.baseWeight, LOC("кг", "kg"), it.baseValue, LOC("грн", "UAH"));
            if (legendInteractable && !isTourActive_ && ImGui::IsItemHovered()) {
              isRowHovered = true;
            }

            if (isRowHovered && !isTourActive_) {
              hoveredCatalogCategory_ = -1;
              hoveredCatalogItemName_ = it.name;
              if (isPresentInMaze) {
                showStyledTooltip(
                    config.uiScale,
                    "%zu. %s [%s]\n%s: %d %s | %s: %d %s",
                    i + 1, it.name.c_str(),
                    LOC("В поточному комплексі", "In current complex"),
                    LOC("Вага", "Weight"), it.baseWeight, LOC("кг", "kg"),
                    LOC("Вартість", "Value"), it.baseValue, LOC("грн", "UAH"));
              } else {
                showStyledTooltip(config.uiScale,
                                  "%zu. %s\n%s: %d %s | %s: %d %s",
                                  i + 1, it.name.c_str(),
                                  LOC("Вага", "Weight"), it.baseWeight, LOC("кг", "kg"),
                                  LOC("Вартість", "Value"), it.baseValue, LOC("грн", "UAH"));
              }
              if (!isTourActive_ && lastHoveredCatalogItemName_ != it.name) {
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
          if (ImGui::Button(LOC("Вгору до легенди###ScrollUpLegendBtn", "Up to Legend###ScrollUpLegendBtn"),
                            ImVec2(-1.0f, 26.0f * config.uiScale))) {
            isCatalogClosing_ = true;
            catalogScrollOnCloseStart_ = ImGui::GetScrollY();
          }
          if (legendInteractable && ImGui::IsItemHovered()) {
            showStyledTooltip(config.uiScale,
                              "%s", LOC("Повернутися до початкового списку легенди.",
                                        "Return to the initial legend list."));
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
        if (hasExtraScrap) {
          std::snprintf(collectedBtnText, sizeof(collectedBtnText),
                        "%s: %d / %d %s (%zu/%zu %s [+%zu])###CollectedBtn",
                        LOC("Квота", "Quota"),
                        curCollectedVal, quota, LOC("грн", "UAH"),
                        collectedCount, targetItemsCount, LOC("од.", "items"),
                        extraCount);
        } else {
          std::snprintf(collectedBtnText, sizeof(collectedBtnText),
                        "%s: %d / %d %s (%zu/%zu %s)###CollectedBtn",
                        LOC("Квота", "Quota"),
                        curCollectedVal, quota, LOC("грн", "UAH"),
                        collectedCount, targetItemsCount, LOC("од.", "items"));
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

        collectedBtnLocalY_ = ImGui::GetCursorPosY() - ImGui::GetStyle().WindowPadding.y;

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
        if (legendInteractable && ImGui::IsItemHovered()) {
          if (isQuotaMet) {
            bool isExceeded = (curCollectedVal >= quota + 50);
            if (isExceeded) {
              showStyledTooltip(
                  config.uiScale,
                  "Квоту Компанії ПЕРЕВИКОНАНО!\n"
                  "Зібрано %d грн із необхідних %d грн (+%d грн перевиконання).",
                  curCollectedVal, quota, curCollectedVal - quota);
            } else {
              showStyledTooltip(config.uiScale,
                                "Мінімальну квоту Компанії успішно виконано!\n"
                                "Зібрано %d грн із необхідних %d грн.",
                                curCollectedVal, quota);
            }
          } else if (hasExtraScrap) {
            showStyledTooltip(
                config.uiScale,
                "На шляху робітника лежав брухт, який помістився в рюкзак,\n"
                "тому робітник зібрав на %zu од. більше, ніж планувалося!",
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

        if (isCollectedOpen_ || collectedOpenProgress_ > 0.001f) {
          ImGui::Spacing();
          float collectedContentStartY = ImGui::GetCursorPosY();

          // Unified Quota status and opportunistic scrap card (1 title + 1 unified description)
          if (isQuotaMet) {
            int surplusVal = curCollectedVal - quota;
            bool isExceeded = (curCollectedVal >= quota + 50);
            ImGui::PushStyleColor(ImGuiCol_Border,
                                  ImVec4(0.25f, 0.95f, 0.50f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                                  ImVec4(0.10f, 0.25f, 0.16f, 0.92f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleVar(
                ImGuiStyleVar_WindowPadding,
                ImVec2(8.0f * config.uiScale, 6.0f * config.uiScale));
            ImGui::BeginChild("##UnifiedQuotaSuccessNotice", ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_Border |
                                  ImGuiChildFlags_AutoResizeY);
            if (hasExtraScrap) {
              ImGui::TextColored(
                  ImVec4(0.40f, 1.0f, 0.60f, 1.0f),
                  "[OK] КВОТУ ПЕРЕВИКОНАНО (ПОНАДПЛАНОВИЙ ЗБІР +%zu ОД.)!",
                  extraCount);
              ImGui::TextWrapped(
                  "Зібрано %d грн із необхідних %d грн (+%d грн перевиконання). "
                  "На шляху знайдено додатковий брухт, який успішно вмістився "
                  "в рюкзак. Евакуація гарантує 100%% успіх місії!",
                  curCollectedVal, quota, surplusVal);
            } else {
              ImGui::TextColored(
                  ImVec4(0.40f, 1.0f, 0.60f, 1.0f), "%s",
                  isExceeded ? "[OK] КВОТУ ПЕРЕВИКОНАНО!"
                             : "[OK] МІНІМАЛЬНУ КВОТУ ВИКОНАНО!");
              ImGui::TextWrapped(
                  "Зібрано %d грн із необхідних %d грн (+%d грн перевиконання). "
                  "Евакуація гарантує 100%% успіх місії!",
                  curCollectedVal, quota, surplusVal);
            }
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
            if (hasExtraScrap) {
              ImGui::TextColored(
                  ImVec4(1.0f, 0.85f, 0.30f, 1.0f),
                  "[*] ПРОГРЕС КВОТИ ТА ПОНАДПЛАНОВИЙ ЗБІР (+%zu ОД.):",
                  extraCount);
              ImGui::TextWrapped(
                  "Зібрано %d із %d грн (до плану залишилось %d грн). На шляху "
                  "підібрано попутний брухт, який помістився у вільне місце рюкзака.",
                  curCollectedVal, quota, remainingVal);
            } else {
              ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.30f, 1.0f),
                                 "[*] ПРОГРЕС КВОТИ: %d / %d грн",
                                 curCollectedVal, quota);
              ImGui::TextWrapped("До виконання вимоги Компанії залишилося "
                                 "підібрати брухту на %d грн.",
                                 remainingVal);
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::Spacing();
          }

          if (collectedIds.empty()) {
            ImGui::TextDisabled("  Робітник ще не зібрав жодного брухту.");
          } else {
            ImGui::PushStyleVar(
                ImGuiStyleVar_CellPadding,
                ImVec2(8.0f * config.uiScale, 3.5f * config.uiScale));

            if (ImGui::BeginTable("CollectedScrapTable", 3,
                                  ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_PadOuterX |
                                      ImGuiTableFlags_SizingStretchProp)) {
              ImGui::TableSetupColumn("Назва",
                                      ImGuiTableColumnFlags_WidthStretch);
              ImGui::TableSetupColumn("Вага",
                                      ImGuiTableColumnFlags_WidthFixed,
                                      48.0f * config.uiScale);
              ImGui::TableSetupColumn("Ціна",
                                      ImGuiTableColumnFlags_WidthFixed,
                                      56.0f * config.uiScale);

              ImGui::PushStyleColor(ImGuiCol_TableHeaderBg,
                                    ImVec4(0.12f, 0.20f, 0.16f, 0.95f));
              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              const char *headers[3] = {"Назва", "Вага", "Ціна"};
              for (int colIdx = 0; colIdx < 3; ++colIdx) {
                ImGui::TableSetColumnIndex(colIdx);
                char hdrId[16];
                std::snprintf(hdrId, sizeof(hdrId), "##Hdr_%d", colIdx);
                ImGui::TableHeader(hdrId);
                ImVec2 hMin = ImGui::GetItemRectMin();
                ImVec2 hMax = ImGui::GetItemRectMax();
                float textW = ImGui::CalcTextSize(headers[colIdx]).x;
                float textH = ImGui::GetTextLineHeight();
                float tx = hMin.x + (hMax.x - hMin.x - textW) * 0.5f;
                float ty = hMin.y + (hMax.y - hMin.y - textH) * 0.5f;
                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(tx, ty), ImGui::GetColorU32(ImGuiCol_Text),
                    headers[colIdx]);
              }
              ImGui::PopStyleColor();

              ImVec2 legendMin(btnX, targetLegendY);
              ImVec2 legendMax(btnX + legendW, targetLegendY + curWindowH);
              ImVec2 mouse = ImGui::GetMousePos();
              bool mouseInLegend =
                  (mouse.x >= legendMin.x && mouse.x <= legendMax.x &&
                   mouse.y >= legendMin.y && mouse.y <= legendMax.y);

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
                ImVec2 rowStart = ImGui::GetCursorScreenPos();
                bool isRowHovered = false;

                ImGui::TableSetColumnIndex(0);
                if (!isPlanned) {
                  // Opportunistic item: number with dot is highlighted in golden/amber color
                  ImVec2 numMin = ImGui::GetCursorScreenPos();
                  ImGui::TextColored(ImVec4(1.0f, 0.82f, 0.25f, 1.0f),
                                     "%zu. ", i + 1);
                  if (legendInteractable && !isTourActive_ && ImGui::IsItemHovered()) {
                    isRowHovered = true;
                  }
                  ImGui::SameLine(0.0f, 0.0f);
                  ImGui::TextColored(col, "%s", it.name.c_str());
                  if (legendInteractable && !isTourActive_ && ImGui::IsItemHovered()) {
                    isRowHovered = true;
                  }
                  // Underline opportunistic item with matching gold color (spanning both number and name)
                  ImVec2 nameMax = ImGui::GetItemRectMax();
                  ImGui::GetWindowDrawList()->AddLine(
                      ImVec2(numMin.x, nameMax.y - 1.0f),
                      ImVec2(nameMax.x, nameMax.y - 1.0f),
                      IM_COL32(255, 210, 64, 255), 1.5f);
                } else {
                  ImGui::TextColored(col, "%zu. %s", i + 1, it.name.c_str());
                  if (legendInteractable && !isTourActive_ && ImGui::IsItemHovered()) {
                    isRowHovered = true;
                  }
                }

                ImGui::TableSetColumnIndex(1);
                char wBuf[32];
                std::snprintf(wBuf, sizeof(wBuf), "%d кг", it.weight);
                float wW = ImGui::CalcTextSize(wBuf).x;
                float availW1 = ImGui::GetContentRegionAvail().x;
                if (availW1 > wW) {
                  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                       (availW1 - wW) * 0.5f);
                }
                ImGui::TextDisabled("%s", wBuf);
                if (legendInteractable && !isTourActive_ && ImGui::IsItemHovered()) {
                  isRowHovered = true;
                }

                ImGui::TableSetColumnIndex(2);
                char vBuf[32];
                std::snprintf(vBuf, sizeof(vBuf), "%d грн", it.value);
                float vW = ImGui::CalcTextSize(vBuf).x;
                float availW2 = ImGui::GetContentRegionAvail().x;
                if (availW2 > vW) {
                  ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                                       (availW2 - vW) * 0.5f);
                }
                ImGui::TextDisabled("%s", vBuf);
                if (legendInteractable && !isTourActive_ && ImGui::IsItemHovered()) {
                  isRowHovered = true;
                }

                // Check entire row bounding box for seamless hover strictly within the visible legend bounds
                float rowH = ImGui::GetTextLineHeight();
                float rowTop = rowStart.y;
                float rowBottom = rowStart.y + rowH + 4.0f;
                bool isRowVisibleInWindow =
                    (rowBottom >= targetLegendY && rowTop <= targetLegendY + curWindowH);

                if (legendInteractable && !isTourActive_ && mouseInLegend && isRowVisibleInWindow) {
                  if (isRowHovered ||
                      (mouse.y >= rowTop - 1.0f && mouse.y <= rowBottom &&
                       mouse.x >= btnX && mouse.x <= btnX + legendW)) {
                    isRowHovered = true;
                  }
                } else {
                  isRowHovered = false;
                }

                if (isRowHovered && !isTourActive_) {
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
            ImGui::PopStyleVar();
          }

          ImGui::Spacing();
          if (ImGui::Button("Вгору до легенди###ScrollUpCollectedBtn",
                            ImVec2(-1.0f, 26.0f * config.uiScale))) {
            isCollectedClosing_ = true;
            collectedScrollOnCloseStart_ = ImGui::GetScrollY();
          }
          if (legendInteractable && ImGui::IsItemHovered()) {
            showStyledTooltip(config.uiScale,
                              "Повернутися до початкового списку легенди.");
          }

          measuredCollectedContentH_ =
              std::max(40.0f, ImGui::GetCursorPosY() - collectedContentStartY + 12.0f);
        }

        if (hoveredCatalogItemName_.empty()) {
          lastHoveredCatalogItemName_.clear();
        }
        if (hoveredCatalogCategory_ < 0) {
          lastHoveredCatalogCategory_ = -1;
        }

        // Measure collapsed height only when completely open and both sublists closed
        if (legendInteractable && easeT >= 0.999f && !isCatalogOpen_ &&
            catalogOpenProgress_ <= 0.0f && !isCollectedOpen_ &&
            collectedOpenProgress_ <= 0.0f) {
          float measuredH = std::ceil(ImGui::GetCursorPosY() + padY);
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
        lastRenderedLegendBottomY_ = targetLegendY + curWindowH;
        ImGui::GetWindowDrawList()->PopClipRect();
      }
      ImGui::End();

      ImGui::PopStyleVar(6);
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

      if (ImGui::Button(LOC("[C] Центрувати на робітнику", "[C] Center on Worker"), ImVec2(btnW, btnH))) {
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
                            "%s", LOC("Центрування недоступне під час генерації та "
                                      "трасування маршруту.",
                                      "Centering unavailable during generation and routing."));
        } else {
          showStyledTooltip(
              config.uiScale, "%s",
              config.cameraFollowAgent
                  ? LOC("Вимкнути стеження камери за робітником.", "Disable camera following worker.")
                  : LOC("Центрувати та увімкнути стеження камери за робітником.", "Center and enable camera following worker."));
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
                        bool isManualMode,
                        bool compactMode = false) {
    const auto &agent = engine.getAgent();
    const auto &pos = agent.getPosition();

    // 1. Evacuation reached
    bool isExitDisabled =
        (engine.getConfig().routeMode == core::RouteMode::RoundTrip);
    const auto &evacTarget =
        isExitDisabled ? maze.getStartPoint() : maze.getExitPoint();
    if ((agent.getStatus() == simulation::AgentStatus::Success) ||
        (pos == evacTarget && agent.getStepsTaken() > 0)) {
      return {LOC("Евакуація...", "Evacuation..."), ImVec4(0.20f, 1.0f, 0.35f, 1.0f)};
    }

    // 2. Start point before departure / parachuting
    if (renderer.isWaitingForDrop() || renderer.isParachuting() ||
        (pos == maze.getStartPoint() && agent.getStepsTaken() == 0)) {
      return {LOC("Десантування...", "Drop-in..."), ImVec4(1.0f, 0.60f, 0.15f, 1.0f)};
    }

    // 3. Exhausted
    if (engine.getConfig().energyLimitEnabled &&
        (agent.isExhausted() ||
         agent.getStatus() == simulation::AgentStatus::Exhausted)) {
      return {LOC("Енергія вичерпана!", "Energy Depleted!"), ImVec4(0.92f, 0.05f, 0.05f, 1.0f)};
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
          return {std::string(LOC("Збір брухту: ", "Collecting scrap: ")) + optItem->name, catCol};
        }
      } else if (optItem.has_value() &&
                 engine.getConfig().backpackLimitEnabled) {
        int freeKg = std::max(0, agent.getBackpack().getMaxCapacity() -
                                     agent.getBackpack().getCurrentWeight());
        if (optItem->weight > freeKg) {
          std::string msg =
              (freeKg == 0)
                  ? (std::string(LOC("Рюкзак повний (", "Backpack full (")) + std::to_string(optItem->weight) +
                     LOC(" кг)", " kg)"))
                  : (std::string(LOC("Замало місця (", "Not enough space (")) + std::to_string(optItem->weight) +
                     " > " + std::to_string(freeKg) + LOC(" кг)", " kg)"));
          return {msg, ImVec4(1.0f, 0.36f, 0.24f, 1.0f)};
        }
      }
    }

    // 5. Manual control vs Autonomous
    if (isManualMode) {
      return {compactMode ? LOC("Ручний збір", "Manual Scrap") : LOC("Ручне збирання брухту", "Manual Scrap Collection"),
              ImVec4(1.0f, 0.82f, 0.25f, 1.0f)};
    } else {
      return {compactMode ? LOC("Авто-збирання", "Auto Tour") : LOC("Автономне збирання брухту", "Autonomous Route Tour"),
              ImVec4(0.68f, 0.94f, 1.00f, 1.0f)};
    }
  }

  void renderMissionStatusPanel(UIState &state, core::Config &config,
                                const map::Maze &maze,
                                const simulation::SimulationEngine &engine,
                                const ProceduralRenderer &renderer,
                                float windowWidth, float windowHeight) {
    float dt = ImGui::GetIO().DeltaTime;
    const float PANEL_ANIM_DUR = 0.24f;
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
        (engine.isManualMode() && !engine.isAtEvacuationCell() &&
         engine.getAgent().getStatus() != simulation::AgentStatus::Success) ||
        (isTourActive_ && tourCurrentStep_ == 9);
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
    float mmFrameH = Minimap::getFrameHeight(config.minimapScale);
    float monitorFrameTopY = windowHeight - mmFrameH - Minimap::PADDING;

    float panelW = mmFrameW;
    float panelX = windowWidth - mmFrameW - Minimap::PADDING;
    float panelPadX = std::round(10.0f * config.uiScale);
    float panelPadY = std::round(6.0f * config.uiScale);
    float panelGapY = std::round(4.0f * config.uiScale);
    float barH = std::max(18.0f * config.uiScale, ImGui::GetFontSize() + 4.0f);
    float panelH = std::round(panelPadY * 2.0f + ImGui::GetTextLineHeight() +
                              3.0f * barH + 3.0f * panelGapY);

    float attachEase = getHudAttachEase();
    if (hudIntroProgress_ < 0.999f || attachEase <= 0.001f) {
      return;
    }
    if (isTourActive_ && (tourCurrentStep_ == 0 || tourCurrentStep_ >= 2) && tourCurrentStep_ != 9) {
      return;
    }

    float btnH = std::max(28.0f * config.uiScale, ImGui::GetFrameHeight());

    float panelBottomY = monitorFrameTopY - 4.0f;
    float effectiveEaseT = easeT * attachEase;
    float currentPanelY = panelBottomY - panelH + (1.0f - effectiveEaseT) * panelH;
    float toggleBtnY = currentPanelY - (btnH + 3.0f) * attachEase;

    lastMissionPanelTopY_ = toggleBtnY;

    // 1. Toggle button sitting directly above the Mission Panel / Monitor
    if (!isTourActive_ || tourCurrentStep_ != 9) {
      ImGui::SetNextWindowPos(ImVec2(panelX, toggleBtnY), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(panelW, btnH), ImGuiCond_Always);

      ImGuiWindowFlags btnFlags =
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      if (ImGui::Begin("##MissionPanelTopToggleWindow", nullptr, btnFlags)) {
        const char *btnLabel = missionPanelCollapsed_
                                   ? LOC("[B] Показати показники місії", "[B] Show Mission Status")
                                   : LOC("[B] Сховати показники місії", "[B] Hide Mission Status");
        if (ImGui::Button(btnLabel, ImVec2(panelW, btnH))) {
          missionPanelCollapsed_ = !missionPanelCollapsed_;
        }
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(
              config.uiScale, "%s",
              missionPanelCollapsed_
                  ? LOC("Відкрити панель показників місії (Енергія, Вантаж, Квота) [B].",
                        "Open mission status HUD (Energy, Weight, Quota) [B].")
                  : LOC("Сховати панель показників місії [B].",
                        "Hide mission status HUD [B]."));
        }
      }
      ImGui::End();
      ImGui::PopStyleVar(2);
    }

    // 2. Mission status panel (Status line + 3 Progress bars)
    if (effectiveEaseT > 0.001f) {
      ImGui::SetNextWindowPos(ImVec2(panelX, currentPanelY), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_Always);

      ImGuiWindowFlags pFlags =
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
          ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                          ImVec2(panelPadX, panelPadY));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                          ImVec2(8.0f * config.uiScale, panelGapY));

      if (ImGui::Begin("##MissionStatusPanelWindow", nullptr, pFlags)) {
        ImDrawList *dl = ImGui::GetWindowDrawList();
        dl->PushClipRect(ImVec2(panelX - 10.0f, 0.0f),
                         ImVec2(panelX + panelW + 10.0f, monitorFrameTopY), true);
        ImU32 panelBgCol = IM_COL32(23, 20, 26, 245);
        ImU32 panelBorderCol = IM_COL32(122, 41, 61, 235);
        float roundRadius = 6.0f * config.uiScale;

        dl->AddRectFilled(ImVec2(panelX, currentPanelY),
                          ImVec2(panelX + panelW, currentPanelY + panelH),
                          panelBgCol, roundRadius);
        dl->AddRect(ImVec2(panelX, currentPanelY),
                    ImVec2(panelX + panelW, currentPanelY + panelH),
                    panelBorderCol, roundRadius, 0, 1.0f);
        lastMissionPanelBottomY_ = currentPanelY + panelH;

        bool isCompact = (panelW < 330.0f) || (config.minimapScale <= 1.05f);
        auto statusDisp = getAgentStatusDisplay(engine, maze, renderer,
                                               engine.isManualMode(),
                                               isCompact);

        ImGui::Text("%s", LOC("Стан робітника:", "Worker status:"));
        ImGui::SameLine();
        ImGui::TextColored(statusDisp.color, "%s", statusDisp.text.c_str());

        const auto &agent = engine.getAgent();

        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              ImVec4(0.20f, 0.15f, 0.03f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                              ImVec4(0.64f, 0.48f, 0.04f, 0.95f));
        if (engine.getConfig().energyLimitEnabled) {
          float energyFraction =
              static_cast<float>(agent.getEnergyPercentage()) / 100.0f;
          char energyBuf[64];
          std::snprintf(energyBuf, sizeof(energyBuf), "%s: %d / %d %s",
                        LOC("Енергія", "Energy"),
                        agent.getCurrentEnergy(), agent.getMaxEnergy(),
                        LOC("од.", "pts"));
          ImGui::ProgressBar(energyFraction, ImVec2(-1, barH), energyBuf);
        } else {
          ImGui::ProgressBar(1.0f, ImVec2(-1, barH), LOC("Енергія: Необмежена", "Energy: Unlimited"));
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(config.uiScale,
                            "%s", LOC("Поточний рівень заряду енергії робітника.",
                                      "Current battery energy charge of worker."));
        }

        if (engine.getConfig().backpackLimitEnabled) {
          float bpFraction =
              static_cast<float>(agent.getBackpack().getUtilizationPercent()) /
              100.0f;
          char bpBuf[64];
          std::snprintf(bpBuf, sizeof(bpBuf), "%s: %d / %d %s",
                        LOC("Вантаж", "Payload"),
                        agent.getBackpack().getCurrentWeight(),
                        agent.getBackpack().getMaxCapacity(),
                        LOC("кг", "kg"));
          ImGui::ProgressBar(bpFraction, ImVec2(-1, barH), bpBuf);
        } else {
          ImGui::ProgressBar(1.0f, ImVec2(-1, barH), LOC("Вантаж: Необмежений", "Payload: Unlimited"));
        }
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(config.uiScale, "%s", LOC("Заповненість рюкзака робітника.", "Current backpack payload utilization."));
        }

        int curValue = agent.getBackpack().getCurrentValue();
        int targetQuota = config.companyQuota;
        float quotaFraction =
            std::clamp(static_cast<float>(curValue) /
                           static_cast<float>(std::max(1, targetQuota)),
                       0.0f, 1.0f);
        char quotaBuf[64];
        bool isQuotaMet = (curValue >= targetQuota);
        bool isQuotaExceeded = (curValue >= targetQuota + 50);
        if (isQuotaExceeded) {
          std::snprintf(quotaBuf, sizeof(quotaBuf),
                        "%s: %d / %d %s [%s]",
                        LOC("Квота", "Quota"), curValue, targetQuota,
                        LOC("грн", "UAH"), LOC("ПЕРЕВИКОНАНО!", "EXCEEDED!"));
          ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                ImVec4(0.08f, 0.22f, 0.11f, 0.80f));
          ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                                ImVec4(0.12f, 0.55f, 0.22f, 0.95f));
        } else if (isQuotaMet) {
          std::snprintf(quotaBuf, sizeof(quotaBuf),
                        "%s: %d / %d %s [%s]",
                        LOC("Квота", "Quota"), curValue, targetQuota,
                        LOC("грн", "UAH"), LOC("ВИКОНАНО!", "MET!"));
          ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                ImVec4(0.08f, 0.22f, 0.11f, 0.80f));
          ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                                ImVec4(0.10f, 0.46f, 0.18f, 0.95f));
        } else {
          std::snprintf(quotaBuf, sizeof(quotaBuf), "%s: %d / %d %s (%d%%)",
                        LOC("Квота", "Quota"), curValue, targetQuota,
                        LOC("грн", "UAH"),
                        static_cast<int>(quotaFraction * 100.0f));
          ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                ImVec4(0.24f, 0.08f, 0.12f, 0.65f));
          ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                                ImVec4(1.00f, 0.72f, 0.18f, 1.0f));
        }
        ImGui::ProgressBar(quotaFraction, ImVec2(-1, barH), quotaBuf);
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
          showStyledTooltip(
              config.uiScale, "%s",
              isQuotaExceeded
                  ? LOC("Квоту Компанії ПЕРЕВИКОНАНО!", "Company Quota EXCEEDED!")
                  : isQuotaMet ? LOC("Мінімальну квоту Компанії виконано!", "Minimum Company Quota completed!")
                               : LOC("Прогрес виконання квоти Компанії.", "Company quota collection progress."));
        }
        dl->PopClipRect();
      }
      ImGui::End();
      ImGui::PopStyleVar(4);
    }

    // 3. Manual mode card: Centered vertically to the CRT monitor casing, to the left of monitor
    if (cardEaseT > 0.001f) {
      float cardPadX = 10.0f * config.uiScale;
      float cardPadY = 8.0f * config.uiScale;
      float cardBtnH =
          std::max(28.0f * config.uiScale, ImGui::GetFrameHeight() + 4.0f);
      const char *returnBtnLabel =
          LOC("[Z] Повернутись до останнього автоматичного кроку",
              "[Z] Rewind to last automated route step");
      float btnTextW = ImGui::CalcTextSize(returnBtnLabel).x;
      float cardW =
          std::round(btnTextW + cardPadX * 2.0f + 20.0f * config.uiScale);
      float cardTextLineH = ImGui::GetTextLineHeight();
      float cardTextGapY = std::round(3.0f * config.uiScale);
      float cardBtnGapY = std::round(6.0f * config.uiScale);
      float cardTextH = cardTextLineH * 2.0f + cardTextGapY;
      float cardH =
          std::round(cardPadY * 2.0f + cardTextH + cardBtnGapY + cardBtnH);

      float monitorBottomY = windowHeight - Minimap::PADDING;
      float targetCardY = std::round(monitorBottomY - cardH);
      float startCardY = windowHeight + 10.0f;
      float effectiveCardEase = (isTourActive_ && tourCurrentStep_ == 9) ? 1.0f : cardEaseT;
      float cardY = std::round(startCardY - (startCardY - targetCardY) * effectiveCardEase);
      float cardX = std::round(panelX - cardW - 8.0f);

      lastManualCardPos_ = ImVec2(cardX, cardY);
      lastManualCardSize_ = ImVec2(cardW, cardH);

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
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImU32 cardBgCol = IM_COL32(28, 22, 24, 245);
        ImU32 cardBorderCol = IM_COL32(180, 110, 35, 235);
        float roundRadius = 6.0f * config.uiScale;

        dl->AddRectFilled(ImVec2(cardX, cardY),
                          ImVec2(cardX + cardW, cardY + cardH),
                          cardBgCol, roundRadius);
        dl->AddRect(ImVec2(cardX, cardY),
                    ImVec2(cardX + cardW, cardY + cardH),
                    cardBorderCol, roundRadius, 0, 1.0f);

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
      }
      ImGui::End();
      ImGui::PopStyleVar(5);
    }
  }

  static bool renderLanguageToggleButton(const char* strId, ImVec2 pos, ImVec2 size, float alphaMultiplier = 1.0f) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGui::SetCursorPos(pos);

    bool clicked = ImGui::InvisibleButton(strId, size);
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    if (hovered) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    ImVec2 pMin = ImGui::GetItemRectMin();
    ImVec2 pMax = ImGui::GetItemRectMax();

    // Snap button boundaries to integer pixel grid to eliminate subpixel rasterization jitter
    pMin.x = std::round(pMin.x);
    pMin.y = std::round(pMin.y);
    pMax.x = std::round(pMax.x);
    pMax.y = std::round(pMax.y);

    float round = 6.0f;
    float btnH = pMax.y - pMin.y;
    float centerY = std::round((pMin.y + pMax.y) * 0.5f);

    // Frame background & border
    ImU32 colBg = active ? IM_COL32(26, 42, 62, static_cast<int>(245.0f * alphaMultiplier))
                         : (hovered ? IM_COL32(20, 32, 48, static_cast<int>(235.0f * alphaMultiplier))
                                    : IM_COL32(12, 17, 26, static_cast<int>(220.0f * alphaMultiplier)));
    ImU32 colBorder = hovered ? IM_COL32(0, 230, 255, static_cast<int>(245.0f * alphaMultiplier))
                              : IM_COL32(52, 90, 130, static_cast<int>(180.0f * alphaMultiplier));

    drawList->AddRectFilled(pMin, pMax, colBg, round);
    drawList->AddRect(pMin, pMax, colBorder, round, 0, 1.5f);

    // Vector Globe Icon on the left
    float globeRadius = std::clamp(btnH * 0.22f, 6.0f, 8.5f);
    ImVec2 globeCenter(std::round(pMin.x + globeRadius + 9.0f), centerY);
    ImU32 globeColor = hovered ? IM_COL32(0, 245, 255, static_cast<int>(255.0f * alphaMultiplier))
                               : IM_COL32(0, 200, 235, static_cast<int>(220.0f * alphaMultiplier));

    drawList->AddCircle(globeCenter, globeRadius, globeColor, 20, 1.3f);
    drawList->AddLine(ImVec2(globeCenter.x - globeRadius + 1.2f, globeCenter.y),
                      ImVec2(globeCenter.x + globeRadius - 1.2f, globeCenter.y),
                      globeColor, 1.1f);
    drawList->AddEllipse(globeCenter, ImVec2(globeRadius * 0.46f, globeRadius - 0.4f), globeColor, 0.0f, 20, 1.1f);

    bool isUA = Localization::instance().isUkrainian();

    ImVec2 uaTextSize = ImGui::CalcTextSize("UA");
    ImVec2 enTextSize = ImGui::CalcTextSize("EN");

    // Strictly bounded slot geometry inside [zoneLeft, zoneRight]
    float zoneLeft = std::round(globeCenter.x + globeRadius + 8.0f);
    float zoneRight = std::round(pMax.x - 6.0f);
    float zoneW = std::max(20.0f, zoneRight - zoneLeft);
    float gap = 6.0f;
    float slotW = std::round((zoneW - gap) * 0.5f);
    float slotH = std::round(std::clamp(btnH - 8.0f, 14.0f, btnH - 4.0f));

    // Slot 1: UA pill
    ImVec2 uaMin(zoneLeft, std::round(centerY - slotH * 0.5f));
    ImVec2 uaMax(zoneLeft + slotW, std::round(centerY + slotH * 0.5f));

    if (isUA) {
      drawList->AddRectFilled(uaMin, uaMax, IM_COL32(0, 200, 255, static_cast<int>(45.0f * alphaMultiplier)), 4.0f);
      drawList->AddRect(uaMin, uaMax, IM_COL32(0, 230, 255, static_cast<int>(180.0f * alphaMultiplier)), 4.0f, 0, 1.2f);
    }

    ImVec2 uaTextPos(std::round(uaMin.x + (slotW - uaTextSize.x) * 0.5f), std::round(centerY - uaTextSize.y * 0.5f));
    ImU32 colUaText = isUA ? IM_COL32(255, 255, 255, static_cast<int>(255.0f * alphaMultiplier))
                           : IM_COL32(130, 150, 170, static_cast<int>(180.0f * alphaMultiplier));
    drawList->AddText(uaTextPos, colUaText, "UA");

    // Slot 2: EN pill
    ImVec2 enMin(zoneRight - slotW, std::round(centerY - slotH * 0.5f));
    ImVec2 enMax(zoneRight, std::round(centerY + slotH * 0.5f));

    if (!isUA) {
      drawList->AddRectFilled(enMin, enMax, IM_COL32(0, 200, 255, static_cast<int>(45.0f * alphaMultiplier)), 4.0f);
      drawList->AddRect(enMin, enMax, IM_COL32(0, 230, 255, static_cast<int>(180.0f * alphaMultiplier)), 4.0f, 0, 1.2f);
    }

    ImVec2 enTextPos(std::round(enMin.x + (slotW - enTextSize.x) * 0.5f), std::round(centerY - enTextSize.y * 0.5f));
    ImU32 colEnText = !isUA ? IM_COL32(255, 255, 255, static_cast<int>(255.0f * alphaMultiplier))
                            : IM_COL32(130, 150, 170, static_cast<int>(180.0f * alphaMultiplier));
    drawList->AddText(enTextPos, colEnText, "EN");

    // Divider hairline centered between slots
    float dividerX = std::round((uaMax.x + enMin.x) * 0.5f);
    drawList->AddLine(ImVec2(dividerX, std::round(centerY - slotH * 0.35f)),
                      ImVec2(dividerX, std::round(centerY + slotH * 0.35f)),
                      IM_COL32(70, 95, 125, static_cast<int>(160.0f * alphaMultiplier)), 1.0f);

    if (hovered) {
      showStyledTooltip(
          1.0f, "%s",
          LOC("Мова інтерфейсу: Українська\nНатисніть для перемикання на English",
              "Interface Language: English\nClick to switch to Ukrainian"));
    }

    if (clicked) {
      Localization::instance().toggleLanguage();
    }
    return clicked;
  }

  static void applyTextGlitchToDrawList(ImDrawList* dl, float intensity, float timer,
                                        const std::vector<std::pair<float, float>>& glitchBands) {
    if (!dl || dl->VtxBuffer.Size == 0 || intensity <= 0.001f) {
      return;
    }

    ImVec2 whiteUv = ImGui::GetIO().Fonts->TexUvWhitePixel;
    auto hashInt = [](uint32_t x) -> float {
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = (x >> 16) ^ x;
      return static_cast<float>(x & 0xFFFF) / 65535.0f;
    };

    uint32_t tSeed = static_cast<uint32_t>(timer * 140.0f);

    for (int i = 0; i < dl->VtxBuffer.Size; ++i) {
      ImDrawVert& v = dl->VtxBuffer[i];
      // Check if vertex is textured glyph (not a solid rectangle / line / circle)
      if (std::abs(v.uv.x - whiteUv.x) < 0.0001f && std::abs(v.uv.y - whiteUv.y) < 0.0001f) {
        continue;
      }

      // 1. Line-based high-frequency horizontal micro-jitter
      uint32_t lineId = static_cast<uint32_t>(std::floor(v.pos.y / 15.0f));
      float lineRnd = hashInt(lineId * 1013u + tSeed * 17u);
      float microJitterX = (lineRnd * 2.0f - 1.0f) * 6.0f * intensity;
      v.pos.x += microJitterX;

      // 2. Heavy horizontal slice shearing if vertex falls inside an active glitch band
      for (const auto& band : glitchBands) {
        if (v.pos.y >= band.first && v.pos.y <= band.first + band.second) {
          float bandRnd = hashInt(static_cast<uint32_t>(band.first) * 31u + tSeed);
          float bandShift = (bandRnd * 2.0f - 1.0f) * 28.0f * intensity;
          v.pos.x += bandShift;
          break;
        }
      }

      // 3. Chromatic RGB text color aberration / flicker
      ImU32 origAlpha = (v.col >> IM_COL32_A_SHIFT) & 0xFF;
      if (origAlpha > 20) {
        float colRnd = hashInt(lineId * 509u + tSeed * 37u);
        if (colRnd < 0.28f * intensity) {
          // Tint text neon cyan
          v.col = IM_COL32(0, 245, 255, origAlpha);
        } else if (colRnd > 1.0f - 0.28f * intensity) {
          // Tint text neon red
          v.col = IM_COL32(255, 45, 80, origAlpha);
        }
      }
    }
  }

  static void renderLanguageGlitchOverlay(float windowWidth, float windowHeight) {
    if (!Localization::instance().isGlitching()) {
      return;
    }

    float progress = Localization::instance().getGlitchProgress();
    float timer = Localization::instance().getGlitchTimer();
    ImDrawList* fg = ImGui::GetForegroundDrawList();

    uint32_t seed = static_cast<uint32_t>(timer * 10000.0f) + 1337u;
    auto nextRnd = [&seed]() -> float {
      seed = (seed * 1664525u + 1013904223u);
      return static_cast<float>(seed & 0xFFFF) / 65535.0f;
    };

    float intensity = progress;

    // 1. Dark CRT interference vignette (atmospheric, non-blinding)
    if (intensity > 0.25f) {
      int darkAlpha = static_cast<int>((intensity - 0.25f) * 55.0f);
      fg->AddRectFilled(ImVec2(0.0f, 0.0f), ImVec2(windowWidth, windowHeight),
                        IM_COL32(0, 0, 0, darkAlpha));
    }

    // 2. High-density horizontal tearing slices (20 to 28 slices for heavy analog distortion)
    int numBands = 20 + static_cast<int>(nextRnd() * 9.0f);
    std::vector<std::pair<float, float>> activeBands;
    activeBands.reserve(numBands);

    for (int i = 0; i < numBands; ++i) {
      float bandY = nextRnd() * windowHeight;
      float bandH = 3.0f + nextRnd() * 22.0f;
      float shift = (nextRnd() * 2.0f - 1.0f) * 65.0f * intensity;
      activeBands.push_back({bandY, bandH});

      // Analog black scan sync bar (adds authentic CRT TV rolling without glare)
      if (nextRnd() > 0.70f) {
        int blackAlpha = static_cast<int>((35.0f + nextRnd() * 45.0f) * intensity);
        fg->AddRectFilled(
            ImVec2(0.0f, bandY),
            ImVec2(windowWidth, bandY + bandH * 0.6f),
            IM_COL32(0, 0, 0, blackAlpha));
      }

      // Clearly visible punchy cyan slice band
      int cyanAlpha = static_cast<int>((45.0f + nextRnd() * 55.0f) * intensity);
      fg->AddRectFilled(
          ImVec2(std::clamp(shift, -140.0f, windowWidth), bandY),
          ImVec2(std::clamp(windowWidth + shift, 0.0f, windowWidth + 140.0f), bandY + bandH),
          IM_COL32(0, 225, 250, cyanAlpha));

      // Opposite chromatic red slice band
      float redShift = -shift * 0.88f;
      int redAlpha = static_cast<int>((48.0f + nextRnd() * 60.0f) * intensity);
      fg->AddRectFilled(
          ImVec2(std::clamp(redShift, -140.0f, windowWidth), bandY + 0.5f),
          ImVec2(std::clamp(windowWidth + redShift, 0.0f, windowWidth + 140.0f), bandY + bandH),
          IM_COL32(250, 35, 70, redAlpha));

      // Digital micro-noise blocks on slices
      if (nextRnd() > 0.35f) {
        float blockX = nextRnd() * (windowWidth - 80.0f);
        float blockW = 18.0f + nextRnd() * 110.0f;
        fg->AddRectFilled(
            ImVec2(blockX, bandY),
            ImVec2(blockX + blockW, bandY + bandH * 0.75f),
            (nextRnd() > 0.5f) ? IM_COL32(0, 245, 255, static_cast<int>(75.0f * intensity))
                               : IM_COL32(255, 50, 85, static_cast<int>(75.0f * intensity)));
      }
    }

    // 3. Dense Digital Noise Static Grain (120 to 180 scattered analog grain packets)
    int numGrains = 120 + static_cast<int>(nextRnd() * 60.0f);
    for (int g = 0; g < numGrains; ++g) {
      float gx = nextRnd() * windowWidth;
      float gy = nextRnd() * windowHeight;
      float gw = 1.0f + nextRnd() * 6.0f;
      float gh = 1.0f + nextRnd() * 2.5f;
      ImU32 gCol = (nextRnd() > 0.5f)
          ? IM_COL32(0, 230, 255, static_cast<int>((40.0f + nextRnd() * 60.0f) * intensity))
          : IM_COL32(255, 50, 80, static_cast<int>((40.0f + nextRnd() * 60.0f) * intensity));
      fg->AddRectFilled(ImVec2(gx, gy), ImVec2(gx + gw, gy + gh), gCol);
    }

    // 4. CRT Horizontal Scanlines (tight, authentic analog raster)
    float scanlineSpacing = 3.0f;
    float scanOffset = std::fmod(timer * 340.0f, scanlineSpacing * 2.0f);
    int scanAlphaCyan = static_cast<int>(24.0f * intensity);
    int scanAlphaRed = static_cast<int>(22.0f * intensity);
    for (float y = scanOffset; y < windowHeight; y += scanlineSpacing * 2.0f) {
      fg->AddLine(ImVec2(0.0f, y), ImVec2(windowWidth, y),
                  IM_COL32(0, 220, 245, scanAlphaCyan), 1.0f);
      fg->AddLine(ImVec2(0.0f, y + scanlineSpacing), ImVec2(windowWidth, y + scanlineSpacing),
                  IM_COL32(245, 30, 65, scanAlphaRed), 1.0f);
    }

    // 5. Apply chromatic glitch and horizontal micro-shearing to ALL TEXT across all active ImGui windows
    ImGuiContext* g = ImGui::GetCurrentContext();
    if (g) {
      for (ImGuiWindow* win : g->Windows) {
        if (win && win->WasActive && win->DrawList) {
          applyTextGlitchToDrawList(win->DrawList, intensity, timer, activeBands);
        }
      }
      ImDrawList* bgDl = ImGui::GetBackgroundDrawList();
      if (bgDl && bgDl->VtxBuffer.Size > 0) {
        applyTextGlitchToDrawList(bgDl, intensity, timer, activeBands);
      }
    }

    // 6. Glitching Sci-Fi Notification Badge in Upper Center
    if (intensity > 0.15f) {
      const char* langStr = Localization::instance().isUkrainian()
          ? "[ SYSTEM LOCALE: UKRAINIAN / UA ]"
          : "[ SYSTEM LOCALE: ENGLISH / EN ]";
      ImVec2 textSize = ImGui::CalcTextSize(langStr);
      float badgeW = textSize.x + 36.0f;
      float badgeH = textSize.y + 16.0f;
      float badgeX = (windowWidth - badgeW) * 0.5f;
      float badgeY = 32.0f;

      float jx = (nextRnd() * 2.0f - 1.0f) * 5.0f * intensity;
      float jy = (nextRnd() * 2.0f - 1.0f) * 2.5f * intensity;

      ImVec2 bMin(std::round(badgeX + jx), std::round(badgeY + jy));
      ImVec2 bMax(std::round(badgeX + badgeW + jx), std::round(badgeY + badgeH + jy));

      fg->AddRectFilled(bMin, bMax, IM_COL32(10, 14, 22, static_cast<int>(230.0f * intensity)), 4.0f);
      fg->AddRect(bMin, bMax, IM_COL32(0, 230, 255, static_cast<int>(200.0f * intensity)), 4.0f, 0, 1.2f);

      ImVec2 tPos(badgeX + 18.0f + jx, badgeY + 8.0f + jy);
      fg->AddText(ImVec2(tPos.x + 3.0f * intensity, tPos.y),
                  IM_COL32(255, 32, 72, static_cast<int>(220.0f * intensity)), langStr);
      fg->AddText(ImVec2(tPos.x - 3.0f * intensity, tPos.y),
                  IM_COL32(0, 230, 255, static_cast<int>(220.0f * intensity)), langStr);
      fg->AddText(tPos, IM_COL32(255, 255, 255, static_cast<int>(255.0f * intensity)), langStr);
    }
  }

  void renderFloatingLanguageButton(float windowWidth, float windowHeight, float uiScale) {
    (void)windowWidth;
    (void)uiScale;
    if (isTourActive_) return;

    float currentWindowX = -sidebarWidth_ * (1.0f - sidebarOpenProgress_);
    float visibleSidebarRight = std::max(0.0f, currentWindowX + sidebarWidth_);
    float btnW = 120.0f;
    float btnH = 34.0f;
    float btnX = std::round(visibleSidebarRight + 16.0f);
    float btnY = std::round(windowHeight - btnH - 16.0f);

    // Provide 4px padding so the window's clipping rect never cuts the button's 1.5px border
    float pad = 4.0f;
    ImGui::SetNextWindowPos(ImVec2(btnX - pad, btnY - pad), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(btnW + pad * 2.0f, btnH + pad * 2.0f), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoBackground |
                             ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("##FloatingLangButton", nullptr, flags)) {
      renderLanguageToggleButton("##FloatingLangToggle", ImVec2(pad, pad), ImVec2(btnW, btnH), 1.0f);
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }

  void renderTourOverlay(float windowWidth, float windowHeight, float uiScale,
                         core::Config &config, UIState &state,
                         simulation::SimulationEngine &engine,
                         const map::Maze &maze) {
    if (!isTourActive_) {
      return;
    }
    (void)state;
    (void)engine;
    (void)maze;

    // Esc Key Hold Exit Detection (3.0 seconds hold)
    bool escPressed = ImGui::IsKeyDown(ImGuiKey_Escape);
    if (escPressed) {
      escHoldTimer_ += ImGui::GetIO().DeltaTime;
      if (escHoldTimer_ >= ESC_HOLD_DURATION) {
        endTour();
        return;
      }
    } else {
      escHoldTimer_ =
          std::max(0.0f, escHoldTimer_ - ImGui::GetIO().DeltaTime * 2.5f);
    }

    struct TourStepDef {
      const char *title;
      const char *subtitle;
      const char *text;
      int targetSection; // -1 for outside sidebar, 1..5 for sidebar sections
    };

    static constexpr int TOUR_TOTAL_STEPS = 20;
    static const TourStepDef TOUR_STEPS[TOUR_TOTAL_STEPS] = {
        {"Тактичний CRT-радар (Міні-карта)",
         "Повний огляд усього підземного комплексу в реальному часі",
         "- Відображає геометрію лабіринту, кімнати, коридори та стіни.\n"
         "- Кольорові мітки брухту (синій, жовтий, пурпуровий).\n"
         "- Позиція робітника з радіолокаційним пульсуючим маяком.\n"
         "- [ЛКМ на радарі] — миттєве переміщення камери у вибраний сектор.\n"
         "- [Коліщатко на радарі] — плавне масштабування карти комплексу.",
         -1},
        {"Показники місії (HUD)",
         "Телеметрія життєзабезпечення робітника та прогрес виконання плану",
         "- 'Стан робітника' — поточний статус (Збір брухту, Рух, Десантування / Евакуація, Очікування).\n"
         "- Смуга Енергії — запас енергії робітника для виконання автономної операції.\n"
         "- Смуга Вантажу — поточна вага та ліміт місткості рюкзака.\n"
         "- Смуга Квоти — прогрес виконання фінансового плану Компанії.\n"
         "- Кнопка зверху або клавіша [B] — швидке згортання/розгортання панелі.",
         -1},
        {"Легенда позначок мапи",
         "Довідник тактичних символів та позначок лабіринту",
         "- Розкривається кнопкою зверху або клавішею [M].\n"
         "- Пояснює колірні маркери: Шлюзи Старт (A) та Вихід (B), болото, пройдена траєкторія та слід.\n"
         "- Показує кольори трьох вагових категорій брухту: Дрібний, Побутовий, Дорогоцінний.\n"
         "- При наведенні на типи клітинок або брухт підсвічує їх на мапі.",
         -1},
        {"Каталог цінностей комплексу",
         "Повна база 60 найменувань промислового та побутового брухту",
         "- Вбудований каталог з трьома ваговими категоріями.\n"
         "- Наведення на предмет у каталозі активує тактичне фокусування камери на цьому предметі у комплексі.\n"
         "- Відображає точну вагу, базову ціну та питому цінність (грн/кг).\n"
         "- Допомагає візуально оцінити потенціал знахідок у кожному секторі.",
         -1},
        {"Вміст рюкзака / Зібраний брухт",
         "Деталізований маніфест завантажених у рюкзак предметів",
         "- Зелена кнопка 'Квота: X / Y грн (N/M од.)' відкриває інвентар зібраного.\n"
         "- Таблиця зібраного: порядковий номер, назва, вага та вартість.\n"
         "- Позначка та нижнє підкреслення золотим кольором — попутний брухт, який лежав на дорозі і який влазив у рюкзак.\n"
         "- Зелений банер сигналізує про успішне виконання квоти Компанії.",
         -1},
        {"Головний термінал керування",
         "Центральна консоль налаштування симуляції та алгоритмів",
         "- Відкривається натисканням [Esc] або кліком на бокову ручку зліва.\n"
         "- Складається з 5 функціональних блоків із неоновою індикацією.\n"
         "- Підтримує плавне змінення ширини панелі мишкою.\n"
         "- Дозволяє гнучко експериментувати з параметрами прямо під час місії.",
         1},
        {"Розділ 1 — Масштаб та фокусування камери",
         "Швидке позиціонування та навігація по простору лабіринту",
         "- Кнопки [-] Віддалити та [+] Наблизити — плавний дискретний зум.\n"
         "- 'Весь лабіринт' — авто-підбір масштабу для охоплення 100% мапи.\n"
         "- 'Центрувати' [C] — режим постійного стеження камери за робітником.\n"
         "- Повзунок масштабу камери (0.20x - 4.50x) для ручного точного налаштування.",
         1},
        {"Розділ 1 — Тактичні шари відображення",
         "Керування графічними оверлеями та продуктивністю рушія",
         "- 'Показувати сітку клітинок' — перемикання координатної сітки підлоги.\n"
         "- 'Показувати оптимальний маршрут' — відображення блакитної неонової лінії TSP.\n"
         "- 'Показувати пройдені клітинки' — візуалізація пройденого сліду робітника.\n"
         "- 'Вертикальна синхронізація (VSync)' — обмеження FPS під частоту монітора.\n"
         "- Масштаб інтерфейсу UI та розмір міні-карти.",
         1},
        {"Розділ 1 — Гарячі клавіші та екскурс",
         "Миттєвий доступ до всіх тактичних функцій з клавіатури",
         "- [Esc] — відкрити/сховати головну бічну панель керування.\n"
         "- [F11] — перемикання повноекранного безрамкового режиму.\n"
         "- [W, A, S, D] — ручне керування переміщенням робітника в комплексі.\n"
         "- [B] — сховати/показати HUD показників місії.\n"
         "- [C] — центрувати огляд на робітнику, [Z] — повернутися до маршруту.\n"
         "- [M] — відкрити/сховати легенду позначок мапи (каталог та інвентар).\n"
         "- [Space] — крок симуляції (затисніть для безперервного ходу).\n"
         "- [Shift+Space] — крок назад, [Ctrl+Space] — старт / фініш.",
         1},
        {"Розділ 1 — Ручне керування робітником [W, A, S, D]",
         "Пряме тактичне пілотування робітником та повернення до плану",
         "- Клавіші [W, A, S, D] — вільне ручне пересування комплексом у будь-який момент.\n"
         "- Дозволяє самостійно досліджувати коридори, збирати попутний брухт та оминати небезпеки.\n"
         "- При вході в ручний режим біля монітора з'являється тактична картка повернення.\n"
         "- Клавіша [Z] (або кнопка на картці) — миттєве повернення робітника до прокладеного алгоритмом плану.",
         1},
        {"Розділ 2 — Генерація комплексу BSP",
         "Процедурна генерація підземного комплексу (Binary Space Partitioning)",
         "- Розмір комплексу: від 20x20 до 100x100 клітинок.\n"
         "- Генерація мережі ізольованих кімнат та сполучних коридорів.\n"
         "- Власний сід (Seed): відтворюваність експериментів та конкретних карт.\n"
         "- Розподіл важкопрохідних боліт та випадковий розподіл цінностей.\n"
         "- Увага: для застосування параметрів необхідно натиснути 'Згенерувати новий лабіринт'!",
         2},
        {"Розділ 3 — Фінансові та ресурсні ліміти",
         "Критичні обмеження експедиції для збирача брухту",
         "- 'Квота компанії' — мінімальна сума (у грн), необхідна для успішної евакуації.\n"
         "- 'Запас енергії' — ліміт очок дії (переміщення прямо = 10 од., по діагоналі = 14 од., болото = +25/+35 од.).\n"
         "- 'Місткість рюкзака' — гранична вага (у кг), яку здатен нести робітник.\n"
         "- Кнопки [+] дозволяють миттєво підігнати ресурси під поточний маршрут.",
         3},
        {"Розділ 3 — Алгоритми задачі про рюкзак",
         "Вибір інтелектуальної стратегії відбору предметів (Knapsack Problem)",
         "- 0/1 Knapsack (DP): метод динамічного програмування, що гарантує точний глобальний оптимум максимізації вартості під ліміт ваги.\n"
         "- Greedy Knapsack: швидка жадібна евристика за питомою цінністю (V/W). Працює за O(N log N), оптимальна для великих комплексів.",
         3},
        {"Розділ 3 — Алгоритми пошуку шляху",
         "Обчислення найкоротших траєкторій між ключовими вузлами",
         "- A* (A-Star): евристичний пошук з октагональною евристикою та врахуванням енерговитрат по болотах. Швидкий та адаптивний.\n"
         "- Алгоритм Дейкстри: гарантований точний пошук найдешевшого маршруту у зваженому графі без евристики.\n"
         "- BFS (Breadth-First Search): хвильовий пошук у ширину в незваженому просторі клітинок.",
         3},
        {"Розділ 3 — Дослідницькі режими експедиції",
         "Вимкнення фізичних обмежень для тестування та аналізу",
         "- 'Необмежена енергія' — робітник не вичерпує батарею, алгоритм будує повний маршрут.\n"
         "- 'Необмежений рюкзак' — скасовує ваговий ліміт, дозволяючи зібрати 100% брухту в комплексі.\n"
         "- Зручно для наукового порівняння та демонстрації граничних можливостей алгоритмів.",
         3},
        {"Розділ 3 — Режими евакуації та консервація шлюзів",
         "Вибір кінцевої точки місії (A -> B або A -> A)",
         "- 'Точка A -> B (Вихід)' — наскрізна евакуація через аварійний шлюз виходу (B).\n"
         "- 'A -> A (База)' — повернення до вхідного шлюзу (A). Шлюз виходу автоматично консервується та позначається червоним хрестом на карті.\n"
         "- Дозволяє моделювати ситуацію заблокованого аварійного шлюзу або повторного повернення на базу.",
         3},
        {"Розділ 3 — TSP Оптимізація маршруту та перерахунок",
         "Задача комівояжера для обходу зібраного рюкзака цінностей",
         "- Алгоритм найближчого сусіда + 2-Opt локальний пошук для мінімізації довжини шляху між точками збору.\n"
         "- Кнопка 'Перерахувати оптимальний план' світиться помаранчевим при будь-якій зміні лімітів або алгоритмів.\n"
         "- Рядок статусу показує актуальну кількість предметів, кроків та витрату енергії.",
         3},
        {"Розділ 3 — JSON-конфігурація та звіти",
         "Збереження налаштувань місії та експорт аудиторської звітності",
         "- 'Завантажити input_config.json' — застосування збереженої конфігурації.\n"
         "- 'Зберегти конфіг у JSON' — фіксація поточної карти, сіду, квоти та параметрів.\n"
         "- 'Експортувати report_route.json' — повний науково-технічний звіт оптимізації рюкзака, маршруту TSP та координат.",
         3},
        {"Розділ 4 — Плеєр симуляції експедиції",
         "Покрокове та неперервне відтворення руху робітника",
         "- Лічильник кроків показує поточну позицію на маршруті та загальну довжину.\n"
         "- [ < ] — крок назад (перемотування історії та повернення енергії).\n"
         "- [ ▶ ] / [ || ] — запуск симуляції / пауза.\n"
         "- [ > ] — одиночний крок вперед по розрахованому маршруту.\n"
         "- [ |< ] (На початок) та [ >| ] (Одразу на фініш) — швидка навігація [Ctrl+Space].\n"
         "- Повзунок швидкості ходу від 0.5x до 4.0x.",
         4},
        {"Розділ 5 — Порівняльний аналіз (Бенчмарки)",
         "Науково-дослідний полігон оцінки комбінацій алгоритмів",
         "- Кнопка 'Запустити повний аналіз' тестує всі 6 пар алгоритмів (Knapsack x Pathfinding).\n"
         "- Точні метрики часу (мкс/мс), витрати енергії, кроків та відсотка оптимуму.\n"
         "- Відображення приросту ефективності від евристики 2-Opt.\n"
         "- Кнопки експорту таблиці у CSV та повного звіту у JSON.",
         5}};

    tourCurrentStep_ = std::clamp(tourCurrentStep_, 0, TOUR_TOTAL_STEPS - 1);
    const auto &step = TOUR_STEPS[tourCurrentStep_];

    if (tourCurrentStep_ != lastTourStep_) {
      lastTourStep_ = tourCurrentStep_;
      tourTextAlpha_ = 0.0f;
    }

    // Synchronize UI panels for the active step
    float mmFrameW = Minimap::getFrameWidth(config.minimapScale);
    float mmFrameH = Minimap::getFrameHeight(config.minimapScale);
    float monitorLeftX = windowWidth - mmFrameW - Minimap::PADDING;
    float monitorTopY = windowHeight - mmFrameH - Minimap::PADDING;
    float legendW = std::max(340.0f * config.uiScale, mmFrameW);
    float legendRightX = windowWidth - Minimap::PADDING;
    float legendLeftX = legendRightX - legendW;
    float legendBtnH = std::max(34.0f * config.uiScale, ImGui::GetFrameHeight() + 6.0f);
    float baseLegendH = 345.0f * config.uiScale;
    float collapsedH = (collapsedLegendH_ > 50.0f) ? collapsedLegendH_ : baseLegendH;

    ImVec2 targetSpotMin(0.0f, 0.0f);
    ImVec2 targetSpotMax(0.0f, 0.0f);

    if (tourCurrentStep_ < 5) {
      sidebarCollapsed_ = true;
      sidebarOpenProgress_ = 0.0f;
      sidebarScrollTargetY_ = -1.0f;
      if (tourCurrentStep_ == 0) {
        config.showLegend = false;
        legendAnimT_ = 0.0f;
        missionPanelCollapsed_ = true;
        missionPanelOpenProgress_ = 0.0f;
        isCatalogOpen_ = false;
        isCollectedOpen_ = false;
        targetSpotMin = ImVec2(monitorLeftX - 2.0f, monitorTopY - 2.0f);
        targetSpotMax = ImVec2(std::min(windowWidth - 2.0f, monitorLeftX + mmFrameW + 2.0f),
                               std::min(windowHeight - 2.0f, monitorTopY + mmFrameH + 2.0f));
      } else if (tourCurrentStep_ == 1) {
        config.showLegend = false;
        legendAnimT_ = 0.0f;
        missionPanelCollapsed_ = false;
        missionPanelOpenProgress_ = 1.0f;
        isCatalogOpen_ = false;
        isCollectedOpen_ = false;
        float actualTop = (lastMissionPanelTopY_ > 10.0f) ? lastMissionPanelTopY_ : (monitorTopY - 140.0f * uiScale);
        float actualBottom = (lastMissionPanelBottomY_ > 10.0f) ? lastMissionPanelBottomY_ : (monitorTopY - 4.0f);
        float panelX = windowWidth - mmFrameW - Minimap::PADDING;
        float panelW = mmFrameW;
        float pad = 4.0f * uiScale;
        targetSpotMin = ImVec2(panelX - pad, actualTop - pad);
        targetSpotMax = ImVec2(std::min(windowWidth - 2.0f, panelX + panelW + pad),
                               actualBottom + pad);
      } else if (tourCurrentStep_ == 2) {
        missionPanelCollapsed_ = true;
        missionPanelOpenProgress_ = 0.0f;
        config.showLegend = true;
        legendAnimT_ = 1.0f;
        legendScroller_.targetScrollY = 0.0f;
        isCatalogOpen_ = false;
        isCollectedOpen_ = false;
        float btmY = (lastRenderedLegendBottomY_ > 10.0f) ? lastRenderedLegendBottomY_ : (legendBtnH + 6.0f + collapsedH);
        targetSpotMin = ImVec2(legendLeftX - 4.0f, 2.0f);
        targetSpotMax = ImVec2(legendRightX + 4.0f, btmY + 2.0f);
      } else if (tourCurrentStep_ == 3) {
        missionPanelCollapsed_ = true;
        missionPanelOpenProgress_ = 0.0f;
        config.showLegend = true;
        legendAnimT_ = 1.0f;
        legendScroller_.targetScrollY = catalogBtnLocalY_;
        isCatalogOpen_ = true;
        isCatalogClosing_ = false;
        catalogOpenProgress_ = 1.0f;
        isCollectedOpen_ = false;
        float btmY = (lastRenderedLegendBottomY_ > 10.0f) ? lastRenderedLegendBottomY_ : (monitorTopY - 6.0f);
        targetSpotMin = ImVec2(legendLeftX - 4.0f, 2.0f);
        targetSpotMax = ImVec2(legendRightX + 4.0f, btmY + 2.0f);
      } else if (tourCurrentStep_ == 4) {
        missionPanelCollapsed_ = true;
        missionPanelOpenProgress_ = 0.0f;
        config.showLegend = true;
        legendAnimT_ = 1.0f;
        legendScroller_.targetScrollY = collectedBtnLocalY_;
        isCatalogOpen_ = false;
        isCollectedOpen_ = true;
        isCollectedClosing_ = false;
        collectedOpenProgress_ = 1.0f;
        float btmY = (lastRenderedLegendBottomY_ > 10.0f) ? lastRenderedLegendBottomY_ : (legendBtnH + 6.0f + collapsedH + measuredCollectedContentH_);
        targetSpotMin = ImVec2(legendLeftX - 4.0f, 2.0f);
        targetSpotMax = ImVec2(legendRightX + 4.0f, btmY + 2.0f);
      }
    } else {
      sidebarCollapsed_ = false;
      sidebarOpenProgress_ = 1.0f;
      config.showLegend = false;
      legendAnimT_ = 0.0f;
      missionPanelCollapsed_ = true;
      missionPanelOpenProgress_ = 0.0f;
      isCatalogOpen_ = false;
      isCollectedOpen_ = false;
      float sideW = sidebarWidth_;
      if (tourCurrentStep_ == 5) {
        targetSpotMin = ImVec2(0.0f, 0.0f);
        targetSpotMax = ImVec2(sideW + 4.0f, windowHeight);
        sidebarScrollTargetY_ = 0.0f;
      } else if (tourCurrentStep_ == 9) {
        float pad = 4.0f * uiScale;
        float cardX = (lastManualCardSize_.x > 10.0f) ? lastManualCardPos_.x : (monitorLeftX - 320.0f * uiScale);
        float cardY = (lastManualCardSize_.y > 10.0f) ? lastManualCardPos_.y : (windowHeight - Minimap::PADDING - 110.0f * uiScale);
        float cardW = (lastManualCardSize_.x > 10.0f) ? lastManualCardSize_.x : 300.0f * uiScale;
        float cardH = (lastManualCardSize_.y > 10.0f) ? lastManualCardSize_.y : 100.0f * uiScale;
        targetSpotMin = ImVec2(cardX - pad, cardY - pad);
        targetSpotMax = ImVec2(cardX + cardW + pad, cardY + cardH + pad);
        sidebarScrollTargetY_ = tourBlockLocalY_[8];
      } else if ((tourCurrentStep_ >= 6 && tourCurrentStep_ <= 8) || (tourCurrentStep_ >= 10 && tourCurrentStep_ <= 19)) {
        float itemH = std::max(40.0f * config.uiScale, tourBlockH_[tourCurrentStep_]);
        float topPad = 6.0f * uiScale;
        float botPad = 6.0f * uiScale;
        float titleBarBottom = 34.0f * config.uiScale;

        if (tourBlockMax_[tourCurrentStep_].y > tourBlockMin_[tourCurrentStep_].y) {
          targetSpotMin = ImVec2(4.0f * uiScale, std::max(titleBarBottom + 2.0f * uiScale, tourBlockMin_[tourCurrentStep_].y - topPad));
          targetSpotMax = ImVec2(sideW - 6.0f, tourBlockMax_[tourCurrentStep_].y + botPad);
        } else {
          float itemScreenY = tourBlockLocalY_[tourCurrentStep_] - sidebarScroller_.smoothScrollY;
          targetSpotMin = ImVec2(4.0f * uiScale, std::max(titleBarBottom + 2.0f * uiScale, itemScreenY - topPad));
          targetSpotMax = ImVec2(sideW - 6.0f, itemScreenY + itemH + botPad);
        }
        sidebarScrollTargetY_ = std::max(0.0f, tourBlockLocalY_[tourCurrentStep_] - 44.0f * config.uiScale);
      }
    }

    // Keyboard navigation
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
      tourCurrentStep_ = std::min(TOUR_TOTAL_STEPS - 1, tourCurrentStep_ + 1);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
      tourCurrentStep_ = std::max(0, tourCurrentStep_ - 1);
    }

    // Interpolate spotlight and card position smoothly
    float dt = ImGui::GetIO().DeltaTime;
    tourDimAlpha_ = std::min(1.0f, tourDimAlpha_ + dt * 4.5f);
    float cardW = std::min(520.0f * uiScale, windowWidth - 40.0f);
    float cardEstH = 220.0f * uiScale;
    float botNavH = 62.0f * uiScale;
    float maxCardY = windowHeight - botNavH - 18.0f * uiScale - cardEstH - 12.0f * uiScale;
    float targetCardY = std::clamp(targetSpotMin.y + 10.0f * uiScale, 64.0f * uiScale, maxCardY);
    float targetCardX = 20.0f * uiScale;
    if (tourCurrentStep_ == 9) {
      targetCardX = std::clamp(targetSpotMin.x, sidebarWidth_ + 20.0f * uiScale, windowWidth - cardW - 20.0f * uiScale);
      targetCardY = std::max(56.0f * uiScale, targetSpotMin.y - cardEstH - 32.0f * uiScale);
    } else if (tourCurrentStep_ < 5) {
      targetCardX = std::max(20.0f * uiScale, targetSpotMin.x - cardW - 24.0f * uiScale);
    } else {
      targetCardX = std::min(windowWidth - cardW - 20.0f * uiScale, targetSpotMax.x + 24.0f * uiScale);
    }

    if (!hasValidSpotlight_) {
      hasValidSpotlight_ = true;
      currentSpotMin_ = targetSpotMin;
      currentSpotMax_ = targetSpotMax;
      currentCardPos_ = ImVec2(targetCardX, targetCardY);
      tourTextAlpha_ = 1.0f;
    } else {
      float tEase = std::clamp(dt * 15.0f, 0.0f, 1.0f);
      currentSpotMin_.x += (targetSpotMin.x - currentSpotMin_.x) * tEase;
      currentSpotMin_.y += (targetSpotMin.y - currentSpotMin_.y) * tEase;
      currentSpotMax_.x += (targetSpotMax.x - currentSpotMax_.x) * tEase;
      currentSpotMax_.y += (targetSpotMax.y - currentSpotMax_.y) * tEase;
      currentCardPos_.x += (targetCardX - currentCardPos_.x) * tEase;
      currentCardPos_.y += (targetCardY - currentCardPos_.y) * tEase;
      tourTextAlpha_ = std::min(1.0f, tourTextAlpha_ + dt * 6.5f);
    }

    // Draw Darkening Overlay with Spotlight Cutout as an ImGui window so subsequent
    // tour card windows and bottom navigation render ON TOP of it.
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
    ImGuiWindowFlags dimFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("##TourDimWindow", nullptr, dimFlags)) {
      ImDrawList *dl = ImGui::GetWindowDrawList();
      ImU32 dimCol = IM_COL32(4, 4, 8, static_cast<int>(205.0f * tourDimAlpha_));

      if (currentSpotMax_.x > currentSpotMin_.x && currentSpotMax_.y > currentSpotMin_.y) {
        if (currentSpotMin_.y > 0.0f) {
          dl->AddRectFilled(ImVec2(0.0f, 0.0f), ImVec2(windowWidth, currentSpotMin_.y),
                            dimCol);
        }
        if (currentSpotMax_.y < windowHeight) {
          dl->AddRectFilled(ImVec2(0.0f, currentSpotMax_.y),
                            ImVec2(windowWidth, windowHeight), dimCol);
        }
        if (currentSpotMin_.x > 0.0f) {
          dl->AddRectFilled(ImVec2(0.0f, currentSpotMin_.y),
                            ImVec2(currentSpotMin_.x, currentSpotMax_.y), dimCol);
        }
        if (currentSpotMax_.x < windowWidth) {
          dl->AddRectFilled(ImVec2(currentSpotMax_.x, currentSpotMin_.y),
                            ImVec2(windowWidth, currentSpotMax_.y), dimCol);
        }

        // High-visibility glowing cyan spotlight border
        dl->AddRect(currentSpotMin_, currentSpotMax_,
                    IM_COL32(0, 220, 255, static_cast<int>(240.0f * tourDimAlpha_)),
                    6.0f * uiScale, 0, 2.5f * uiScale);
        dl->AddRect(ImVec2(currentSpotMin_.x - 2.0f, currentSpotMin_.y - 2.0f),
                    ImVec2(currentSpotMax_.x + 2.0f, currentSpotMax_.y + 2.0f),
                    IM_COL32(0, 180, 255, static_cast<int>(75.0f * tourDimAlpha_)),
                    8.0f * uiScale, 0, 1.5f * uiScale);
      } else {
        dl->AddRectFilled(ImVec2(0.0f, 0.0f), ImVec2(windowWidth, windowHeight),
                          dimCol);
      }
    }
    ImGui::End();

    // Esc Hold Banner at top center
    float bannerW = std::min(380.0f * uiScale, windowWidth - 32.0f);
    float bannerH = 46.0f * uiScale;
    float bannerX = (windowWidth - bannerW) * 0.5f;
    float bannerY = 12.0f * uiScale;

    ImGui::SetNextWindowPos(ImVec2(bannerX, bannerY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bannerW, bannerH), ImGuiCond_Always);
    ImGuiWindowFlags bFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(8.0f * uiScale, 4.0f * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (ImGui::Begin("##TourEscHoldBanner", nullptr, bFlags)) {
      ImDrawList *dl = ImGui::GetWindowDrawList();
      dl->AddRectFilled(ImVec2(bannerX, bannerY),
                        ImVec2(bannerX + bannerW, bannerY + bannerH),
                        IM_COL32(20, 22, 30, 240), 6.0f * uiScale);
      dl->AddRect(ImVec2(bannerX, bannerY),
                  ImVec2(bannerX + bannerW, bannerY + bannerH),
                  IM_COL32(0, 180, 220, 220), 6.0f * uiScale, 0, 1.2f);

      float holdFraction =
          std::clamp(escHoldTimer_ / ESC_HOLD_DURATION, 0.0f, 1.0f);
      char escText[64];
      if (escHoldTimer_ > 0.05f) {
        std::snprintf(escText, sizeof(escText), "%s: %.1f / 2.0 %s",
                      LOC("Вихід з екскурсу", "Exit Tour"),
                      escHoldTimer_,
                      LOC("с", "s"));
      } else {
        std::snprintf(escText, sizeof(escText), "%s",
                      LOC("Затисніть [Esc] на 2 сек для виходу",
                          "Hold [Esc] for 2 sec to exit"));
      }
      float tW = ImGui::CalcTextSize(escText).x;
      ImGui::SetCursorPosX((bannerW - tW) * 0.5f);
      ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.40f, 1.0f), "%s", escText);

      ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                            ImVec4(0.95f, 0.40f, 0.20f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.16f, 0.8f));
      ImGui::ProgressBar(holdFraction, ImVec2(-1, 6.0f * uiScale), "");
      ImGui::PopStyleColor(2);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);

    // Floating Interactive Tour Card (smoothly glides and adapts)
    ImGui::SetNextWindowPos(currentCardPos_, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(cardW, 0.0f), ImGuiCond_Always);

    ImGuiWindowFlags cardFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.13f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.95f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f * uiScale);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f * uiScale);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(14.0f * uiScale, 12.0f * uiScale));

    if (ImGui::Begin("##InteractiveTourCardWindow", nullptr, cardFlags)) {
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tourTextAlpha_);
      ImGui::TextColored(ImVec4(0.00f, 0.90f, 1.00f, 1.0f), "%s %d / %d:",
                         LOC("Крок", "Step"),
                         tourCurrentStep_ + 1, TOUR_TOTAL_STEPS);
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(1.00f, 1.00f, 1.00f, 1.0f), "%s", step.title);

      ImGui::TextColored(ImVec4(0.85f, 0.75f, 0.40f, 1.0f), "%s",
                         step.subtitle);
      ImGui::Separator();
      ImGui::Spacing();

      ImGui::PushTextWrapPos(cardW - 28.0f * uiScale);
      ImGui::TextUnformatted(step.text);
      ImGui::PopTextWrapPos();

      ImGui::Spacing();
      ImGui::PopStyleVar();
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    // Bottom Navigation Bar with 2 wide cyan buttons: [ <  Назад ]  [ Далі > / Завершити ✓ ]
    float botNavW = 340.0f * uiScale;
    float botNavX = (windowWidth - botNavW) * 0.5f;
    float botNavY = windowHeight - botNavH - 18.0f * uiScale;

    ImGui::SetNextWindowPos(ImVec2(botNavX, botNavY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(botNavW, botNavH), ImGuiCond_Always);
    ImGuiWindowFlags botNavFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.13f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.85f, 1.00f, 0.95f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f * uiScale);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f * uiScale);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(10.0f * uiScale, 8.0f * uiScale));

    if (ImGui::Begin("##TourBottomNavBarWindow", nullptr, botNavFlags)) {
      float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
      float totalAvail = ImGui::GetContentRegionAvail().x;
      float arrowBtnW = (totalAvail - itemSpacing) * 0.5f;
      float btnH = 44.0f * uiScale;

      bool isFirst = (tourCurrentStep_ == 0);
      bool isLast = (tourCurrentStep_ == TOUR_TOTAL_STEPS - 1);

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.42f, 0.52f, 0.90f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(0.18f, 0.56f, 0.70f, 1.00f));
      if (isFirst) {
        ImGui::BeginDisabled(true);
      }
      if (boldFont_) ImGui::PushFont(boldFont_);
      if (ImGui::Button(LOC(" <  Назад ", " <  Back "), ImVec2(arrowBtnW, btnH))) {
        tourCurrentStep_ = std::max(0, tourCurrentStep_ - 1);
      }
      if (boldFont_) ImGui::PopFont();
      if (isFirst) {
        ImGui::EndDisabled();
      } else if (ImGui::IsItemHovered()) {
        showStyledTooltip(uiScale, "%s", LOC("Попередній крок", "Previous step"));
      }
      ImGui::PopStyleColor(2);

      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.42f, 0.52f, 0.90f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(0.18f, 0.56f, 0.70f, 1.00f));
      const char *nextLabel = isLast ? LOC(" Завершити ✓ ", " Finish ✓ ") : LOC(" Далі > ", " Next > ");
      if (boldFont_) ImGui::PushFont(boldFont_);
      if (ImGui::Button(nextLabel, ImVec2(arrowBtnW, btnH))) {
        if (isLast) {
          endTour();
        } else {
          tourCurrentStep_++;
        }
      }
      if (boldFont_) ImGui::PopFont();
      ImGui::PopStyleColor(2);
      if (ImGui::IsItemHovered()) {
        showStyledTooltip(uiScale, "%s", isLast ? LOC("Завершити екскурс", "Finish tour")
                                                : LOC("Наступний крок", "Next step"));
      }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    // Render Foreground Cyan Glowing Spotlight Border on top of ALL windows
    ImDrawList *fgDl = ImGui::GetForegroundDrawList();
    if (tourDimAlpha_ > 0.05f && currentSpotMax_.x > currentSpotMin_.x && currentSpotMax_.y > currentSpotMin_.y) {
      fgDl->AddRect(currentSpotMin_, currentSpotMax_,
                    IM_COL32(0, 220, 255, static_cast<int>(255.0f * tourDimAlpha_)),
                    6.0f * uiScale, 0, 2.5f * uiScale);
      fgDl->AddRect(ImVec2(currentSpotMin_.x - 2.0f, currentSpotMin_.y - 2.0f),
                    ImVec2(currentSpotMax_.x + 2.0f, currentSpotMax_.y + 2.0f),
                    IM_COL32(0, 180, 255, static_cast<int>(90.0f * tourDimAlpha_)),
                    8.0f * uiScale, 0, 1.5f * uiScale);
    }
  }

  void renderWorldTooltip(
      const map::Maze &maze, const simulation::SimulationEngine &engine,
      const sf::Vector2f &worldPos, bool mouseOverUI,
      const sf::Vector2i &mousePixel = {0, 0}, float windowWidth = 1920.0f,
      float windowHeight = 1080.0f, bool isExitDisabled = false,
      float scanRadius = -1.0f, const sf::Vector2f &scanOrigin = {0.0f, 0.0f},
      bool isSonarScanning = false, float routeProgress = 1.0f,
      bool isFadingOut = false, bool showOptimalRoute = true,
      bool showWalkedCells = true) {
    if (mouseOverUI || isFadingOut || isTourActive_)
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
    if (showOptimalRoute && pCell && !plan.fullPath.empty() && routeProgress > 0.001f) {
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
                           "%s", LOC("Шлюз (Старт) — Вхідна стрілка", "Airlock (Start) — Entry Arrow"));
        ImGui::Separator();
        ImGui::TextUnformatted(
            LOC("Вектор заходу робітника у комплекс через початковий шлюз",
                "Worker entry trajectory into the complex via start airlock"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати шлюзу", "Airlock coords"), startPt.x, startPt.y);
      } else if (isHoveringExitArrow) {
        ImGui::TextColored(ImVec4(0.18f, 0.80f, 0.44f, 1.0f),
                           "%s", LOC("Вихід (Евакуація) — Стрілка евакуації", "Exit (Evacuation) — Evacuation Arrow"));
        ImGui::Separator();
        ImGui::TextUnformatted(
            LOC("Вектор виходу та евакуації робітника з комплексу",
                "Worker evacuation vector out of the industrial complex"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати шлюзу", "Airlock coords"), maze.getExitPoint().x,
                    maze.getExitPoint().y);
      } else if (isHoveringAgent) {
        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "%s", LOC("Робітник", "Worker"));
        ImGui::Separator();
        ImGui::TextUnformatted(
            LOC("Поточна позиція автономного робітника (GPS тактичний маяк)",
                "Current position of autonomous worker (GPS beacon)"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
        ImGui::Text("%s: %s", LOC("Стан", "Status"), agent.getStatusString().c_str());
        ImGui::Text("%s: %d / %d %s", LOC("Запас енергії", "Energy reserve"), agent.getCurrentEnergy(),
                    agent.getMaxEnergy(), LOC("од.", "pts"));
        ImGui::Text("%s: %d / %d %s (%d %s)",
                    LOC("Вантаж", "Payload"),
                    agent.getBackpack().getCurrentWeight(),
                    agent.getBackpack().getMaxCapacity(),
                    LOC("кг", "kg"),
                    agent.getBackpack().getCurrentValue(),
                    LOC("грн", "UAH"));
      } else if (itemHere.has_value() && !isCollected) {
        if (itemHere->category == core::ItemCategory::LightScrap) {
          ImGui::TextColored(ImVec4(0.20f, 0.41f, 1.00f, 1.0f),
                             "%s: %s", LOC("Дрібний брухт", "Small Scrap"), itemHere->name.c_str());
        } else if (itemHere->category == core::ItemCategory::MediumScrap) {
          ImGui::TextColored(ImVec4(1.00f, 1.00f, 0.00f, 1.0f),
                             "%s: %s", LOC("Побутовий брухт", "Household Scrap"), itemHere->name.c_str());
        } else {
          ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
                             "%s: %s", LOC("Дорогоцінний брухт", "Precious Scrap"), itemHere->name.c_str());
        }
        ImGui::Separator();
        ImGui::Text("%s: %s", LOC("Категорія", "Category"),
                    core::itemCategoryToString(itemHere->category).c_str());
        ImGui::Text("%s: %d %s", LOC("Вага", "Weight"), itemHere->weight, LOC("кг", "kg"));
        ImGui::Text("%s: %d %s", LOC("Цінність", "Value"), itemHere->value, LOC("грн", "UAH"));
        ImGui::Text("%s (V/W): %.2f %s", LOC("Питома цінність", "Value Density"),
                    itemHere->getValueDensity(), LOC("грн/кг", "UAH/kg"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
      } else if (isHoveringRouteLine) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.9f, 1.0f),
                           "%s", LOC("Лінія маршруту (TSP)", "Planned Route (TSP)"));
        ImGui::Separator();
        ImGui::TextUnformatted(
            LOC("Розрахована лінія оптимального шляху робітника",
                "Calculated optimal path trajectory for worker"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
      } else if (itemHere.has_value() && isCollected) {
        if (itemHere->category == core::ItemCategory::LightScrap) {
          ImGui::TextColored(ImVec4(0.20f, 0.41f, 1.00f, 1.0f),
                             "%s: %s", LOC("Дрібний брухт", "Small Scrap"), itemHere->name.c_str());
        } else if (itemHere->category == core::ItemCategory::MediumScrap) {
          ImGui::TextColored(ImVec4(1.00f, 1.00f, 0.00f, 1.0f),
                             "%s: %s", LOC("Побутовий брухт", "Household Scrap"), itemHere->name.c_str());
        } else {
          ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
                             "%s: %s", LOC("Дорогоцінний брухт", "Precious Scrap"), itemHere->name.c_str());
        }
        ImGui::Separator();
        ImGui::Text("%s: %s", LOC("Категорія", "Category"),
                    core::itemCategoryToString(itemHere->category).c_str());
        ImGui::Text("%s: %d %s", LOC("Вага", "Weight"), itemHere->weight, LOC("кг", "kg"));
        ImGui::Text("%s: %d %s", LOC("Цінність", "Value"), itemHere->value, LOC("грн", "UAH"));
        ImGui::Text("%s (V/W): %.2f %s", LOC("Питома цінність", "Value Density"),
                    itemHere->getValueDensity(), LOC("грн/кг", "UAH/kg"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.55f, 1.0f),
                           "%s", LOC("Вже підібрано робітником у рюкзак", "Already collected into backpack"));
      } else if (pCell && pCell->getType() == core::CellType::Start) {
        ImGui::TextColored(ImVec4(0.95f, 0.55f, 0.1f, 1.0f), "%s", LOC("Шлюз (Старт)", "Airlock (Start)"));
        ImGui::Separator();
        ImGui::TextUnformatted(
            LOC("Початкова точка, звідки робітник починає місію",
                "Starting point where worker begins mission"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
      } else if (pCell && pCell->getType() == core::CellType::Exit) {
        if (isExitDisabled) {
          ImGui::TextColored(ImVec4(0.85f, 0.35f, 0.35f, 1.0f),
                             "%s", LOC("Вихід (Недоступний / Заблокований)", "Exit (Unavailable / Locked)"));
          ImGui::Separator();
          ImGui::TextUnformatted(
              LOC("Режим евакуації: 'A -> A (База)'. Шлюз законсервовано, вихід лише через початковий шлюз (A).",
                  "Evacuation mode: 'A -> A (Base)'. Exit locked, return only via start airlock (A)."));
          ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
        } else {
          ImGui::TextColored(ImVec4(0.18f, 0.80f, 0.44f, 1.0f),
                             "%s", LOC("Вихід (Евакуація)", "Exit (Evacuation)"));
          ImGui::Separator();
          ImGui::TextUnformatted(
              LOC("Шлюз евакуації, куди робітник прямує після збору брухту",
                  "Extraction hatch where worker heads after collecting scrap"));
          ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
        }
      } else if (pCell && pCell->getType() == core::CellType::Mud) {
        ImGui::TextColored(ImVec4(0.85f, 0.65f, 0.35f, 1.0f),
                           "%s", LOC("Багнюка / Болото / Вода", "Mud / Swamp / Water"));
        ImGui::Separator();
        ImGui::TextUnformatted(LOC("Важкопрохідна ділянка: витрата енергії +25 прямо, +35 по діагоналі",
                                   "Difficult terrain: energy cost +25 orthogonal, +35 diagonal"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
        if (showWalkedCells && pCell->isWalked()) {
          ImGui::Separator();
          ImGui::TextColored(ImVec4(0.85f, 0.65f, 0.35f, 1.0f),
                             "%s", LOC("Пройдений слід: Блакитно-коричнева пройдена болотяна клітинка",
                                       "Walked trail: Cyan-brown traversed mud cell"));
        }
      } else if (showWalkedCells && pCell && pCell->isWalked()) {
        ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.95f, 1.0f), "%s", LOC("Пройдений слід", "Walked Trail"));
        ImGui::Separator();
        ImGui::TextUnformatted(LOC("Клітинка, яку робітник уже пройшов", "Cell already traversed by worker"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
      } else if (pCell && pCell->getType() == core::CellType::Wall) {
        if ((cellPoint.x == cellBeforeStart.x &&
             cellPoint.y == cellBeforeStart.y) ||
            (cellPoint.x == cellAfterExit.x &&
             cellPoint.y == cellAfterExit.y)) {
          return;
        }
        ImGui::TextColored(ImVec4(0.6f, 0.65f, 0.75f, 1.0f), "%s", LOC("Стіна комплексу", "Complex Wall"));
        ImGui::Separator();
        ImGui::TextUnformatted(LOC("Непрохідна металева перешкода лабіринту", "Impassable metallic maze obstacle"));
        ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
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
                             "%s", LOC("Кімната комплексу", "Complex Room"));
          ImGui::Separator();
          ImGui::Text("%s (#%d)", LOC("Сектор кімнати", "Room Sector"), inRoom->id);
          ImGui::TextUnformatted(
              LOC("Вільний прохід комплексу (витрата: 10 прямо, 14 по діагоналі)",
                  "Free interior floor (cost: 10 orthogonal, 14 diagonal)"));
          ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
        } else {
          ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f),
                             "%s", LOC("Коридор комплексу", "Complex Corridor"));
          ImGui::Separator();
          ImGui::TextUnformatted(
              LOC("Вільний коридор комплексу (витрата: 10 прямо, 14 по діагоналі)",
                  "Free corridor path (cost: 10 orthogonal, 14 diagonal)"));
          ImGui::Text("%s: [%d, %d]", LOC("Координати", "Coordinates"), cellPoint.x, cellPoint.y);
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

  static constexpr float DEFAULT_MISSION_PANEL_WIDTH = 345.0f;
  float missionPanelWidth_ = DEFAULT_MISSION_PANEL_WIDTH;
  float savedMissionPanelBaseWidth_ = DEFAULT_MISSION_PANEL_WIDTH;
  bool isDraggingMissionHandle_ = false;
  float missionDragAccumX_ = 0.0f;
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
  mutable float lastMissionPanelBottomY_ = -1.0f;
  mutable ImVec2 lastManualCardPos_{0.0f, 0.0f};
  mutable ImVec2 lastManualCardSize_{0.0f, 0.0f};
  bool hudIntroActive_ = false;
  float hudIntroProgress_ = 1.0f;
  float hudAttachProgress_ = 1.0f;

  bool isTourActive_ = false;
  int tourCurrentStep_ = 0;
  int lastTourStep_ = -1;
  float escHoldTimer_ = 0.0f;
  static constexpr float ESC_HOLD_DURATION = 2.0f;
  bool ignoreEscUntilRelease_ = false;
  ImVec2 currentSpotMin_{0.0f, 0.0f};
  ImVec2 currentSpotMax_{0.0f, 0.0f};
  ImVec2 currentCardPos_{20.0f, 64.0f};
  float tourTextAlpha_ = 1.0f;
  float tourDimAlpha_ = 0.0f;
  float sidebarScrollTargetY_ = -1.0f;
  bool hasValidSpotlight_ = false;
  float lastRenderedLegendBottomY_ = 0.0f;
  float catalogBtnLocalY_ = 0.0f;
  float collectedBtnLocalY_ = 0.0f;
  ImVec2 tourBlockMin_[20]{};
  ImVec2 tourBlockMax_[20]{};
  float tourBlockLocalY_[20]{};
  float tourBlockH_[20]{};
  bool resetSidebarScrollToTop_ = false;

  bool sectionsOpen_[5] = {true, true, true, true, true};
  bool hasLoadedSectionStates_ = false;
  ImFont *boldFont_ = nullptr;
};

} // namespace ui
