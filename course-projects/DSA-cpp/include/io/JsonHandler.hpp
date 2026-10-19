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
};

} // namespace io
