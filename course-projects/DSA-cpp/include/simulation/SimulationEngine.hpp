#pragma once

#include "Agent.hpp"
#include "map/Maze.hpp"
#include "core/Config.hpp"
#include "algorithms/routing/RoutePlan.hpp"
#include <algorithm>
#include <vector>
#include <unordered_set>

namespace simulation {

struct HistoryFrame {
    core::Point agentPosition;
    int currentEnergy;
    int energySpent;
    int stepsTaken;
    AgentStatus status;
    std::vector<core::Item> backpackItems;
    std::vector<int> collectedItemIds;
    size_t routeStepIndex;
};

class SimulationEngine {
public:
    SimulationEngine(map::Maze& maze, const core::Config& config)
        : maze_(maze), config_(config), agent_(config.initialEnergy, config.manualBackpackCapacity),
          baseStepDelay_(config.stepDelaySeconds) {}

    void updateConfig(const core::Config& config) {
        config_ = config;
    }

    void initPlan(const algorithms::routing::RoutePlan& plan, int energy, int backpackCapacity) {
        plan_ = plan;
        agent_.reset(maze_.getStartPoint(), energy, backpackCapacity);
        maze_.resetVisualizationFlags();
        if (maze_.isValid(maze_.getStartPoint())) {
            maze_.at(maze_.getStartPoint()).setWalked(true);
        }
        history_.clear();
        collectedItemIds_.clear();
        plannedItemIds_.clear();
        for (const auto& wp : plan_.visitOrder) {
            if (wp.itemId >= 0) {
                plannedItemIds_.insert(wp.itemId);
            }
        }
        currentStepIndex_ = 0;
        accumulatedTime_ = 0.0f;
        isPaused_ = true;
        isCompleted_ = false;
        emergencyEvacuationTriggered_ = false;

        recordHistory();
    }

    void update(float dt) {
        if (isPaused_ || isCompleted_ || plan_.fullPath.empty()) {
            return;
        }

        float safeDt = std::min(dt, 0.25f);
        accumulatedTime_ += safeDt;
        float currentStepDuration = baseStepDelay_ / config_.speedMultiplier;

        while (accumulatedTime_ >= currentStepDuration && !isCompleted_) {
            accumulatedTime_ -= currentStepDuration;
            stepForward();
        }
    }

    void togglePlayPause() noexcept {
        if (!isCompleted_ && plan_.routeValid) {
            isPaused_ = !isPaused_;
        }
    }

    void play() noexcept {
        if (!isCompleted_ && plan_.routeValid) {
            isPaused_ = false;
        }
    }

    void pause() noexcept {
        isPaused_ = true;
    }

    void setPaused(bool paused) noexcept {
        if (paused) {
            pause();
        } else {
            play();
        }
    }

    bool stepForward() {
        if (!plan_.routeValid || currentStepIndex_ + 1 >= plan_.fullPath.size()) {
            isCompleted_ = true;
            isPaused_ = true;
            if (plan_.routeValid) {
                agent_.setStatus(AgentStatus::Success);
            }
            return false;
        }

        const auto& currentPos = plan_.fullPath[currentStepIndex_];
        const auto& nextPos = plan_.fullPath[currentStepIndex_ + 1];

        bool isDiag = (currentPos.x != nextPos.x && currentPos.y != nextPos.y);
        int stepCost = maze_.at(nextPos).getEnergyCost(isDiag);

        if (config_.energyLimitEnabled) {
            if (!agent_.consumeEnergy(stepCost, true)) {
                isCompleted_ = true;
                isPaused_ = true;
                agent_.setStatus(AgentStatus::Exhausted);
                return false;
            }
        } else {
            agent_.consumeEnergy(stepCost, false);
        }

        ++currentStepIndex_;
        agent_.setPosition(nextPos);
        agent_.addStep();

        maze_.at(nextPos).setWalked(true);

        auto& cell = maze_.at(nextPos);
        if (cell.hasResource()) {
            int rId = cell.getResourceId();
            bool isPlanned = plannedItemIds_.empty() || (plannedItemIds_.find(rId) != plannedItemIds_.end());
            if (isPlanned && std::find(collectedItemIds_.begin(), collectedItemIds_.end(), rId) == collectedItemIds_.end()) {
                auto itemOpt = maze_.getItemById(rId);
                if (itemOpt.has_value() && agent_.getBackpack().canFit(*itemOpt)) {
                    agent_.getBackpack().addItem(*itemOpt);
                    collectedItemIds_.push_back(rId);
                }
            }
        }

        if (config_.energyLimitEnabled) {
            checkScenarioA1Safety();
        }

        recordHistory();

        if (currentStepIndex_ + 1 >= plan_.fullPath.size()) {
            isCompleted_ = true;
            isPaused_ = true;
            agent_.setStatus(AgentStatus::Success);
        }

        return true;
    }

    bool stepBackward() {
        if (!plan_.routeValid || history_.size() <= 1) {
            return false;
        }

        history_.pop_back();

        const auto& prev = history_.back();
        agent_.setPosition(prev.agentPosition);
        agent_.setStatus(prev.status);
        agent_.restoreState(prev.currentEnergy, prev.energySpent, prev.stepsTaken);
        currentStepIndex_ = prev.routeStepIndex;
        collectedItemIds_ = prev.collectedItemIds;
        emergencyEvacuationTriggered_ = (prev.status == AgentStatus::Evacuating);

        auto& bp = agent_.getBackpack();
        bp.clear();
        for (const auto& item : prev.backpackItems) {
            bp.addItem(item);
        }

        for (int y = 0; y < maze_.getHeight(); ++y) {
            for (int x = 0; x < maze_.getWidth(); ++x) {
                maze_.at(x, y).setWalked(false);
            }
        }
        for (size_t i = 0; i <= currentStepIndex_ && i < plan_.fullPath.size(); ++i) {
            maze_.at(plan_.fullPath[i]).setWalked(true);
        }

        isCompleted_ = false;
        return true;
    }

    void instantFinish() {
        if (!plan_.routeValid) return;
        isPaused_ = true;
        while (!isCompleted_ && stepForward()) {
        }
    }

    void resetToStart() {
        if (!plan_.routeValid) return;
        agent_.reset(maze_.getStartPoint(), agent_.getMaxEnergy(), agent_.getBackpack().getMaxCapacity());
        maze_.resetVisualizationFlags();
        if (maze_.isValid(maze_.getStartPoint())) {
            maze_.at(maze_.getStartPoint()).setWalked(true);
        }
        history_.clear();
        collectedItemIds_.clear();
        currentStepIndex_ = 0;
        accumulatedTime_ = 0.0f;
        isPaused_ = true;
        isCompleted_ = false;
        emergencyEvacuationTriggered_ = false;
        recordHistory();
    }

    void setSpeedMultiplier(float speed) noexcept {
        config_.speedMultiplier = std::clamp(speed, 0.5f, 4.0f);
    }

    [[nodiscard]] float getSpeedMultiplier() const noexcept {
        return config_.speedMultiplier;
    }

    [[nodiscard]] bool isPaused() const noexcept { return isPaused_; }
    [[nodiscard]] bool isCompleted() const noexcept { return isCompleted_; }
    [[nodiscard]] bool isEmergencyEvacuation() const noexcept { return emergencyEvacuationTriggered_; }
    [[nodiscard]] size_t getCurrentStepIndex() const noexcept { return currentStepIndex_; }
    [[nodiscard]] size_t getTotalSteps() const noexcept { return plan_.fullPath.empty() ? 0 : plan_.fullPath.size() - 1; }

    Agent& getAgent() noexcept { return agent_; }
    const Agent& getAgent() const noexcept { return agent_; }

    const algorithms::routing::RoutePlan& getPlan() const noexcept { return plan_; }

    [[nodiscard]] const std::vector<int>& getCollectedItemIds() const noexcept { return collectedItemIds_; }

    void replenishEnergy(int amount) {
        agent_.replenishEnergy(amount);
        if (agent_.getStatus() == AgentStatus::Exhausted) {
            agent_.setStatus(AgentStatus::Collecting);
            isCompleted_ = false;
        }
    }

private:
    void recordHistory() {
        HistoryFrame frame;
        frame.agentPosition = agent_.getPosition();
        frame.currentEnergy = agent_.getCurrentEnergy();
        frame.energySpent = agent_.getEnergySpent();
        frame.stepsTaken = agent_.getStepsTaken();
        frame.status = agent_.getStatus();
        frame.backpackItems = agent_.getBackpack().getItems();
        frame.collectedItemIds = collectedItemIds_;
        frame.routeStepIndex = currentStepIndex_;
        history_.push_back(frame);
    }

    void checkScenarioA1Safety() {
        if (!config_.energyLimitEnabled) return;

        // Calculate approximate distance to target exit/start
        core::Point dest = (config_.routeMode == core::RouteMode::PointAToPointB) ?
                           maze_.getExitPoint() : maze_.getStartPoint();

        int dx = std::abs(agent_.getPosition().x - dest.x);
        int dy = std::abs(agent_.getPosition().y - dest.y);
        int minEnergyToSafety = (dx + dy) * 10;

        if (agent_.getCurrentEnergy() <= minEnergyToSafety + 30) {
            emergencyEvacuationTriggered_ = true;
            agent_.setStatus(AgentStatus::Evacuating);
        }
    }

    map::Maze& maze_;
    core::Config config_;
    Agent agent_;
    algorithms::routing::RoutePlan plan_;
    std::vector<HistoryFrame> history_;
    std::vector<int> collectedItemIds_;
    std::unordered_set<int> plannedItemIds_;

    size_t currentStepIndex_ = 0;
    float accumulatedTime_ = 0.0f;
    float baseStepDelay_ = 0.75f; // 1 move per 0.75 seconds by default
    bool isPaused_ = true;
    bool isCompleted_ = false;
    bool emergencyEvacuationTriggered_ = false;
};

} // namespace simulation
