#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "core/Config.hpp"
#include "core/Types.hpp"
#include "core/Item.hpp"
#include "map/Maze.hpp"
#include "algorithms/routing/RoutePlan.hpp"
#include "simulation/SimulationEngine.hpp"
#include "io/CsvExporter.hpp"

namespace io {

class JsonHandler {
public:
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &now_c);
#else
        localtime_r(&now_c, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    // -------------------------------------------------------------
    // Save configuration to JSON (input_config.json)
    // -------------------------------------------------------------
    static bool saveInputConfig(const std::string& filepath,
                                const core::Config& config,
                                int knapsackAlgoIndex = 0,
                                int pathfinderAlgoIndex = 0) {
        try {
            nlohmann::json j;

            j["description"] = "Конфігураційний файл параметрів місії Lethal Company (DSA Course Project)";
            j["version"] = "1.0";
            j["last_modified"] = getCurrentTimestamp();

            j["map"] = {
                {"width", config.mapWidth},
                {"height", config.mapHeight},
                {"seed", config.randomSeed},
                {"min_room_size", config.minRoomSize},
                {"max_room_size", config.maxRoomSize},
                {"mud_spawn_chance", config.mudSpawnChance},
                {"extra_passage_chance", config.extraPassageChance}
            };

            j["agent"] = {
                {"backpack_capacity_kg", config.manualBackpackCapacity},
                {"backpack_limit_enabled", config.backpackLimitEnabled},
                {"company_quota_credits", config.companyQuota},
                {"initial_energy", config.initialEnergy},
                {"energy_limit_enabled", config.energyLimitEnabled},
                {"route_mode", (config.routeMode == core::RouteMode::PointAToPointB) ? "PointAToPointB" : "RoundTrip"}
            };

            std::string knapName = (knapsackAlgoIndex == 0) ? "0/1 Knapsack DP" : "Greedy (Value/Weight)";
            std::string pathName = (pathfinderAlgoIndex == 0) ? "A* (Octile)" :
                                   (pathfinderAlgoIndex == 1) ? "Dijkstra" : "BFS";

            j["algorithms"] = {
                {"knapsack_algorithm", knapName},
                {"knapsack_algo_index", knapsackAlgoIndex},
                {"pathfinder_algorithm", pathName},
                {"pathfinder_algo_index", pathfinderAlgoIndex}
            };

            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            file << j.dump(4);
            file.close();
            return true;
        } catch (...) {
            return false;
        }
    }

    // -------------------------------------------------------------
    // Load configuration from JSON (input_config.json)
    // -------------------------------------------------------------
    static bool loadInputConfig(const std::string& filepath,
                                core::Config& config,
                                int& knapsackAlgoIndex,
                                int& pathfinderAlgoIndex,
                                std::string& errorMsg) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                errorMsg = "Не вдалося відкрити файл: " + filepath;
                return false;
            }

            nlohmann::json j;
            file >> j;
            file.close();

            if (j.contains("map") && j["map"].is_object()) {
                const auto& jm = j["map"];
                if (jm.contains("width") && jm["width"].is_number_integer()) config.mapWidth = jm["width"];
                if (jm.contains("height") && jm["height"].is_number_integer()) config.mapHeight = jm["height"];
                if (jm.contains("seed") && jm["seed"].is_number_unsigned()) config.randomSeed = jm["seed"];
                if (jm.contains("min_room_size") && jm["min_room_size"].is_number_integer()) config.minRoomSize = jm["min_room_size"];
                if (jm.contains("max_room_size") && jm["max_room_size"].is_number_integer()) config.maxRoomSize = jm["max_room_size"];
                if (jm.contains("mud_spawn_chance") && jm["mud_spawn_chance"].is_number()) config.mudSpawnChance = jm["mud_spawn_chance"];
                if (jm.contains("extra_passage_chance") && jm["extra_passage_chance"].is_number()) config.extraPassageChance = jm["extra_passage_chance"];
            }

            if (j.contains("agent") && j["agent"].is_object()) {
                const auto& ja = j["agent"];
                if (ja.contains("backpack_capacity_kg") && ja["backpack_capacity_kg"].is_number_integer()) {
                    config.manualBackpackCapacity = ja["backpack_capacity_kg"];
                }
                if (ja.contains("backpack_limit_enabled") && ja["backpack_limit_enabled"].is_boolean()) {
                    config.backpackLimitEnabled = ja["backpack_limit_enabled"];
                }
                if (ja.contains("company_quota_credits") && ja["company_quota_credits"].is_number_integer()) {
                    config.companyQuota = ja["company_quota_credits"];
                }
                if (ja.contains("initial_energy") && ja["initial_energy"].is_number_integer()) {
                    config.initialEnergy = ja["initial_energy"];
                }
                if (ja.contains("energy_limit_enabled") && ja["energy_limit_enabled"].is_boolean()) {
                    config.energyLimitEnabled = ja["energy_limit_enabled"];
                }
                if (ja.contains("route_mode")) {
                    if (ja["route_mode"].is_string()) {
                        std::string modeStr = ja["route_mode"];
                        if (modeStr == "RoundTrip" || modeStr == "Base") {
                            config.routeMode = core::RouteMode::RoundTrip;
                        } else {
                            config.routeMode = core::RouteMode::PointAToPointB;
                        }
                    } else if (ja["route_mode"].is_number_integer()) {
                        config.routeMode = (ja["route_mode"] == 1) ? core::RouteMode::RoundTrip : core::RouteMode::PointAToPointB;
                    }
                }
            }

            if (j.contains("algorithms") && j["algorithms"].is_object()) {
                const auto& jalgo = j["algorithms"];
                if (jalgo.contains("knapsack_algo_index") && jalgo["knapsack_algo_index"].is_number_integer()) {
                    knapsackAlgoIndex = std::clamp(static_cast<int>(jalgo["knapsack_algo_index"]), 0, 1);
                } else if (jalgo.contains("knapsack_algorithm") && jalgo["knapsack_algorithm"].is_string()) {
                    std::string s = jalgo["knapsack_algorithm"];
                    if (s.find("Greedy") != std::string::npos || s.find("Жадіб") != std::string::npos) {
                        knapsackAlgoIndex = 1;
                    } else {
                        knapsackAlgoIndex = 0;
                    }
                }

                if (jalgo.contains("pathfinder_algo_index") && jalgo["pathfinder_algo_index"].is_number_integer()) {
                    pathfinderAlgoIndex = std::clamp(static_cast<int>(jalgo["pathfinder_algo_index"]), 0, 2);
                } else if (jalgo.contains("pathfinder_algorithm") && jalgo["pathfinder_algorithm"].is_string()) {
                    std::string s = jalgo["pathfinder_algorithm"];
                    if (s.find("Dijkstra") != std::string::npos || s.find("Дейкстр") != std::string::npos) {
                        pathfinderAlgoIndex = 1;
                    } else if (s.find("BFS") != std::string::npos || s.find("ширин") != std::string::npos) {
                        pathfinderAlgoIndex = 2;
                    } else {
                        pathfinderAlgoIndex = 0;
                    }
                }
            }

            config.clampSettings();
            return true;
        } catch (const std::exception& e) {
            errorMsg = std::string("Помилка парсингу JSON: ") + e.what();
            return false;
        } catch (...) {
            errorMsg = "Невідома помилка при зчитуванні JSON.";
            return false;
        }
    }

    // -------------------------------------------------------------
    // Ensure default config file exists on disk
    // -------------------------------------------------------------
    static void ensureDefaultInputConfig(const std::string& filepath, const core::Config& defaultConfig) {
        std::ifstream check(filepath);
        if (!check.good()) {
            saveInputConfig(filepath, defaultConfig, 0, 0);
        }
    }

    // -------------------------------------------------------------
    // Export full route & analytics report (report_route.json)
    // -------------------------------------------------------------
    static bool exportRouteReport(const std::string& filepath,
                                  const map::Maze& maze,
                                  const core::Config& config,
                                  const algorithms::routing::RoutePlan& plan,
                                  const simulation::SimulationEngine& engine,
                                  const std::vector<io::BenchmarkRow>& benchmarks,
                                  int knapsackAlgoIndex = 0,
                                  int pathfinderAlgoIndex = 0) {
        try {
            nlohmann::json j;

            j["metadata"] = {
                {"title", "Lethal Company Autonomous Agent Route Report"},
                {"timestamp", getCurrentTimestamp()},
                {"status", engine.getAgent().getStatusString()},
                {"is_completed", engine.isCompleted()},
                {"is_evacuated", engine.getAgent().getStatus() == simulation::AgentStatus::Success}
            };

            j["map"] = {
                {"width", maze.getWidth()},
                {"height", maze.getHeight()},
                {"seed", config.randomSeed},
                {"rooms_count", maze.getRooms().size()},
                {"start_point", {{"x", maze.getStartPoint().x}, {"y", maze.getStartPoint().y}}},
                {"exit_point", {{"x", maze.getExitPoint().x}, {"y", maze.getExitPoint().y}}},
                {"total_items_in_maze", maze.getItems().size()},
                {"total_item_weight_kg", maze.getTotalItemWeight()},
                {"total_item_value_credits", maze.getTotalItemValue()}
            };

            // Selected items in backpack
            const auto& backpack = engine.getAgent().getBackpack();
            nlohmann::json itemsJson = nlohmann::json::array();
            for (const auto& it : backpack.getItems()) {
                itemsJson.push_back({
                    {"id", it.id},
                    {"name", it.name},
                    {"weight_kg", it.weight},
                    {"value_credits", it.value},
                    {"density", it.getValueDensity()},
                    {"category", (it.category == core::ItemCategory::LightScrap) ? "Light" :
                                 (it.category == core::ItemCategory::MediumScrap) ? "Medium" : "Heavy"},
                    {"x", it.position.x},
                    {"y", it.position.y}
                });
            }

            std::string knapName = (knapsackAlgoIndex == 0) ? "0/1 Knapsack DP (Exact)" : "Greedy (Value/Weight)";
            double greedyQuality = 100.0;
            for (const auto& b : benchmarks) {
                if (b.category.find("Рюкзак") != std::string::npos || b.category.find("Knapsack") != std::string::npos) {
                    if (b.algorithmName.find("Greedy") != std::string::npos || b.algorithmName.find("Жадіб") != std::string::npos) {
                        greedyQuality = b.qualityPercent;
                    }
                }
            }

            j["knapsack_optimization"] = {
                {"algorithm", knapName},
                {"backpack_capacity_kg", backpack.getMaxCapacity()},
                {"collected_weight_kg", backpack.getCurrentWeight()},
                {"collected_value_credits", backpack.getCurrentValue()},
                {"company_quota_credits", config.companyQuota},
                {"quota_met", backpack.getCurrentValue() >= config.companyQuota},
                {"greedy_vs_dp_quality_percent", greedyQuality},
                {"items_count", backpack.getItems().size()},
                {"extra_opportunistic_items", engine.getExtraCollectedCount()},
                {"collected_items", itemsJson}
            };

            // Route & TSP analytics
            std::string pathName = (pathfinderAlgoIndex == 0) ? "A* (Octile Heuristic)" :
                                   (pathfinderAlgoIndex == 1) ? "Dijkstra (Uniform Cost)" : "BFS (Breadth-First Search)";

            nlohmann::json waypointsJson = nlohmann::json::array();
            for (size_t i = 0; i < plan.visitOrder.size(); ++i) {
                const auto& wp = plan.visitOrder[i];
                waypointsJson.push_back({
                    {"order_index", i},
                    {"label", wp.label},
                    {"item_id", wp.itemId},
                    {"x", wp.position.x},
                    {"y", wp.position.y},
                    {"weight", wp.itemWeight},
                    {"value", wp.itemValue}
                });
            }

            nlohmann::json trajectoryJson = nlohmann::json::array();
            for (size_t step = 0; step < plan.fullPath.size(); ++step) {
                const auto& pt = plan.fullPath[step];
                trajectoryJson.push_back({
                    {"step", step},
                    {"x", pt.x},
                    {"y", pt.y}
                });
            }

            j["route_analytics"] = {
                {"pathfinder_algorithm", pathName},
                {"route_mode", (config.routeMode == core::RouteMode::PointAToPointB) ? "Point A to Point B (Exit)" : "Round Trip (Return to Base)"},
                {"route_valid", plan.routeValid},
                {"total_steps", plan.totalSteps},
                {"total_energy_cost", plan.totalEnergyCost},
                {"initial_energy", config.initialEnergy},
                {"energy_spent", engine.getAgent().getEnergySpent()},
                {"energy_remaining", engine.getAgent().getCurrentEnergy()},
                {"steps_taken", engine.getAgent().getStepsTaken()},
                {"two_opt_gain_percent", plan.twoOptGainPercent},
                {"calculation_time_us", plan.calculationTimeMicroseconds},
                {"waypoints_count", plan.visitOrder.size()},
                {"waypoints", waypointsJson},
                {"full_trajectory", trajectoryJson}
            };

            // Batch Benchmarks
            nlohmann::json benchArray = nlohmann::json::array();
            for (const auto& b : benchmarks) {
                benchArray.push_back({
                    {"category", b.category},
                    {"algorithm", b.algorithmName},
                    {"dimension", b.mapDimension},
                    {"steps", b.steps},
                    {"energy_cost", b.energyCost},
                    {"visited_nodes", b.visitedNodes},
                    {"time_ms", static_cast<double>(b.timeMicrosec) / 1000.0},
                    {"time_us", b.timeMicrosec},
                    {"items_count", b.itemCount},
                    {"weight_kg", b.weightKg},
                    {"value_credits", b.valueCredits},
                    {"quality_percent", b.qualityPercent}
                });
            }
            j["benchmarks"] = benchArray;

            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            file << j.dump(4);
            file.close();
            return true;
        } catch (...) {
            return false;
        }
    }

    // -------------------------------------------------------------
    // Full Application Session Persistence (session_state.json)
    // -------------------------------------------------------------
    struct SessionGuiState {
        float sidebarWidth = 650.0f;
        bool sidebarCollapsed = true;
        float missionPanelWidth = 345.0f;
        bool missionPanelCollapsed = false;
        bool startupHintDismissed = true;
        float cameraZoom = 1.0f;
        float cameraCenterX = 0.0f;
        float cameraCenterY = 0.0f;
        int knapsackAlgoIndex = 0;
        int pathfinderAlgoIndex = 0;
        bool showLegend = false;
        bool catalogOpen = false;
        bool collectedOpen = false;
        bool monitorPoweredOn = true;
        float sidebarScrollY = 0.0f;
        std::vector<bool> sectionsOpen = {true, true, true, true, true};
        bool isFullscreen = true;
        bool customSeedEnabled = false;
        std::string customSeedInput = "";
        std::string language = "UA";
    };

    static bool saveSessionState(const std::string& filepath,
                                 const core::Config& config,
                                 const SessionGuiState& guiState,
                                 const map::Maze& maze,
                                 const simulation::SimulationEngine& engine,
                                 const std::vector<io::BenchmarkRow>& benchmarks) {
        try {
            nlohmann::json j;
            j["version"] = "1.0";
            j["timestamp"] = getCurrentTimestamp();
            j["has_active_session"] = (maze.getWidth() > 0 && maze.getHeight() > 0 && !maze.getItems().empty());

            // 1. Config & Display checkboxes
            j["config"] = {
                {"map_width", config.mapWidth},
                {"map_height", config.mapHeight},
                {"min_room_size", config.minRoomSize},
                {"max_room_size", config.maxRoomSize},
                {"mud_spawn_chance", config.mudSpawnChance},
                {"extra_passage_chance", config.extraPassageChance},
                {"auto_backpack_capacity", config.autoBackpackCapacity},
                {"show_grid", config.showGrid},
                {"show_optimal_route", config.showOptimalRoute},
                {"show_walked_cells", config.showWalkedCells},
                {"camera_follow_agent", config.cameraFollowAgent},
                {"vsync_enabled", config.vsyncEnabled},
                {"ui_scale", config.uiScale},
                {"minimap_scale", config.minimapScale},
                {"show_legend", config.showLegend},
                {"random_seed", config.randomSeed},
                {"company_quota", config.companyQuota},
                {"manual_backpack_capacity", config.manualBackpackCapacity},
                {"initial_energy", config.initialEnergy},
                {"energy_limit_enabled", config.energyLimitEnabled},
                {"backpack_limit_enabled", config.backpackLimitEnabled},
                {"route_mode", (config.routeMode == core::RouteMode::PointAToPointB) ? 0 : 1},
                {"speed_multiplier", config.speedMultiplier}
            };

            // 2. GUI dimensions & states
            j["gui"] = {
                {"sidebar_width", guiState.sidebarWidth},
                {"sidebar_collapsed", guiState.sidebarCollapsed},
                {"mission_panel_width", guiState.missionPanelWidth},
                {"mission_panel_collapsed", guiState.missionPanelCollapsed},
                {"startup_hint_dismissed", guiState.startupHintDismissed},
                {"camera_zoom", guiState.cameraZoom},
                {"camera_center_x", guiState.cameraCenterX},
                {"camera_center_y", guiState.cameraCenterY},
                {"knapsack_algo_index", guiState.knapsackAlgoIndex},
                {"pathfinder_algo_index", guiState.pathfinderAlgoIndex},
                {"show_legend", guiState.showLegend},
                {"catalog_open", guiState.catalogOpen},
                {"collected_open", guiState.collectedOpen},
                {"monitor_powered_on", guiState.monitorPoweredOn},
                {"sidebar_scroll_y", guiState.sidebarScrollY},
                {"sections_open", guiState.sectionsOpen},
                {"is_fullscreen", guiState.isFullscreen},
                {"custom_seed_enabled", guiState.customSeedEnabled},
                {"custom_seed_input", guiState.customSeedInput},
                {"language", guiState.language}
            };

            // 3. Maze layout, rooms, items, and cells
            nlohmann::json roomsJson = nlohmann::json::array();
            for (const auto& r : maze.getRooms()) {
                roomsJson.push_back({
                    {"id", r.id},
                    {"x", r.x},
                    {"y", r.y},
                    {"w", r.width},
                    {"h", r.height},
                    {"items", r.itemIds}
                });
            }

            nlohmann::json itemsJson = nlohmann::json::array();
            for (const auto& it : maze.getItems()) {
                itemsJson.push_back({
                    {"id", it.id},
                    {"name", it.name},
                    {"weight", it.weight},
                    {"value", it.value},
                    {"cat", static_cast<int>(it.category)},
                    {"x", it.position.x},
                    {"y", it.position.y}
                });
            }

            nlohmann::json cellsJson = nlohmann::json::array();
            for (int y = 0; y < maze.getHeight(); ++y) {
                for (int x = 0; x < maze.getWidth(); ++x) {
                    const auto& cell = maze.at(x, y);
                    cellsJson.push_back({
                        static_cast<int>(cell.getType()),
                        cell.isWalked() ? 1 : 0,
                        cell.getResourceId()
                    });
                }
            }

            j["maze"] = {
                {"width", maze.getWidth()},
                {"height", maze.getHeight()},
                {"seed", maze.getSeed()},
                {"start_x", maze.getStartPoint().x},
                {"start_y", maze.getStartPoint().y},
                {"exit_x", maze.getExitPoint().x},
                {"exit_y", maze.getExitPoint().y},
                {"rooms", roomsJson},
                {"items", itemsJson},
                {"cells", cellsJson}
            };

            // 4. Simulation Engine & Agent
            const auto& agent = engine.getAgent();
            nlohmann::json bpItemsJson = nlohmann::json::array();
            for (const auto& it : agent.getBackpack().getItems()) {
                bpItemsJson.push_back({
                    {"id", it.id},
                    {"name", it.name},
                    {"weight", it.weight},
                    {"value", it.value},
                    {"cat", static_cast<int>(it.category)},
                    {"x", it.position.x},
                    {"y", it.position.y}
                });
            }

            const auto& plan = engine.getPlan();
            nlohmann::json waypointsJson = nlohmann::json::array();
            for (const auto& wp : plan.visitOrder) {
                waypointsJson.push_back({
                    {"x", wp.position.x},
                    {"y", wp.position.y},
                    {"item_id", wp.itemId},
                    {"label", wp.label},
                    {"weight", wp.itemWeight},
                    {"value", wp.itemValue}
                });
            }

            nlohmann::json fullPathJson = nlohmann::json::array();
            for (const auto& pt : plan.fullPath) {
                fullPathJson.push_back({{"x", pt.x}, {"y", pt.y}});
            }

            j["simulation"] = {
                {"agent_x", agent.getPosition().x},
                {"agent_y", agent.getPosition().y},
                {"current_energy", agent.getCurrentEnergy()},
                {"max_energy", agent.getMaxEnergy()},
                {"energy_spent", agent.getEnergySpent()},
                {"steps_taken", agent.getStepsTaken()},
                {"status", static_cast<int>(agent.getStatus())},
                {"backpack_capacity", agent.getBackpack().getMaxCapacity()},
                {"backpack_items", bpItemsJson},
                {"collected_item_ids", engine.getCollectedItemIds()},
                {"current_step_index", engine.getCurrentStepIndex()},
                {"is_manual_mode", engine.isManualMode()},
                {"is_paused", engine.isPaused()},
                {"is_completed", engine.isCompleted()},
                {"plan", {
                    {"route_valid", plan.routeValid},
                    {"total_energy_cost", plan.totalEnergyCost},
                    {"energy_before_2opt", plan.energyBefore2Opt},
                    {"total_steps", plan.totalSteps},
                    {"two_opt_gain_percent", plan.twoOptGainPercent},
                    {"calculation_time_us", plan.calculationTimeMicroseconds},
                    {"waypoints", waypointsJson},
                    {"full_path", fullPathJson}
                }}
            };

            // 5. Benchmarks table
            nlohmann::json benchArray = nlohmann::json::array();
            for (const auto& b : benchmarks) {
                benchArray.push_back({
                    {"category", b.category},
                    {"algorithm", b.algorithmName},
                    {"dimension", b.mapDimension},
                    {"steps", b.steps},
                    {"energy_cost", b.energyCost},
                    {"visited_nodes", b.visitedNodes},
                    {"time_us", b.timeMicrosec},
                    {"items_count", b.itemCount},
                    {"weight_kg", b.weightKg},
                    {"value_credits", b.valueCredits},
                    {"quality_percent", b.qualityPercent}
                });
            }
            j["benchmarks"] = benchArray;

            std::ofstream file(filepath);
            if (!file.is_open()) return false;
            file << j.dump(4);
            file.close();
            return true;
        } catch (...) {
            return false;
        }
    }

    static bool loadSessionState(const std::string& filepath,
                                 core::Config& config,
                                 SessionGuiState& guiState,
                                 map::Maze& maze,
                                 simulation::SimulationEngine& engine,
                                 std::vector<io::BenchmarkRow>& benchmarks) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) return false;

            nlohmann::json j;
            file >> j;
            file.close();

            if (!j.contains("has_active_session") || !j["has_active_session"].get<bool>()) {
                return false;
            }

            // 1. Config
            if (j.contains("config") && j["config"].is_object()) {
                const auto& jc = j["config"];
                if (jc.contains("map_width")) config.mapWidth = jc["map_width"];
                if (jc.contains("map_height")) config.mapHeight = jc["map_height"];
                if (jc.contains("min_room_size")) config.minRoomSize = jc["min_room_size"];
                if (jc.contains("max_room_size")) config.maxRoomSize = jc["max_room_size"];
                if (jc.contains("mud_spawn_chance")) config.mudSpawnChance = jc["mud_spawn_chance"];
                if (jc.contains("extra_passage_chance")) config.extraPassageChance = jc["extra_passage_chance"];
                if (jc.contains("auto_backpack_capacity")) config.autoBackpackCapacity = jc["auto_backpack_capacity"];
                if (jc.contains("show_grid")) config.showGrid = jc["show_grid"];
                if (jc.contains("show_optimal_route")) config.showOptimalRoute = jc["show_optimal_route"];
                if (jc.contains("show_walked_cells")) config.showWalkedCells = jc["show_walked_cells"];
                if (jc.contains("camera_follow_agent")) config.cameraFollowAgent = jc["camera_follow_agent"];
                if (jc.contains("vsync_enabled")) config.vsyncEnabled = jc["vsync_enabled"];
                if (jc.contains("ui_scale")) config.uiScale = jc["ui_scale"];
                if (jc.contains("minimap_scale")) config.minimapScale = jc["minimap_scale"];
                if (jc.contains("show_legend")) config.showLegend = jc["show_legend"];
                if (jc.contains("random_seed")) config.randomSeed = jc["random_seed"];
                if (jc.contains("company_quota")) config.companyQuota = jc["company_quota"];
                if (jc.contains("manual_backpack_capacity")) config.manualBackpackCapacity = jc["manual_backpack_capacity"];
                if (jc.contains("initial_energy")) config.initialEnergy = jc["initial_energy"];
                if (jc.contains("energy_limit_enabled")) config.energyLimitEnabled = jc["energy_limit_enabled"];
                if (jc.contains("backpack_limit_enabled")) config.backpackLimitEnabled = jc["backpack_limit_enabled"];
                if (jc.contains("route_mode")) config.routeMode = (jc["route_mode"] == 1) ? core::RouteMode::RoundTrip : core::RouteMode::PointAToPointB;
                if (jc.contains("speed_multiplier")) config.speedMultiplier = jc["speed_multiplier"];
            }

            // 2. GUI
            if (j.contains("gui") && j["gui"].is_object()) {
                const auto& jg = j["gui"];
                if (jg.contains("sidebar_width")) guiState.sidebarWidth = jg["sidebar_width"];
                if (jg.contains("sidebar_collapsed")) guiState.sidebarCollapsed = jg["sidebar_collapsed"];
                if (jg.contains("mission_panel_width")) guiState.missionPanelWidth = jg["mission_panel_width"];
                if (jg.contains("mission_panel_collapsed")) guiState.missionPanelCollapsed = jg["mission_panel_collapsed"];
                if (jg.contains("startup_hint_dismissed")) guiState.startupHintDismissed = jg["startup_hint_dismissed"];
                if (jg.contains("camera_zoom")) guiState.cameraZoom = jg["camera_zoom"];
                if (jg.contains("camera_center_x")) guiState.cameraCenterX = jg["camera_center_x"];
                if (jg.contains("camera_center_y")) guiState.cameraCenterY = jg["camera_center_y"];
                if (jg.contains("knapsack_algo_index")) guiState.knapsackAlgoIndex = jg["knapsack_algo_index"];
                if (jg.contains("pathfinder_algo_index")) guiState.pathfinderAlgoIndex = jg["pathfinder_algo_index"];
                if (jg.contains("show_legend")) guiState.showLegend = jg["show_legend"];
                if (jg.contains("catalog_open")) guiState.catalogOpen = jg["catalog_open"];
                if (jg.contains("collected_open")) guiState.collectedOpen = jg["collected_open"];
                if (jg.contains("monitor_powered_on")) guiState.monitorPoweredOn = jg["monitor_powered_on"];
                if (jg.contains("sidebar_scroll_y")) guiState.sidebarScrollY = jg["sidebar_scroll_y"];
                if (jg.contains("sections_open") && jg["sections_open"].is_array()) {
                    guiState.sectionsOpen.clear();
                    for (const auto& v : jg["sections_open"]) {
                        guiState.sectionsOpen.push_back(v.get<bool>());
                    }
                }
                if (jg.contains("is_fullscreen")) guiState.isFullscreen = jg["is_fullscreen"];
                if (jg.contains("custom_seed_enabled")) guiState.customSeedEnabled = jg["custom_seed_enabled"];
                if (jg.contains("custom_seed_input")) guiState.customSeedInput = jg.value("custom_seed_input", "");
                if (jg.contains("language")) guiState.language = jg.value("language", "UA");
            }

            // 3. Maze
            if (!j.contains("maze") || !j["maze"].is_object()) return false;
            const auto& jm = j["maze"];
            int w = jm.value("width", config.mapWidth > 0 ? config.mapWidth : 30);
            int h = jm.value("height", config.mapHeight > 0 ? config.mapHeight : 30);
            config.mapWidth = w;
            config.mapHeight = h;
            maze.resize(w, h);
            maze.setSeed(jm.value("seed", 0u));
            maze.setStartPoint({jm.value("start_x", 1), jm.value("start_y", 1)});
            maze.setExitPoint({jm.value("exit_x", w - 2), jm.value("exit_y", h - 2)});

            if (jm.contains("rooms") && jm["rooms"].is_array()) {
                std::vector<map::Room> rooms;
                for (const auto& jr : jm["rooms"]) {
                    map::Room r;
                    r.id = jr.value("id", 0);
                    r.x = jr.value("x", 0);
                    r.y = jr.value("y", 0);
                    r.width = jr.value("w", 0);
                    r.height = jr.value("h", 0);
                    if (jr.contains("items") && jr["items"].is_array()) {
                        for (int id : jr["items"]) r.itemIds.push_back(id);
                    }
                    rooms.push_back(r);
                }
                maze.getRooms() = rooms;
            }

            if (jm.contains("items") && jm["items"].is_array()) {
                std::vector<core::Item> items;
                for (const auto& ji : jm["items"]) {
                    core::Item it;
                    it.id = ji.value("id", 0);
                    it.name = ji.value("name", "");
                    it.weight = ji.value("weight", 1);
                    it.value = ji.value("value", 10);
                    it.category = static_cast<core::ItemCategory>(ji.value("cat", 0));
                    it.position = {ji.value("x", 0), ji.value("y", 0)};
                    items.push_back(it);
                }
                maze.getItems() = items;
            }

            if (jm.contains("cells") && jm["cells"].is_array()) {
                const auto& cellsArr = jm["cells"];
                size_t idx = 0;
                for (int y = 0; y < h && idx < cellsArr.size(); ++y) {
                    for (int x = 0; x < w && idx < cellsArr.size(); ++x, ++idx) {
                        const auto& c = cellsArr[idx];
                        if (c.is_array() && c.size() >= 3) {
                            maze.at(x, y).setType(static_cast<core::CellType>(c[0].get<int>()));
                            maze.at(x, y).setWalked(c[1].get<int>() != 0);
                            maze.at(x, y).setResourceId(c[2].get<int>());
                        }
                    }
                }
            }
            maze.recomputeExteriorWalls();

            // 4. Simulation Engine & Agent
            if (!j.contains("simulation") || !j["simulation"].is_object()) return false;
            const auto& js = j["simulation"];
            core::Point agentPos{js.value("agent_x", maze.getStartPoint().x), js.value("agent_y", maze.getStartPoint().y)};
            int curEnergy = js.value("current_energy", config.initialEnergy);
            int energySpent = js.value("energy_spent", 0);
            int stepsTaken = js.value("steps_taken", 0);
            simulation::AgentStatus status = static_cast<simulation::AgentStatus>(js.value("status", 0));
            size_t stepIdx = js.value("current_step_index", 0);
            bool isManual = js.value("is_manual_mode", false);
            bool isPaused = js.value("is_paused", true);
            bool isCompleted = js.value("is_completed", false);

            std::vector<core::Item> bpItems;
            if (js.contains("backpack_items") && js["backpack_items"].is_array()) {
                for (const auto& ji : js["backpack_items"]) {
                    core::Item it;
                    it.id = ji.value("id", 0);
                    it.name = ji.value("name", "");
                    it.weight = ji.value("weight", 1);
                    it.value = ji.value("value", 10);
                    it.category = static_cast<core::ItemCategory>(ji.value("cat", 0));
                    it.position = {ji.value("x", 0), ji.value("y", 0)};
                    bpItems.push_back(it);
                }
            }

            std::vector<int> colIds;
            if (js.contains("collected_item_ids") && js["collected_item_ids"].is_array()) {
                for (int id : js["collected_item_ids"]) colIds.push_back(id);
            }

            algorithms::routing::RoutePlan plan;
            if (js.contains("plan") && js["plan"].is_object()) {
                const auto& jp = js["plan"];
                plan.routeValid = jp.value("route_valid", false);
                plan.totalEnergyCost = jp.value("total_energy_cost", 0);
                plan.energyBefore2Opt = jp.value("energy_before_2opt", 0);
                plan.totalSteps = jp.value("total_steps", 0);
                plan.twoOptGainPercent = jp.value("two_opt_gain_percent", 0.0);
                plan.calculationTimeMicroseconds = jp.value("calculation_time_us", 0LL);

                if (jp.contains("waypoints") && jp["waypoints"].is_array()) {
                    for (const auto& jw : jp["waypoints"]) {
                        algorithms::routing::Waypoint wp;
                        wp.position = {jw.value("x", 0), jw.value("y", 0)};
                        wp.itemId = jw.value("item_id", -1);
                        wp.label = jw.value("label", "");
                        wp.itemWeight = jw.value("weight", 0);
                        wp.itemValue = jw.value("value", 0);
                        plan.visitOrder.push_back(wp);
                    }
                }

                if (jp.contains("full_path") && jp["full_path"].is_array()) {
                    for (const auto& jpt : jp["full_path"]) {
                        plan.fullPath.push_back({jpt.value("x", 0), jpt.value("y", 0)});
                    }
                }
            }

            int maxEnergy = js.value("max_energy", config.initialEnergy);
            int maxCap = js.value("max_capacity", config.manualBackpackCapacity);
            engine.updateConfig(config);
            engine.restoreState(plan, agentPos, curEnergy, energySpent, stepsTaken, status, bpItems, colIds, stepIdx, isManual, isPaused, isCompleted, maxEnergy, maxCap);

            // 5. Benchmarks
            benchmarks.clear();
            if (j.contains("benchmarks") && j["benchmarks"].is_array()) {
                for (const auto& jb : j["benchmarks"]) {
                    io::BenchmarkRow row;
                    row.category = jb.value("category", "");
                    row.algorithmName = jb.value("algorithm", "");
                    row.mapDimension = jb.value("dimension", 0);
                    row.steps = jb.value("steps", 0);
                    row.energyCost = jb.value("energy_cost", 0);
                    row.visitedNodes = jb.value("visited_nodes", 0);
                    row.timeMicrosec = jb.value("time_us", 0LL);
                    row.itemCount = jb.value("items_count", 0);
                    row.weightKg = jb.value("weight_kg", 0);
                    row.valueCredits = jb.value("value_credits", 0);
                    row.qualityPercent = jb.value("quality_percent", 0.0);
                    benchmarks.push_back(row);
                }
            }

            return true;
        } catch (...) {
            return false;
        }
    }
};

} // namespace io
