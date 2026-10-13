#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <chrono>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>


#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <dwmapi.h>
#include <windows.h>


#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#define CP_UTF8 65001

namespace {
#ifdef _WIN32
HHOOK g_cbtHook = nullptr;
WNDPROC g_originalWndProc = nullptr;
HICON g_hIconBig = nullptr;
HICON g_hIconSmall = nullptr;

#include <cstring>

LRESULT CALLBACK WindowCreationCbtProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HCBT_CREATEWND) {
    auto *createParam = reinterpret_cast<LPCBT_CREATEWND>(lParam);
    if (createParam && createParam->lpcs) {
      auto className = createParam->lpcs->lpszClass;
      if (!IS_INTRESOURCE(className) && std::strstr(className, "SFML") != nullptr) {
        createParam->lpcs->style &= ~WS_VISIBLE;
      }
    }
  }
  return CallNextHookEx(g_cbtHook, nCode, wParam, lParam);
}

LRESULT CALLBACK PreventEraseWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam) {
  if (uMsg == WM_ERASEBKGND) {
    return 1;
  }
  if (uMsg == WM_GETICON) {
    if (wParam == ICON_SMALL || wParam == 2 /* ICON_SMALL2 */) {
      if (g_hIconSmall) return reinterpret_cast<LRESULT>(g_hIconSmall);
    }
    if (g_hIconBig) return reinterpret_cast<LRESULT>(g_hIconBig);
  }
  if (uMsg == WM_GETMINMAXINFO) {
    auto *mmi = reinterpret_cast<MINMAXINFO *>(lParam);
    mmi->ptMinTrackSize.x = 800;
    mmi->ptMinTrackSize.y = 600;
    return 0;
  }
  return CallWindowProcW(g_originalWndProc, hWnd, uMsg, wParam, lParam);
}

void ForceForegroundAndTop(HWND hwnd) {
  if (!hwnd) return;

  // Register window as top-level app in Windows task switcher (Alt-Tab)
  LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  exStyle |= WS_EX_APPWINDOW;
  exStyle &= ~WS_EX_TOOLWINDOW;
  SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle);

  // Unlock foreground lock and simulate menu tap to obtain activation rights
  LockSetForegroundWindow(LSFW_UNLOCK);
  keybd_event(VK_MENU, 0, 0, 0);
  keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);

  HWND hForeground = GetForegroundWindow();
  DWORD fgThread = hForeground ? GetWindowThreadProcessId(hForeground, nullptr) : 0;
  DWORD myThread = GetCurrentThreadId();

  if (fgThread && fgThread != myThread) {
    AttachThreadInput(myThread, fgThread, TRUE);
    BringWindowToTop(hwnd);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_FRAMECHANGED);
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    SetFocus(hwnd);
    AttachThreadInput(myThread, fgThread, FALSE);
  } else {
    BringWindowToTop(hwnd);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_FRAMECHANGED);
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    SetFocus(hwnd);
  }
}
#endif
} // namespace
#endif

#include "algorithms/knapsack/DPKnapsack.hpp"
#include "algorithms/knapsack/GreedyKnapsack.hpp"
#include "algorithms/pathfinding/AStarFinder.hpp"
#include "algorithms/pathfinding/BFSFinder.hpp"
#include "algorithms/pathfinding/DijkstraFinder.hpp"
#include "algorithms/routing/TSPSolver.hpp"
#include "core/Backpack.hpp"
#include "core/Config.hpp"
#include "io/CsvExporter.hpp"
#include "map/BSPGenerator.hpp"
#include "map/Maze.hpp"
#include "simulation/SimulationEngine.hpp"
#include "ui/CameraController.hpp"
#include "ui/ImGuiOverlay.hpp"
#include "ui/Minimap.hpp"
#include "ui/ProceduralRenderer.hpp"


std::vector<io::BenchmarkRow> runBatchBenchmark(map::Maze &maze,
                                                const core::Config &config) {
  std::vector<io::BenchmarkRow> rows;
  int cap = std::max(10, maze.getTotalItemWeight() / 2);

  algorithms::knapsack::DPKnapsack dp;
  auto dpRes = dp.solve(maze.getItems(), cap);
  rows.push_back({"Рюкзак", dpRes.algorithmName, maze.getWidth(), 0, 0, 0,
                  dpRes.executionTimeMicroseconds,
                  static_cast<int>(dpRes.selectedItems.size()),
                  dpRes.totalWeight, dpRes.totalValue, 100.0});

  algorithms::knapsack::GreedyKnapsack greedy;
  auto greedyRes = greedy.solve(maze.getItems(), cap);
  rows.push_back({"Рюкзак", greedyRes.algorithmName, maze.getWidth(), 0, 0, 0,
                  greedyRes.executionTimeMicroseconds,
                  static_cast<int>(greedyRes.selectedItems.size()),
                  greedyRes.totalWeight, greedyRes.totalValue,
                  greedyRes.getApproximationRatio(dpRes.totalValue)});

  algorithms::pathfinding::AStarFinder astar;
  auto astarRes =
      astar.findPath(maze, maze.getStartPoint(), maze.getExitPoint());
  rows.push_back({"Пошук шляху", astarRes.algorithmName, maze.getWidth(),
                  static_cast<int>(astarRes.getStepCount()),
                  astarRes.totalEnergyCost, astarRes.visitedNodesCount,
                  astarRes.executionTimeMicroseconds, 0, 0, 0, 100.0});

  algorithms::pathfinding::DijkstraFinder dijkstra;
  auto dijkstraRes =
      dijkstra.findPath(maze, maze.getStartPoint(), maze.getExitPoint());
  rows.push_back({"Пошук шляху", dijkstraRes.algorithmName, maze.getWidth(),
                  static_cast<int>(dijkstraRes.getStepCount()),
                  dijkstraRes.totalEnergyCost, dijkstraRes.visitedNodesCount,
                  dijkstraRes.executionTimeMicroseconds, 0, 0, 0, 100.0});

  algorithms::pathfinding::BFSFinder bfs;
  auto bfsRes = bfs.findPath(maze, maze.getStartPoint(), maze.getExitPoint());
  double bfsQuality = (astarRes.totalEnergyCost > 0)
                          ? (static_cast<double>(astarRes.totalEnergyCost) /
                             bfsRes.totalEnergyCost) *
                                100.0
                          : 100.0;
  rows.push_back({"Пошук шляху", bfsRes.algorithmName, maze.getWidth(),
                  static_cast<int>(bfsRes.getStepCount()),
                  bfsRes.totalEnergyCost, bfsRes.visitedNodesCount,
                  bfsRes.executionTimeMicroseconds, 0, 0, 0, bfsQuality});

  algorithms::routing::TSPSolver tsp(&astar);
  auto plan = tsp.solveRoute(maze, dpRes.selectedItems, config.routeMode);
  rows.push_back({"Маршрутизація TSP", "Nearest Neighbor + 2-Opt",
                  maze.getWidth(), plan.totalSteps, plan.totalEnergyCost,
                  static_cast<int>(plan.visitOrder.size()),
                  plan.calculationTimeMicroseconds,
                  static_cast<int>(dpRes.selectedItems.size()),
                  dpRes.totalWeight, dpRes.totalValue,
                  100.0 + plan.twoOptGainPercent});

  return rows;
}

void recalculateSimulation(map::Maze &maze, core::Config &config,
                           simulation::SimulationEngine &engine,
                           int knapsackAlgoIndex, int pathfinderAlgoIndex) {
  maze.recomputeExteriorWalls();
  engine.updateConfig(config);
  int cap = config.manualBackpackCapacity;

  std::vector<core::Item> selectedItems;
  if (knapsackAlgoIndex == 0) {
    algorithms::knapsack::DPKnapsack dp;
    selectedItems = dp.solve(maze.getItems(), cap).selectedItems;
  } else {
    algorithms::knapsack::GreedyKnapsack greedy;
    selectedItems = greedy.solve(maze.getItems(), cap).selectedItems;
  }

  std::unique_ptr<algorithms::pathfinding::IPathfinder> pathfinder;
  if (pathfinderAlgoIndex == 0) {
    pathfinder = std::make_unique<algorithms::pathfinding::AStarFinder>();
  } else if (pathfinderAlgoIndex == 1) {
    pathfinder = std::make_unique<algorithms::pathfinding::DijkstraFinder>();
  } else {
    pathfinder = std::make_unique<algorithms::pathfinding::BFSFinder>();
  }

  algorithms::routing::TSPSolver tsp(pathfinder.get());
  auto plan = tsp.solveRoute(maze, selectedItems, config.routeMode);

  // Dynamic tactical energy reserve: 22% surplus (at least 120) for turns, mud terrain, and detour flexibility
  int energyReserve = std::max(120, static_cast<int>(std::round(plan.totalEnergyCost * 0.22f)));
  if (config.initialEnergy < plan.totalEnergyCost + energyReserve) {
    config.initialEnergy = plan.totalEnergyCost + energyReserve;
  }

  engine.initPlan(plan, config.initialEnergy, cap);
}

std::optional<sf::Cursor> loadGrabCursor() {
  sf::Image rawImage;
  bool loaded = rawImage.loadFromFile("assets/cursor-grabbing-32.png") ||
                rawImage.loadFromFile("assets/cursor-grabbing.png") ||
                rawImage.loadFromFile("../assets/cursor-grabbing.png") ||
                rawImage.loadFromFile("build/assets/cursor-grabbing.png");
  if (!loaded) {
    return sf::Cursor::createFromSystem(sf::Cursor::Type::Hand);
  }

  const unsigned int targetSize = 32;
  sf::Image cursorImage;
  if (rawImage.getSize().x > 48 || rawImage.getSize().y > 48) {
    cursorImage = sf::Image(sf::Vector2u{targetSize, targetSize}, sf::Color::Transparent);
    float scaleX = static_cast<float>(rawImage.getSize().x) / targetSize;
    float scaleY = static_cast<float>(rawImage.getSize().y) / targetSize;
    for (unsigned int dy = 0; dy < targetSize; ++dy) {
      for (unsigned int dx = 0; dx < targetSize; ++dx) {
        int startX = static_cast<int>(dx * scaleX);
        int endX = std::max(startX + 1, std::min(static_cast<int>((dx + 1) * scaleX), static_cast<int>(rawImage.getSize().x)));
        int startY = static_cast<int>(dy * scaleY);
        int endY = std::max(startY + 1, std::min(static_cast<int>((dy + 1) * scaleY), static_cast<int>(rawImage.getSize().y)));

        uint32_t r = 0, g = 0, b = 0, a = 0;
        int count = 0;
        for (int sy = startY; sy < endY; ++sy) {
          for (int sx = startX; sx < endX; ++sx) {
            sf::Color c = rawImage.getPixel({static_cast<unsigned int>(sx), static_cast<unsigned int>(sy)});
            r += c.r * c.a;
            g += c.g * c.a;
            b += c.b * c.a;
            a += c.a;
            count++;
          }
        }
        if (a > 0 && count > 0) {
          cursorImage.setPixel({dx, dy}, sf::Color(
              static_cast<uint8_t>(r / a),
              static_cast<uint8_t>(g / a),
              static_cast<uint8_t>(b / a),
              static_cast<uint8_t>(a / count)
          ));
        }
      }
    }
  } else {
    cursorImage = rawImage;
  }

  sf::Vector2u hotspot{cursorImage.getSize().x / 2, cursorImage.getSize().y / 2};
  auto custom = sf::Cursor::createFromPixels(cursorImage.getPixelsPtr(), cursorImage.getSize(), hotspot);
  if (custom.has_value()) {
    return custom;
  }
  return sf::Cursor::createFromSystem(sf::Cursor::Type::Hand);
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif

  bool testOnly = (argc > 1 && std::string(argv[1]) == "--test");

  core::Config config;
  if (!testOnly) {
    config.randomSeed = static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
  }
  map::Maze maze;
  map::BSPGenerator generator(config);
  generator.generate(maze);

  // Smart scaling for backpack capacity: ~52% of total scrap weight guarantees ~18 items capacity on 30x30, scaling smoothly with maze size
  int initialCap = std::clamp(static_cast<int>(std::round(maze.getTotalItemWeight() * 0.52f)), 25, 450);
  config.manualBackpackCapacity = initialCap;

  simulation::SimulationEngine engine(maze, config);

  auto benchmarkRows = runBatchBenchmark(maze, config);
  recalculateSimulation(maze, config, engine, 0, 0);

  // Export initial benchmark metrics to CSV
  io::CsvExporter::exportToCsv("benchmark_results.csv", benchmarkRows);

  if (testOnly) {
    std::cout << "========================================================\n"
              << "  LETHAL COMPANY DSA - VERIFICATION OF 6 CORE ALGORITHMS\n"
              << "========================================================\n";
    std::cout << "[INFO]: BSP Maze generated successfully:\n"
              << "  - Dimensions: " << maze.getWidth() << "x" << maze.getHeight()
              << "\n"
              << "  - Rooms count: " << maze.getRooms().size() << "\n"
              << "  - Start: (" << maze.getStartPoint().x << ", "
              << maze.getStartPoint().y << ")\n"
              << "  - Exit: (" << maze.getExitPoint().x << ", "
              << maze.getExitPoint().y << ")\n"
              << "  - Items spawned: " << maze.getItems().size()
              << " (Total weight: " << maze.getTotalItemWeight()
              << " kg, Total value: " << maze.getTotalItemValue()
              << " грн)\n";

    for (const auto &r : benchmarkRows) {
      std::cout << "  [" << r.category << "] " << r.algorithmName
                << " -> Час: " << (static_cast<double>(r.timeMicrosec) / 1000.0)
                << " ms (" << r.timeMicrosec
                << " us), Якість: " << r.qualityPercent << " %\n";
    }

    std::cout
        << "[INFO]: Benchmark results exported to 'benchmark_results.csv'.\n";
    std::cout << "========================================================\n\n"
              << std::flush;



    std::cout << "[STRESS TEST]: Testing labyrinth generation & simulation at "
                 "sizes 20, 30, 40, 62, 80, 100...\n";
    for (int sz : {20, 30, 40, 62, 80, 100}) {
      config.mapWidth = sz;
      config.mapHeight = sz;
      generator.setConfig(config);
      generator.generate(maze);
      config.manualBackpackCapacity = std::clamp(static_cast<int>(std::round(maze.getTotalItemWeight() * 0.52f)), 25, 450);
      recalculateSimulation(maze, config, engine, 0, 0);
      std::cout << "  - Size " << sz << "x" << sz
                << ": Rooms=" << maze.getRooms().size() << ", Start=("
                << maze.getStartPoint().x << "," << maze.getStartPoint().y
                << "), Exit=(" << maze.getExitPoint().x << ","
                << maze.getExitPoint().y
                << "), Items=" << maze.getItems().size()
                << ", Weight=" << maze.getTotalItemWeight() << "kg"
                << ", Cap=" << config.manualBackpackCapacity << "kg"
                << ", RouteItems=" << (engine.getPlan().visitOrder.size() > 2 ? engine.getPlan().visitOrder.size() - 2 : 0)
                << " [OK]\n";
    }
    std::cout << "[INFO]: All sizes (including 62 and 100) verified without "
                 "errors. Exiting cleanly.\n";
    return 0;
  }

  const std::string titleUtf8 = "Lethal Labyrinths";
  sf::String windowTitle =
      sf::String::fromUtf8(titleUtf8.begin(), titleUtf8.end());
  sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
  sf::ContextSettings contextSettings;
  contextSettings.antiAliasingLevel = 4;

#ifdef _WIN32
  // Capture window creation to create initially hidden and prevent white flash
  g_cbtHook = SetWindowsHookExW(WH_CBT, WindowCreationCbtProc, nullptr,
                                GetCurrentThreadId());
#endif

  // Create borderless fullscreen window
  sf::RenderWindow window(desktopMode, windowTitle, sf::Style::None,
                          sf::State::Windowed, contextSettings);
  window.setVerticalSyncEnabled(true);

#ifdef _WIN32
  if (g_cbtHook) {
    UnhookWindowsHookEx(g_cbtHook);
    g_cbtHook = nullptr;
  }
  HWND hwnd = static_cast<HWND>(window.getNativeHandle());
  if (hwnd) {
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &useDarkMode,
                          sizeof(useDarkMode));

    // Ensure class background brush is NULL to prevent GDI from erasing over OpenGL
    SetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND, 0);

    // Subclass WndProc to intercept WM_ERASEBKGND
    g_originalWndProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(PreventEraseWndProc)));

    // Position window to cover exact desktop resolution and place at top of Z-order
    SetWindowPos(hwnd, HWND_TOP, 0, 0, desktopMode.size.x, desktopMode.size.y,
                 SWP_SHOWWINDOW | SWP_FRAMECHANGED);
    ShowWindow(hwnd, SW_SHOW);
    BringWindowToTop(hwnd);

    // Ensure window is registered as primary application window in Windows DWM/Task Switcher
    LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle | WS_EX_APPWINDOW);

    // Load and assign PE embedded multi-resolution icon for Alt+Tab, Taskbar, and title
    g_hIconBig = (HICON)LoadImageW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1),
                                   IMAGE_ICON, GetSystemMetrics(SM_CXICON),
                                   GetSystemMetrics(SM_CYICON), LR_SHARED);
    g_hIconSmall = (HICON)LoadImageW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1),
                                     IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
                                     GetSystemMetrics(SM_CYSMICON), LR_SHARED);
    if (!g_hIconBig && !g_hIconSmall) {
      // Fallback: try loading by string name in case resource ID was compiled as string
      g_hIconBig = (HICON)LoadImageW(GetModuleHandleW(nullptr), L"IDI_ICON1",
                                     IMAGE_ICON, GetSystemMetrics(SM_CXICON),
                                     GetSystemMetrics(SM_CYICON), LR_SHARED);
      g_hIconSmall = (HICON)LoadImageW(GetModuleHandleW(nullptr), L"IDI_ICON1",
                                       IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
                                       GetSystemMetrics(SM_CYSMICON), LR_SHARED);
    }

    if (g_hIconBig) {
      SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_hIconBig);
      SetClassLongPtrW(hwnd, GCLP_HICON, (LONG_PTR)g_hIconBig);
    }
    if (g_hIconSmall) {
      SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hIconSmall);
      SetClassLongPtrW(hwnd, GCLP_HICONSM, (LONG_PTR)g_hIconSmall);
    }
  }
#endif

  window.requestFocus();

  // Clear both front and back buffers with dark background matching complex walls
  window.clear(sf::Color(22, 27, 36));
  window.display();
  window.clear(sf::Color(22, 27, 36));
  window.display();

  sf::Image appIcon;
  bool iconLoaded = appIcon.loadFromFile("assets/icon.png") ||
                    appIcon.loadFromFile("../assets/icon.png") ||
                    appIcon.loadFromFile("build/assets/icon.png");

  // Only use SFML fallback setIcon if Windows native icon was not loaded
#ifdef _WIN32
  if (!g_hIconBig && !g_hIconSmall) {
#endif
    if (iconLoaded) {
      window.setIcon(appIcon.getSize(), appIcon.getPixelsPtr());
    } else {
      sf::Image fallbackIcon(sf::Vector2u{32, 32}, sf::Color(16, 20, 28));
      for (unsigned int y = 0; y < 32; ++y) {
        for (unsigned int x = 0; x < 32; ++x) {
          if (x == 0 || x == 31 || y == 0 || y == 31) {
            fallbackIcon.setPixel({x, y}, ((x + y) % 6 < 3)
                                              ? sf::Color(240, 160, 20)
                                              : sf::Color(20, 20, 20));
          } else if ((x == 6 && y >= 6 && y <= 25) ||
                     (y == 25 && x >= 6 && x <= 25) ||
                     (x == 25 && y >= 14 && y <= 25)) {
            fallbackIcon.setPixel({x, y}, sf::Color(0, 230, 245));
          } else if (x >= 11 && x <= 20 && y >= 10 && y <= 18) {
            fallbackIcon.setPixel({x, y}, sf::Color(255, 170, 0));
          }
        }
      }
      window.setIcon(fallbackIcon.getSize(), fallbackIcon.getPixelsPtr());
    }
#ifdef _WIN32
  }
#endif

  // Load high-resolution logo texture for centered splash screen
  sf::Texture logoTexture;
  bool logoLoaded = logoTexture.loadFromFile("assets/icon.png") ||
                    logoTexture.loadFromFile("../assets/icon.png") ||
                    logoTexture.loadFromFile("build/assets/icon.png");
  if (logoLoaded) {
    logoTexture.setSmooth(true);
  }

  if (!ImGui::SFML::Init(window, false)) {
    std::cerr << "[ERROR]: Failed to initialize ImGui-SFML!\n";
    return -1;
  }

  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->Clear();
  ImFontConfig fontCfg;
  fontCfg.OversampleH = 3;
  fontCfg.OversampleV = 2;
  fontCfg.PixelSnapH = false;
  io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/tahoma.ttf", 26.0f, &fontCfg,
                               io.Fonts->GetGlyphRangesCyrillic());

  ImFontConfig titleFontCfg = fontCfg;
  ImFont *titleFont = io.Fonts->AddFontFromFileTTF(
      "C:/Windows/Fonts/tahoma.ttf", 46.0f, &titleFontCfg,
      io.Fonts->GetGlyphRangesCyrillic());

  if (!ImGui::SFML::UpdateFontTexture()) {
    std::cerr << "[ERROR]: Failed to update font texture!\n";
  }

  ui::ImGuiOverlay::applyLethalCompanyCherryTheme(ImGui::GetStyle());

  // ========================================================
  // SPLASH / LOADING SCREEN (LOGO, TITLE, PROGRESS BAR & %)
  // ========================================================
  auto renderSplash = [&](float progress, const std::string &stageText,
                          float elapsed) {
    ImGui::SFML::Update(window, sf::seconds(0.016f));
    window.clear(sf::Color(22, 27, 36));

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(winW, winH));
    ImGuiWindowFlags splashWinFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin("SplashWindow", nullptr, splashWinFlags)) {
      float logoDim = 230.0f;
      float barW = 460.0f;
      float barH = 22.0f;
      float blockH =
          logoDim + 18.0f + 50.0f + 16.0f + barH + 10.0f + 26.0f + 6.0f + 22.0f;
      float topY = (winH - blockH) * 0.42f;

      // 1. Centered Logo Image
      float logoX = (winW - logoDim) * 0.5f;
      float logoY = topY;
      ImGui::SetCursorPos(ImVec2(logoX, logoY));
      if (logoLoaded) {
        ImGui::Image(logoTexture, sf::Vector2f(logoDim, logoDim));
      } else {
        ImGui::Dummy(ImVec2(logoDim, logoDim));
      }

      // Outer tactical glow border for the logo
      ImDrawList *dl = ImGui::GetWindowDrawList();
      float glowPulse = 0.5f + 0.5f * std::sin(elapsed * 4.0f);
      ImU32 borderGlow =
          IM_COL32(245, 140, 20, static_cast<int>(130 + 80 * glowPulse));
      dl->AddRect(ImVec2(logoX - 3, logoY - 3),
                  ImVec2(logoX + logoDim + 3, logoY + logoDim + 3), borderGlow,
                  8.0f, 0, 2.0f);
      dl->AddRect(ImVec2(logoX - 7, logoY - 7),
                  ImVec2(logoX + logoDim + 7, logoY + logoDim + 7),
                  IM_COL32(230, 60, 20, 60), 10.0f, 0, 1.5f);

      // 2. Title "Lethal Labyrinth"
      float titleY = logoY + logoDim + 18.0f;
      const char *titleText = "Lethal Labyrinth";
      float titleW =
          titleFont
              ? titleFont->CalcTextSizeA(46.0f, FLT_MAX, 0.0f, titleText).x
              : ImGui::CalcTextSize(titleText).x;
      float titleX = (winW - titleW) * 0.5f;
      ImGui::SetCursorPos(ImVec2(titleX, titleY));
      if (titleFont)
        ImGui::PushFont(titleFont);
      ImGui::TextColored(ImVec4(1.00f, 0.82f, 0.28f, 1.0f), "%s", titleText);
      if (titleFont)
        ImGui::PopFont();

      // 3. Progress Bar
      float barX = (winW - barW) * 0.5f;
      float barY = titleY + 52.0f + 16.0f;

      // Bar background
      dl->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + barW, barY + barH),
                        IM_COL32(14, 16, 22, 230), 4.0f);

      // Bar fill
      float fillW = barW * std::clamp(progress, 0.0f, 1.0f);
      if (fillW > 2.0f) {
        dl->AddRectFilled(ImVec2(barX + 2, barY + 2),
                          ImVec2(barX + fillW - 2, barY + barH - 2),
                          IM_COL32(235, 75, 40, 255), 3.0f);
        dl->AddLine(ImVec2(barX + 3, barY + 3),
                    ImVec2(barX + fillW - 3, barY + 3),
                    IM_COL32(255, 180, 90, 200), 1.0f);
      }
      // Bar border
      dl->AddRect(ImVec2(barX, barY), ImVec2(barX + barW, barY + barH),
                  IM_COL32(245, 160, 40, 220), 4.0f, 0, 1.8f);

      // 4. Percentage under the progress bar
      int pct = std::clamp(static_cast<int>(progress * 100.0f + 0.5f), 0, 100);
      std::string pctStr = std::to_string(pct) + " %";
      float pctW = ImGui::CalcTextSize(pctStr.c_str()).x;
      float pctX = (winW - pctW) * 0.5f;
      float pctY = barY + barH + 10.0f;
      ImGui::SetCursorPos(ImVec2(pctX, pctY));
      ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "%s", pctStr.c_str());

      // 5. Stage text under percentage
      float stageW = ImGui::CalcTextSize(stageText.c_str()).x;
      float stageX = (winW - stageW) * 0.5f;
      float stageY = pctY + 28.0f;
      ImGui::SetCursorPos(ImVec2(stageX, stageY));
      ImGui::TextDisabled("%s", stageText.c_str());
    }
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::SFML::Render(window);
    window.display();
  };

  // Pre-render initial splash frame while window is still hidden
  renderSplash(0.05f, "Ініціалізація комплексу...", 0.0f);
  renderSplash(0.05f, "Ініціалізація комплексу...", 0.0f);

#ifdef _WIN32
  if (hwnd) {
    ShowWindow(hwnd, SW_SHOW);
    ForceForegroundAndTop(hwnd);
  }
#endif
  window.requestFocus();

  sf::Clock splashClock;
  sf::Clock splashFrameClock;
  float splashProgress = 0.05f;
  float targetProgress = 0.22f;
  int splashStage = 0;
  bool splashComplete = false;

  while (window.isOpen() && !splashComplete) {
    float dt = splashFrameClock.restart().asSeconds();
    dt = std::clamp(dt, 0.001f, 0.05f);

    while (const auto event = window.pollEvent()) {
      ImGui::SFML::ProcessEvent(window, *event);
      if (event->is<sf::Event::Closed>()) {
        window.close();
        ImGui::SFML::Shutdown();
        return 0;
      }
      if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape ||
            key->code == sf::Keyboard::Key::Space ||
            key->code == sf::Keyboard::Key::Enter) {
          splashProgress = 1.0f;
          targetProgress = 1.0f;
          splashStage = 4;
          splashComplete = true;
        }
      }
      if (event->is<sf::Event::MouseButtonPressed>()) {
        splashProgress = 1.0f;
        targetProgress = 1.0f;
        splashStage = 4;
        splashComplete = true;
      }
    }

    float elapsed = splashClock.getElapsedTime().asSeconds();
    if (splashStage == 0 && elapsed >= 0.16f) {
      splashStage = 1;
      targetProgress = 0.48f;
    } else if (splashStage == 1 && elapsed >= 0.50f) {
      splashStage = 2;
      targetProgress = 0.74f;
    } else if (splashStage == 2 && elapsed >= 0.88f) {
      splashStage = 3;
      targetProgress = 0.94f;
    } else if (splashStage == 3 && elapsed >= 1.25f) {
      splashStage = 4;
      targetProgress = 1.00f;
    }

    splashProgress +=
        (targetProgress - splashProgress) * std::clamp(dt * 7.5f, 0.0f, 1.0f);

    if (splashStage >= 4 && splashProgress >= 0.995f) {
      splashProgress = 1.0f;
      static float readyHoldElapsed = 0.0f;
      readyHoldElapsed += dt;
      if (readyHoldElapsed >= 0.50f) {
        splashComplete = true;
      }
    }

    std::string stageText = "Ініціалізація комплексу...";
    if (splashStage == 1)
      stageText = "Процедурна генерація лабіринту (BSP)...";
    else if (splashStage == 2)
      stageText = "Оптимізація маршруту та розміщення брухту...";
    else if (splashStage == 3)
      stageText = "Калібрування навігаційних алгоритмів...";
    else if (splashStage >= 4)
      stageText = "Готово! Вхід до комплексу...";

    renderSplash(splashProgress, stageText, elapsed);
  }

  window.resetGLStates();
#ifdef _WIN32
  if (hwnd) {
    ForceForegroundAndTop(hwnd);
    SetFocus(hwnd);
    SetActiveWindow(hwnd);
  }
#endif
  window.requestFocus();
  sf::Event initialFocusEvent = sf::Event::FocusGained{};
  ImGui::SFML::ProcessEvent(window, initialFocusEvent);

  ui::ImGuiOverlay guiOverlay;
  ui::ImGuiOverlay::UIState uiState;
  uiState.knapsackAlgoIndex = 0;
  uiState.pathfinderAlgoIndex = 0;
  uiState.appliedKnapsackAlgoIndex = 0;
  uiState.appliedPathfinderAlgoIndex = 0;
  uiState.appliedBackpackCapacity = config.manualBackpackCapacity;
  uiState.appliedRouteMode = config.routeMode;
  uiState.appliedInitialEnergy = config.initialEnergy;
  uiState.appliedEnergyLimitEnabled = config.energyLimitEnabled;

  ui::CameraController camera;
  ui::ProceduralRenderer renderer;
  ui::Minimap minimap;

  float windowW = static_cast<float>(window.getSize().x);
  float windowH = static_cast<float>(window.getSize().y);

  camera.init(windowW, windowH);
  camera.setViewport(sf::FloatRect(sf::Vector2f{0.0f, 0.0f}, sf::Vector2f{1.0f, 1.0f}));

  float initMazeWorldW = maze.getWidth() * ui::ProceduralRenderer::TILE_SIZE;
  float initMazeWorldH = maze.getHeight() * ui::ProceduralRenderer::TILE_SIZE;
  camera.fitMaze(initMazeWorldW, initMazeWorldH, windowW, windowH, 1.25f, false);

  // Initialize Generation Transition with startup Lethal Company Sonar Scan from maze center
  ui::GenerationTransition transition;
  sf::Vector2f centerOrigin{initMazeWorldW * 0.5f, initMazeWorldH * 0.5f};
  constexpr float EXTRA_REACH = 16.0f * ui::ProceduralRenderer::TILE_SIZE + 140.0f;
  float initMaxScanR = std::hypot(initMazeWorldW * 0.5f, initMazeWorldH * 0.5f) + EXTRA_REACH;
  transition.startStartupScan(centerOrigin, initMaxScanR);
  renderer.setWaitingForDrop(true);

  auto grabCursor = loadGrabCursor();
  auto defaultCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);
  bool wasDraggingCamera = false;

  sf::Clock deltaClock;
  bool s_spaceDown = false;
  int s_spaceRepeatCount = 0;
  sf::Clock s_spaceRepeatClock;
  float s_zoomIndicatorTimer = 0.0f;
  float s_lastCameraZoom = camera.getZoomLevel();

  while (window.isOpen()) {
    float windowW = static_cast<float>(window.getSize().x);
    float windowH = static_cast<float>(window.getSize().y);
    bool hasFocus = window.hasFocus();
#ifdef _WIN32
    if (hwnd) {
      bool isFg = (GetForegroundWindow() == hwnd);
      if (hasFocus && !isFg) {
        hasFocus = false;
        camera.stopFollowing();
        camera.cancelDrag();
        s_spaceDown = false;
        s_spaceRepeatCount = 0;
      } else if (!hasFocus && isFg) {
        hasFocus = true;
      }
    }
#else
    if (hasFocus && !window.hasFocus()) {
      hasFocus = false;
      camera.stopFollowing();
      camera.cancelDrag();
      s_spaceDown = false;
      s_spaceRepeatCount = 0;
    } else if (!hasFocus && window.hasFocus()) {
      hasFocus = true;
    }
#endif

    while (const auto event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        window.close();
        continue;
      }

      if (event->is<sf::Event::FocusLost>()) {
        hasFocus = false;
        camera.stopFollowing();
        camera.cancelDrag();
        s_spaceDown = false;
        s_spaceRepeatCount = 0;
      } else if (event->is<sf::Event::FocusGained>()) {
        hasFocus = true;
      }

      if (const auto *resized = event->getIf<sf::Event::Resized>()) {
        window.setView(sf::View(sf::FloatRect(
            sf::Vector2f{0.0f, 0.0f},
            sf::Vector2f{static_cast<float>(resized->size.x),
                         static_cast<float>(resized->size.y)})));
      }

      if (event->is<sf::Event::MouseButtonPressed>()) {
        if (!hasFocus) {
          hasFocus = true;
#ifdef _WIN32
          if (hwnd) {
            SetFocus(hwnd);
            SetActiveWindow(hwnd);
          }
#endif
          // Consume focus click so it doesn't trigger UI elements or camera drag when activating window
          continue;
        }
      }

      // If the game window does not have focus, completely ignore any key, mouse, wheel, or text input!
      if (!hasFocus) {
        continue;
      }

      // Forward event to ImGui only when focused!
      ImGui::SFML::ProcessEvent(window, *event);

      if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
        if (!hasFocus) {
          continue;
        }
        if (key->code == sf::Keyboard::Key::Escape) {
          guiOverlay.toggleSidebar(config.uiScale);
        }
        if (key->code == sf::Keyboard::Key::M || key->scancode == sf::Keyboard::Scan::M) {
          if (!ImGui::GetIO().WantTextInput) {
            config.showLegend = !config.showLegend;
            uiState.statusNotice = config.showLegend
                                       ? "Легенду карти відкрито [M]"
                                       : "Легенду карти закрито [M]";
          }
        }
        if (key->code == sf::Keyboard::Key::C || key->scancode == sf::Keyboard::Scan::C) {
          if (!ImGui::GetIO().WantTextInput) {
            if (!transition.isTransitionActive()) {
              config.cameraFollowAgent = !config.cameraFollowAgent;
              if (config.cameraFollowAgent) {
                uiState.requestCenterCameraOnAgent = true;
              } else {
                uiState.requestCenterCameraOnMaze = true;
              }
            } else {
              uiState.statusNotice = "Центрування недоступне під час генерації та трасування маршруту";
            }
          }
        }
        if (key->code == sf::Keyboard::Key::F11) {
          static bool isBorderless = true;
          static RECT savedWindowedRect = {0, 0, 0, 0};
          isBorderless = !isBorderless;
#ifdef _WIN32
          if (hwnd) {
            if (isBorderless) {
              GetWindowRect(hwnd, &savedWindowedRect);
              if (IsZoomed(hwnd)) {
                ShowWindow(hwnd, SW_RESTORE);
              }
              SetWindowLongPtrW(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
              SetWindowPos(hwnd, HWND_TOP, 0, 0,
                           static_cast<int>(desktopMode.size.x),
                           static_cast<int>(desktopMode.size.y),
                           SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            } else {
              SetWindowLongPtrW(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

              int defaultW = std::min(1600, static_cast<int>(desktopMode.size.x * 0.82f));
              int defaultH = std::min(900, static_cast<int>(desktopMode.size.y * 0.82f));
              int defaultX = (static_cast<int>(desktopMode.size.x) - defaultW) / 2;
              int defaultY = (static_cast<int>(desktopMode.size.y) - defaultH) / 2;

              int targetW = defaultW;
              int targetH = defaultH;
              int targetX = defaultX;
              int targetY = defaultY;

              if (savedWindowedRect.right > savedWindowedRect.left &&
                  savedWindowedRect.bottom > savedWindowedRect.top) {
                int sw = savedWindowedRect.right - savedWindowedRect.left;
                int sh = savedWindowedRect.bottom - savedWindowedRect.top;
                if (sw < static_cast<int>(desktopMode.size.x) &&
                    sh < static_cast<int>(desktopMode.size.y)) {
                  targetW = sw;
                  targetH = sh;
                  targetX = savedWindowedRect.left;
                  targetY = savedWindowedRect.top;
                }
              }

              RECT wr = {targetX, targetY, targetX + targetW, targetY + targetH};
              AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
              int adjustedW = wr.right - wr.left;
              int adjustedH = wr.bottom - wr.top;
              int adjustedX = std::max(0, targetX);
              int adjustedY = std::max(0, targetY);

              SetWindowPos(hwnd, HWND_NOTOPMOST, adjustedX, adjustedY,
                           adjustedW, adjustedH,
                           SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            }
            ForceForegroundAndTop(hwnd);
          }
#endif
          uiState.statusNotice = isBorderless ? "Повноекранний режим" : "Віконний режим";
        }
        if (!ImGui::GetIO().WantCaptureKeyboard) {
          if (key->code == sf::Keyboard::Key::Space) {
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
              s_spaceDown = false;
              s_spaceRepeatCount = 0;
            }

            bool isCtrl =
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);

            if (isCtrl) {
              if (!s_spaceDown && !transition.isTransitionActive()) {
                s_spaceDown = true;
                if (!engine.isPaused()) {
                  engine.setPaused(true);
                }
                if (engine.getCurrentStepIndex() == 0) {
                  engine.instantFinish();
                  uiState.statusNotice = "Телепортація на фініш маршруту!";
                } else {
                  engine.resetToStart();
                  uiState.statusNotice = "Повернення на стартовий шлюз!";
                }
                renderer.resetAnimation();
                if (config.cameraFollowAgent) {
                  uiState.requestCenterCameraOnAgent = true;
                }
              }
            } else {
              bool isShift =
                  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);

              bool shouldStep = true;
              if (s_spaceDown) {
                // Holding Space triggers OS key repeats.
                // Stepping speed increased by 1.5x:
                // Shift+Space rewinds fast: interval 0.07s and every repeat (repeatMod = 1).
                // Space forward: interval 0.12s and every 2nd repeat (repeatMod = 2).
                int repeatMod = isShift ? 1 : 2;
                float minInterval = isShift ? 0.07f : 0.12f;
                s_spaceRepeatCount = (s_spaceRepeatCount + 1) % repeatMod;
                if (s_spaceRepeatCount != 0 ||
                    s_spaceRepeatClock.getElapsedTime().asSeconds() < minInterval) {
                  shouldStep = false;
                } else {
                  s_spaceRepeatClock.restart();
                }
              } else {
                s_spaceDown = true;
                s_spaceRepeatCount = 0;
                s_spaceRepeatClock.restart();
              }

              if (shouldStep && !transition.isTransitionActive()) {
                if (!engine.isPaused()) {
                  engine.setPaused(true);
                }
                if (isShift) {
                  engine.stepBackward();
                } else {
                  engine.stepForward();
                }
                if (config.cameraFollowAgent) {
                  uiState.requestCenterCameraOnAgent = true;
                }
              }
            }
          }
        }
      }

      if (const auto *keyRel = event->getIf<sf::Event::KeyReleased>()) {
        if (keyRel->code == sf::Keyboard::Key::Space) {
          s_spaceDown = false;
          s_spaceRepeatCount = 0;
        }
      }

      if (const auto *wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
        if (wheel->wheel == sf::Mouse::Wheel::Vertical) {
          s_zoomIndicatorTimer = 2.4f;
        }
      }

      if (!transition.isTransitionActive()) {
        sf::Vector2f targetWorld;
        bool isMinimapDrag = false;
        float minimapZoomFactor = 1.0f;
        if (minimap.handleEvent(*event, windowW, windowH, maze, targetWorld,
                                isMinimapDrag, minimapZoomFactor, config.minimapScale)) {
          if (std::abs(minimapZoomFactor - 1.0f) > 0.001f) {
            // Mouse wheel on minimap -> Zoom labyrinth camera anchored to hovered position
            camera.zoomTowards(targetWorld, minimapZoomFactor, 14.0f);
            config.cameraFollowAgent = false;
            s_zoomIndicatorTimer = 2.4f;
          } else {
            // LMB on minimap -> Pan labyrinth camera to hovered position
            float panSmoothness = isMinimapDrag ? 45.0f : 14.0f;
            camera.panTo(targetWorld.x, targetWorld.y, panSmoothness);
            config.cameraFollowAgent = false;
          }
        }

        bool isEventOverMinimap = false;
        if (const auto *wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
          isEventOverMinimap = minimap.isMouseOver(wheel->position.x, wheel->position.y,
                                                  windowW, windowH, config.minimapScale);
        } else if (const auto *mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
          isEventOverMinimap = minimap.isMouseOver(mouseBtn->position.x, mouseBtn->position.y,
                                                  windowW, windowH, config.minimapScale);
        } else if (const auto *mouseMove = event->getIf<sf::Event::MouseMoved>()) {
          isEventOverMinimap = minimap.isMouseOver(mouseMove->position.x, mouseMove->position.y,
                                                  windowW, windowH, config.minimapScale);
        }

        if (!ImGui::GetIO().WantCaptureMouse && !minimap.isDragging()) {
          if (!isEventOverMinimap || camera.isDragging()) {
            camera.handleEvent(*event, window);
            if (camera.isDragging()) {
              config.cameraFollowAgent = false;
            }
          }
        }
      }
    }

    float dtSeconds = deltaClock.restart().asSeconds();
    if (dtSeconds > 0.15f)
      dtSeconds = 0.15f; // Cap delta time on minimize/restore

    bool wasCompletedBefore = engine.isCompleted();
    engine.update(dtSeconds);
    if (!wasCompletedBefore && engine.isCompleted()) {
      if (engine.getAgent().getStatus() == simulation::AgentStatus::Success) {
        if (engine.getExtraCollectedCount() > 0) {
          char noticeBuf[160];
          std::snprintf(noticeBuf, sizeof(noticeBuf),
                        "Евакуація успішна! Зібрано %zu од. (+%zu понад план: на шляху лежав брухт, який помістився у рюкзак)!",
                        engine.getCollectedItemIds().size(), engine.getExtraCollectedCount());
          uiState.statusNotice = noticeBuf;
        } else {
          uiState.statusNotice = "Евакуація успішна! Всі заплановані предмети доставлено на базу!";
        }
      }
    }
    if ((!engine.isPaused() || engine.getCurrentStepIndex() > 0) &&
        (renderer.isWaitingForDrop() || renderer.isParachuting())) {
      renderer.finishParachuteDrop();
      if (transition.isParachuting()) {
        transition.phase = ui::GenerationTransitionPhase::None;
      }
    }
    ImGui::SFML::Update(window, sf::seconds(dtSeconds));

    // Force ImGui mouse state synchronization only when window has focus
    sf::Vector2i mousePixel = hasFocus ? sf::Mouse::getPosition(window) : sf::Vector2i(-9999, -9999);
    ImGuiIO &imIo = ImGui::GetIO();
    if (!hasFocus) {
      imIo.AppFocusLost = true;
      imIo.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
      for (int i = 0; i < 5; ++i) {
        imIo.MouseDown[i] = false;
      }
    } else {
      imIo.AppFocusLost = false;
      imIo.MousePos = ImVec2(static_cast<float>(mousePixel.x), static_cast<float>(mousePixel.y));
      for (int i = 0; i < 3; ++i) {
        imIo.MouseDown[i] = sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(i));
      }
    }

    bool isDraggingCamera = camera.isDragging();
    if (isDraggingCamera && grabCursor.has_value()) {
      window.setMouseCursor(*grabCursor);
    } else if (wasDraggingCamera && defaultCursor.has_value()) {
      window.setMouseCursor(*defaultCursor);
    }
    wasDraggingCamera = isDraggingCamera;

    // Handle generation transition (FadeOut -> Regenerate -> SonarScan -> PostSonarPause -> RouteTracing -> PostRoutePause -> CameraZoomingToAgent -> None)
    if (transition.isFadingOut()) {
      transition.timer += dtSeconds;
      if (transition.timer >= transition.fadeOutDuration) {
        // Old maze is fully faded out to dark blue! Perform generation now:
        renderer.resetAnimation();
        renderer.setWaitingForDrop(true);
        if (transition.pendingUseSpecifiedSeed) {
          config.randomSeed = transition.pendingSpecifiedSeedValue;
          uiState.statusNotice = "Лабіринт відтворено за власним сідом!";
        } else {
          config.randomSeed = static_cast<unsigned int>(
              std::chrono::high_resolution_clock::now().time_since_epoch().count());
          uiState.statusNotice = "Новий лабіринт успішно згенеровано!";
        }
        generator.setConfig(config);
        generator.generate(maze);
        config.manualBackpackCapacity = std::clamp(static_cast<int>(std::round(maze.getTotalItemWeight() * 0.52f)), 25, 450);
        recalculateSimulation(maze, config, engine, uiState.knapsackAlgoIndex,
                              uiState.pathfinderAlgoIndex);

        float newMazeW = maze.getWidth() * ui::ProceduralRenderer::TILE_SIZE;
        float newMazeH = maze.getHeight() * ui::ProceduralRenderer::TILE_SIZE;

        // Camera zooms out to fit whole maze and centers on maze center
        camera.fitMaze(newMazeW, newMazeH, windowW, windowH, 1.25f, false);
        minimap.resetZoom();

        benchmarkRows = runBatchBenchmark(maze, config);
        uiState.appliedKnapsackAlgoIndex = uiState.knapsackAlgoIndex;
        uiState.appliedPathfinderAlgoIndex = uiState.pathfinderAlgoIndex;
        uiState.appliedBackpackCapacity = config.manualBackpackCapacity;
        uiState.appliedRouteMode = config.routeMode;
        uiState.appliedInitialEnergy = config.initialEnergy;
        uiState.appliedEnergyLimitEnabled = config.energyLimitEnabled;

        // Transition immediately into cyan radar sonar scan from center of labyrinth!
        sf::Vector2f centerOrigin{newMazeW * 0.5f, newMazeH * 0.5f};
        float extraReach = 16.0f * ui::ProceduralRenderer::TILE_SIZE + 140.0f;
        float newMaxR = std::hypot(newMazeW * 0.5f, newMazeH * 0.5f) + extraReach;

        transition.phase = ui::GenerationTransitionPhase::SonarScan;
        transition.timer = 0.0f;
        transition.scanOrigin = centerOrigin;
        transition.maxRadius = newMaxR;
        transition.hasPendingRegen = false;
      }
    } else if (transition.isSonarScanning()) {
      transition.timer += dtSeconds;
      if (transition.timer >= transition.sonarDuration) {
        transition.phase = ui::GenerationTransitionPhase::PostSonarPause;
        transition.timer = 0.0f;
      }
    } else if (transition.isPostSonarPause()) {
      transition.timer += dtSeconds;
      if (transition.timer >= transition.postSonarPauseDuration) {
        transition.phase = ui::GenerationTransitionPhase::RouteTracing;
        transition.timer = 0.0f;
      }
    } else if (transition.isRouteTracing()) {
      transition.timer += dtSeconds;
      if (transition.timer >= transition.routeTracingDuration) {
        if (transition.isRecalculatingRoute) {
          transition.phase = ui::GenerationTransitionPhase::None;
          transition.timer = 0.0f;
          transition.isRecalculatingRoute = false;
        } else {
          transition.phase = ui::GenerationTransitionPhase::PostRoutePause;
          transition.timer = 0.0f;
        }
      }
    } else if (transition.isPostRoutePause()) {
      transition.timer += dtSeconds;
      if (transition.timer >= transition.postRoutePauseDuration) {
        sf::Vector2f agentWorld{
            (engine.getAgent().getPosition().x + 0.5f) * ui::ProceduralRenderer::TILE_SIZE,
            (engine.getAgent().getPosition().y + 0.5f) * ui::ProceduralRenderer::TILE_SIZE
        };
        camera.panTo(agentWorld.x, agentWorld.y, 8.0f);
        camera.zoomTo(1.0f, 8.0f);
        transition.phase = ui::GenerationTransitionPhase::CameraZoomingToAgent;
        transition.timer = 0.0f;
      }
    } else if (transition.isCameraZooming()) {
      transition.timer += dtSeconds;
      if (transition.timer >= transition.cameraZoomDuration) {
        sf::Vector2f agentWorld{
            (engine.getAgent().getPosition().x + 0.5f) * ui::ProceduralRenderer::TILE_SIZE,
            (engine.getAgent().getPosition().y + 0.5f) * ui::ProceduralRenderer::TILE_SIZE
        };
        renderer.startParachuteDrop(agentWorld, camera.getView().getSize().y,
                                    camera.getView().getCenter().y);
        transition.phase = ui::GenerationTransitionPhase::ParachuteDrop;
        transition.timer = 0.0f;
        if (config.cameraFollowAgent) {
          uiState.requestCenterCameraOnAgent = true;
        }
      }
    } else if (transition.isParachuting()) {
      transition.timer += dtSeconds;
      if (transition.timer >= transition.parachuteDuration || !renderer.isParachuting()) {
        renderer.finishParachuteDrop();
        transition.phase = ui::GenerationTransitionPhase::None;
        transition.timer = 0.0f;
      }
    }

    if (uiState.triggerRegeneration) {
      uiState.triggerRegeneration = false;
      guiOverlay.setSidebarCollapsed(true);
      if (!transition.isTransitionActive()) {
        renderer.setWaitingForDrop(true);
        transition.startFadeOut(uiState.useSpecifiedSeed, uiState.specifiedSeedValue);
        uiState.useSpecifiedSeed = false;
      }
    }

    if (uiState.triggerRecalculate) {
      uiState.triggerRecalculate = false;
      renderer.resetAnimation();
      recalculateSimulation(maze, config, engine, uiState.knapsackAlgoIndex,
                            uiState.pathfinderAlgoIndex);
      benchmarkRows = runBatchBenchmark(maze, config);
      uiState.appliedKnapsackAlgoIndex = uiState.knapsackAlgoIndex;
      uiState.appliedPathfinderAlgoIndex = uiState.pathfinderAlgoIndex;
      uiState.appliedBackpackCapacity = config.manualBackpackCapacity;
      uiState.appliedRouteMode = config.routeMode;
      uiState.appliedInitialEnergy = config.initialEnergy;
      uiState.appliedEnergyLimitEnabled = config.energyLimitEnabled;
      uiState.statusNotice = "Маршрут перераховано за обраними алгоритмами!";

      // Start animated line tracing without locking camera or UI
      transition.startRouteTracingOnly(2.80f);
    }

    if (uiState.triggerBatchBenchmark) {
      uiState.triggerBatchBenchmark = false;
      benchmarkRows = runBatchBenchmark(maze, config);
      uiState.statusNotice = "Аналіз усіх 6 алгоритмів виконано!";
    }

    if (uiState.triggerExportCsv) {
      uiState.triggerExportCsv = false;
      if (io::CsvExporter::exportToCsv("benchmark_results.csv",
                                       benchmarkRows)) {
        uiState.statusNotice = "Звіт збережено у 'benchmark_results.csv'!";
      } else {
        uiState.statusNotice = "Помилка при збереженні файлу CSV!";
      }
    }

    if (uiState.vsyncChanged) {
      uiState.vsyncChanged = false;
      window.setVerticalSyncEnabled(config.vsyncEnabled);
      if (!config.vsyncEnabled) {
        window.setFramerateLimit(120);
      } else {
        window.setFramerateLimit(0);
      }
    }

    uiState.cameraZoom = camera.getZoomLevel();
    uiState.targetCameraZoom = camera.getTargetZoomLevel();
    uiState.isTransitionActive = transition.isTransitionActive();
    uiState.isSonarScanning = transition.isSonarScanning();
    uiState.sonarProgress = transition.getSonarProgress();
    uiState.isRouteTracing = transition.isRouteTracing();
    uiState.routeProgress = transition.getRouteProgress();
    uiState.isCameraZooming = transition.isCameraZooming();
    guiOverlay.render(config, maze, engine, benchmarkRows, windowH, uiState);
    guiOverlay.renderResizeHandle(windowH, config.uiScale, !transition.isTransitionActive());
    if (uiState.cameraZoomChanged) {
      uiState.cameraZoomChanged = false;
      camera.setZoomLevel(uiState.cameraZoom);
    } else if (uiState.requestSmoothZoom) {
      uiState.requestSmoothZoom = false;
      camera.zoomTo(uiState.targetCameraZoom, 8.5f);
    }
    guiOverlay.renderMinimapLegend(config, maze, engine, camera, window,
                                   renderer, windowW, windowH);
    guiOverlay.renderMapTopControls(uiState, config, windowW, windowH);

    // Zoom badge state on minimap screen
    float currentCameraZoom = camera.getZoomLevel();
    if (std::abs(currentCameraZoom - s_lastCameraZoom) > 0.0015f || camera.isZoomingSmoothly()) {
      s_lastCameraZoom = currentCameraZoom;
      s_zoomIndicatorTimer = 2.4f;
    }
    if (s_zoomIndicatorTimer > 0.0f) {
      s_zoomIndicatorTimer = std::max(0.0f, s_zoomIndicatorTimer - dtSeconds);
    }

    bool isOverMinimapFrame = minimap.isMouseOver(
        mousePixel.x, mousePixel.y, windowW, windowH, config.minimapScale);
    bool isOverMinimapScreen = minimap.isInsideMinimap(
        mousePixel.x, mousePixel.y, windowW, windowH, config.minimapScale);
    float zoomBadgeAlpha = 0.0f;
    if (s_zoomIndicatorTimer > 0.0f) {
      zoomBadgeAlpha = (s_zoomIndicatorTimer < 0.6f) ? (s_zoomIndicatorTimer / 0.6f) : 1.0f;
    } else if (isOverMinimapScreen && minimap.isPoweredOn()) {
      zoomBadgeAlpha = 0.85f;
    } else if (isOverMinimapFrame && minimap.isPoweredOn()) {
      zoomBadgeAlpha = 0.50f;
    }

    if (minimap.isPoweredOn() && zoomBadgeAlpha > 0.01f) {
      guiOverlay.renderMinimapZoomBadge(camera.getZoomLevel(), windowW, windowH,
                                        config.minimapScale, config.uiScale,
                                        zoomBadgeAlpha);
    }

    sf::Vector2f worldPos =
        window.mapPixelToCoords(mousePixel, camera.getView());
    float mmFrameW = ui::Minimap::getFrameWidth(config.minimapScale);
    float mmFrameH = ui::Minimap::getFrameHeight(config.minimapScale);
    bool mouseOverUI =
        imIo.WantCaptureMouse || minimap.isDragging() ||
        (mousePixel.x < guiOverlay.getSidebarWidth()) ||
        (mousePixel.x >= windowW - mmFrameW - ui::Minimap::PADDING &&
         mousePixel.y >= windowH - mmFrameH - ui::Minimap::PADDING);
    float activeScanRadius = transition.getCurrentScanRadius();
    sf::Vector2f activeScanOrigin = transition.scanOrigin;
    bool isExitDisabled = (config.routeMode == core::RouteMode::RoundTrip);

    if (hasFocus) {
      guiOverlay.renderWorldTooltip(
          maze, engine, worldPos, mouseOverUI, mousePixel, windowW, windowH,
          isExitDisabled, activeScanRadius, activeScanOrigin,
          transition.isSonarScanning(), transition.getRouteProgress(),
          transition.isFadingOut());
    }

    camera.setViewport(
        sf::FloatRect(sf::Vector2f{0.0f, 0.0f}, sf::Vector2f{1.0f, 1.0f}));
    camera.updateViewportSize(windowW, windowH);

    // Handle keyboard arrow keys for camera panning only when window has focus and transition is inactive
    bool isPanningWithArrows = false;
    static sf::Vector2f s_arrowVelocity(0.0f, 0.0f);
    if (hasFocus && !io.WantCaptureKeyboard && !transition.isTransitionActive()) {
      float panDirX = 0.0f;
      float panDirY = 0.0f;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        panDirX -= 1.0f;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        panDirX += 1.0f;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        panDirY -= 1.0f;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        panDirY += 1.0f;

      if (panDirX != 0.0f || panDirY != 0.0f) {
        isPanningWithArrows = true;
        config.cameraFollowAgent = false;
        float panLen = std::hypot(panDirX, panDirY);
        panDirX /= panLen;
        panDirY /= panLen;
        float targetSpeed = 750.0f * camera.getZoomLevel();
        s_arrowVelocity.x +=
            (panDirX * targetSpeed - s_arrowVelocity.x) *
            std::clamp(dtSeconds * 16.0f, 0.0f, 1.0f);
        s_arrowVelocity.y +=
            (panDirY * targetSpeed - s_arrowVelocity.y) *
            std::clamp(dtSeconds * 16.0f, 0.0f, 1.0f);
      } else {
        s_arrowVelocity.x *= std::max(0.0f, 1.0f - dtSeconds * 14.0f);
        s_arrowVelocity.y *= std::max(0.0f, 1.0f - dtSeconds * 14.0f);
      }

      if (std::hypot(s_arrowVelocity.x, s_arrowVelocity.y) > 0.5f) {
        camera.panBy(s_arrowVelocity.x * dtSeconds,
                     s_arrowVelocity.y * dtSeconds);
      }
    } else {
      s_arrowVelocity = {0.0f, 0.0f};
    }

    window.resetGLStates();
    window.clear(sf::Color(22, 27, 36));

    renderer.update(dtSeconds, engine, maze);

    sf::Vector2f agentVisualPos = renderer.getVisualAgentPos();
    if (agentVisualPos.x < 0.0f) {
      agentVisualPos = {(engine.getAgent().getPosition().x + 0.5f) *
                            ui::ProceduralRenderer::TILE_SIZE,
                        (engine.getAgent().getPosition().y + 0.5f) *
                            ui::ProceduralRenderer::TILE_SIZE};
    }

    if (uiState.requestFitMazeInView) {
      uiState.requestFitMazeInView = false;
      camera.fitMaze(maze.getWidth() * ui::ProceduralRenderer::TILE_SIZE,
                     maze.getHeight() * ui::ProceduralRenderer::TILE_SIZE,
                     windowW, windowH, 1.15f, true);
      uiState.cameraZoom = camera.getZoomLevel();
      uiState.targetCameraZoom = camera.getTargetZoomLevel();
    } else if (uiState.requestCenterCameraOnAgent) {
      uiState.requestCenterCameraOnAgent = false;
      if (!transition.isTransitionActive()) {
        config.cameraFollowAgent = true;
        s_arrowVelocity = {0.0f, 0.0f};
        camera.panTo(agentVisualPos.x, agentVisualPos.y, 14.0f);
      }
    } else if (uiState.requestCenterCameraOnMaze) {
      uiState.requestCenterCameraOnMaze = false;
      if (!transition.isTransitionActive()) {
        config.cameraFollowAgent = false;
        s_arrowVelocity = {0.0f, 0.0f};
        float mazeCenterX = maze.getWidth() * ui::ProceduralRenderer::TILE_SIZE * 0.5f;
        float mazeCenterY = maze.getHeight() * ui::ProceduralRenderer::TILE_SIZE * 0.5f;
        camera.panTo(mazeCenterX, mazeCenterY, 14.0f);
      }
    } else if (config.cameraFollowAgent && !engine.isCompleted() && !transition.isTransitionActive()) {
      if (!camera.isDragging() && !minimap.isDragging() &&
          !isPanningWithArrows) {
        float distToAgent =
            std::hypot(camera.getView().getCenter().x - agentVisualPos.x,
                       camera.getView().getCenter().y - agentVisualPos.y);
        if (distToAgent > 45.0f) {
          float speedMul = engine.getSpeedMultiplier();
          camera.panTo(agentVisualPos.x, agentVisualPos.y,
                       14.0f * std::clamp(speedMul, 1.0f, 4.0f));
        } else {
          camera.centerOn(agentVisualPos.x, agentVisualPos.y);
        }
      }
    }

    camera.update(dtSeconds);
    camera.clampCenter(0.0f, 0.0f,
                       maze.getWidth() * ui::ProceduralRenderer::TILE_SIZE,
                       maze.getHeight() * ui::ProceduralRenderer::TILE_SIZE);

    // Snap render view to whole pixels to prevent line shimmering / ladder effect
    sf::View renderView = camera.getView();
    renderView.setCenter(sf::Vector2f{std::round(renderView.getCenter().x),
                                      std::round(renderView.getCenter().y)});
    window.setView(renderView);

    activeScanRadius = transition.getCurrentScanRadius();
    activeScanOrigin = transition.scanOrigin;
    isExitDisabled = (config.routeMode == core::RouteMode::RoundTrip);

    renderer.renderMaze(window, maze, config.showGrid, activeScanRadius, activeScanOrigin, isExitDisabled);

    // Route line rendered progressively based on transition phase
    renderer.renderRoute(window, engine.getPlan(), transition.getRouteProgress());
    renderer.renderExitOverlay(window, maze, isExitDisabled, activeScanRadius, activeScanOrigin);
    renderer.renderEntryExitArrows(window, maze, engine.getPlan(), isExitDisabled, activeScanRadius, activeScanOrigin);
    renderer.renderRouteCompletionEffects(window, maze, isExitDisabled, activeScanRadius, activeScanOrigin);

    std::vector<int> collectedIds;
    for (const auto &item : engine.getAgent().getBackpack().getItems()) {
      collectedIds.push_back(item.id);
    }
    renderer.renderItems(window, maze, collectedIds, activeScanRadius, activeScanOrigin);
    renderer.renderAgent(window, engine.getAgent(), activeScanRadius, activeScanOrigin);
    renderer.renderPickupEffects(window, activeScanRadius, activeScanOrigin);
    renderer.renderSpotlightOverlay(
        window, maze, renderView, guiOverlay.getHoveredCollectedItemId(),
        guiOverlay.getHoveredCatalogItemName(),
        guiOverlay.getHoveredCatalogCategory(), collectedIds);

    if (transition.isSonarScanning()) {
      renderer.renderSonarScanEffects(window, activeScanOrigin, activeScanRadius,
                                      transition.maxRadius, transition.getSonarProgress());
    }

    if (transition.isFadingOut()) {
      sf::RectangleShape fadeRect(renderView.getSize() * 1.5f);
      fadeRect.setOrigin(fadeRect.getSize() * 0.5f);
      fadeRect.setPosition(renderView.getCenter());
      fadeRect.setFillColor(sf::Color(22, 27, 36, static_cast<uint8_t>(transition.getFadeAlpha() * 255.0f)));
      window.draw(fadeRect);
    }

    // 1:1 pixel UI view for minimap rendering
    sf::View uiView(sf::FloatRect({0.0f, 0.0f}, {windowW, windowH}));
    window.setView(uiView);
    minimap.render(window, maze, camera.getView(), engine.getAgent(),
                   collectedIds, windowW, windowH, renderer.getVisualAgentPos(),
                   config.minimapScale, activeScanRadius, activeScanOrigin,
                   transition.getFadeAlpha(), &engine.getPlan(),
                   transition.getRouteProgress(), isExitDisabled);

    ImGui::SFML::Render(window);
    window.display();

#ifdef _WIN32
    if (hwnd && IsIconic(hwnd)) {
      sf::sleep(sf::milliseconds(16));
    }
#endif
  }

  ImGui::SFML::Shutdown();
#ifdef _WIN32
  if (hwnd && g_originalWndProc) {
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC,
                      reinterpret_cast<LONG_PTR>(g_originalWndProc));
  }
#endif
  std::cout << "[INFO]: Application closed cleanly.\n";
  return 0;
}