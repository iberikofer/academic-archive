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
    bool isManualMode = false;
};

class SimulationEngine {
public:
    SimulationEngine(map::Maze& maze, const core::Config& config)
        : maze_(maze), config_(config), agent_(config.initialEnergy, config.manualBackpackCapacity),
          baseStepDelay_(config.stepDelaySeconds) {}

    void updateConfig(const core::Config& config) {
        config_ = config;
        agent_.getBackpack().setUnlimited(!config_.backpackLimitEnabled);
    }

    [[nodiscard]] const core::Config& getConfig() const noexcept {
        return config_;
    }

    void initPlan(const algorithms::routing::RoutePlan& plan, int energy, int backpackCapacity) {
        plan_ = plan;
        agent_.reset(maze_.getStartPoint(), energy, backpackCapacity);
        agent_.getBackpack().setUnlimited(!config_.backpackLimitEnabled);
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
        isManualMode_ = false;
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
        if (!isManualMode_ && !isCompleted_ && plan_.routeValid) {
            isPaused_ = !isPaused_;
        }
    }

    void play() noexcept {
        if (!isManualMode_ && !isCompleted_ && plan_.routeValid) {
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
        if (isManualMode_) {
            return false;
        }
        if (!plan_.routeValid || currentStepIndex_ + 1 >= plan_.fullPath.size()) {
            isCompleted_ = true;
            isPaused_ = true;
            if (plan_.routeValid && isAtEvacuationCell()) {
                agent_.setStatus(AgentStatus::Success);
            }
            return false;
        }

        const auto& currentPos = plan_.fullPath[currentStepIndex_];
        const auto& nextPos = plan_.fullPath[currentStepIndex_ + 1];

        bool isDiag = (currentPos.x != nextPos.x && currentPos.y != nextPos.y);
        int stepCost = maze_.at(nextPos).getEnergyCost(isDiag);

        bool isExitDisabled = (config_.routeMode == core::RouteMode::RoundTrip);
        const auto& evacTarget = isExitDisabled ? maze_.getStartPoint() : maze_.getExitPoint();
        bool isEvacFinalStep = (nextPos == evacTarget &&
                               (currentStepIndex_ + 1 == plan_.fullPath.size() - 1 || agent_.getStepsTaken() > 0));

        if (config_.energyLimitEnabled) {
            if (isEvacFinalStep && agent_.getCurrentEnergy() < stepCost) {
                // Penultimate step to evacuation cell: allow final push into safety even with insufficient energy!
                agent_.consumeEnergyForced(stepCost);
            } else if (!agent_.consumeEnergy(stepCost, true)) {
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
            if (std::find(collectedItemIds_.begin(), collectedItemIds_.end(), rId) == collectedItemIds_.end()) {
                auto itemOpt = maze_.getItemById(rId);
                if (itemOpt.has_value()) {
                    bool isPlanned = (plannedItemIds_.find(rId) != plannedItemIds_.end());
                    int curWeight = agent_.getBackpack().getCurrentWeight();
                    int maxCap = agent_.getBackpack().getMaxCapacity();

                    if (isPlanned) {
                        // Planned items: prioritized collection (guaranteed by knapsack reservation)
                        if (agent_.getBackpack().canFit(*itemOpt)) {
                            agent_.getBackpack().addItem(*itemOpt);
                            collectedItemIds_.push_back(rId);
                        }
                    } else {
                        // Opportunistic pickup: only pick up unplanned corridor loot if it does NOT
                        // encroach on capacity reserved for yet-to-be-visited planned waypoints
                        int remPlanned = getRemainingPlannedWeight();
                        if (!config_.backpackLimitEnabled ||
                            curWeight + remPlanned + itemOpt->weight <= maxCap) {
                            agent_.getBackpack().addItem(*itemOpt);
                            collectedItemIds_.push_back(rId);
                        }
                    }
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
        bool allowManualEvacUndo =
            isAtEvacuationCell() || agent_.getStatus() == AgentStatus::Success;
        if ((isManualMode_ && !allowManualEvacUndo) || !plan_.routeValid ||
            history_.size() <= 1) {
            return false;
        }

        history_.pop_back();

        const auto& prev = history_.back();
        agent_.setPosition(prev.agentPosition);
        agent_.setStatus(prev.status);
        agent_.restoreState(prev.currentEnergy, prev.energySpent, prev.stepsTaken);
        currentStepIndex_ = prev.routeStepIndex;
        collectedItemIds_ = prev.collectedItemIds;
        isManualMode_ = prev.isManualMode;
        emergencyEvacuationTriggered_ = (prev.status == AgentStatus::Evacuating);

        auto& bp = agent_.getBackpack();
        bp.clear();
        bp.setUnlimited(!config_.backpackLimitEnabled);
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
        for (const auto& f : history_) {
            if (maze_.isValid(f.agentPosition)) {
                maze_.at(f.agentPosition).setWalked(true);
            }
        }

        isCompleted_ = false;
        // Keep isPaused_ true so the user can inspect the restored historical frame without auto-advancing
        return true;
    }

    void instantFinish() {
        if (isManualMode_ || !plan_.routeValid) return;
        isPaused_ = true;
        while (!isCompleted_ && stepForward()) {
        }
    }

    void resetToStart() {
        bool allowManualEvacReset =
            isAtEvacuationCell() || agent_.getStatus() == AgentStatus::Success;
        if ((isManualMode_ && !allowManualEvacReset) || !plan_.routeValid) return;
        int resetEnergy = (config_.initialEnergy > 0) ? config_.initialEnergy : agent_.getMaxEnergy();
        int resetCap = (config_.manualBackpackCapacity > 0) ? config_.manualBackpackCapacity : agent_.getBackpack().getMaxCapacity();
        agent_.reset(maze_.getStartPoint(), resetEnergy, resetCap);
        agent_.getBackpack().setUnlimited(!config_.backpackLimitEnabled);
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
        isManualMode_ = false;
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

    void restoreState(const algorithms::routing::RoutePlan& plan,
                      const core::Point& agentPos,
                      int currentEnergy, int energySpent, int stepsTaken,
                      AgentStatus status,
                      const std::vector<core::Item>& backpackItems,
                      const std::vector<int>& collectedItemIds,
                      size_t currentStepIndex,
                      bool isManualMode,
                      bool isPaused,
                      bool isCompleted,
                      int maxEnergy = -1, int maxCapacity = -1) {
        plan_ = plan;
        if (maxEnergy <= 0) maxEnergy = (config_.initialEnergy > 0) ? config_.initialEnergy : 600;
        if (maxCapacity <= 0) maxCapacity = (config_.manualBackpackCapacity > 0) ? config_.manualBackpackCapacity : 55;
        agent_.setMaxEnergy(maxEnergy);
        agent_.getBackpack().setMaxCapacity(maxCapacity);
        agent_.setPosition(agentPos);
        agent_.setStatus(status);
        agent_.restoreState(currentEnergy, energySpent, stepsTaken);

        auto& bp = agent_.getBackpack();
        bp.clear();
        bp.setUnlimited(!config_.backpackLimitEnabled);
        for (const auto& it : backpackItems) {
            bp.addItem(it);
        }

        collectedItemIds_ = collectedItemIds;
        plannedItemIds_.clear();
        for (const auto& wp : plan_.visitOrder) {
            if (wp.itemId >= 0) {
                plannedItemIds_.insert(wp.itemId);
            }
        }

        currentStepIndex_ = currentStepIndex;
        isManualMode_ = isManualMode;
        isPaused_ = isPaused;
        isCompleted_ = isCompleted;
        accumulatedTime_ = 0.0f;

        // Reconstruct history so stepBackward() works properly even if resumed at finish
        history_.clear();
        if (plan_.routeValid && !plan_.fullPath.empty()) {
            std::vector<core::Item> tempBp;
            std::vector<int> tempCollected;
            size_t maxIdx = std::min(currentStepIndex, plan_.fullPath.size() - 1);
            int simEnergy = maxEnergy;
            int simSpent = 0;

            for (size_t i = 0; i <= maxIdx; ++i) {
                core::Point pt = plan_.fullPath[i];
                if (i > 0) {
                    core::Point prevPt = plan_.fullPath[i - 1];
                    bool isDiag = (prevPt.x != pt.x && prevPt.y != pt.y);
                    int cost = maze_.at(pt).getEnergyCost(isDiag);
                    simEnergy = std::max(0, simEnergy - cost);
                    simSpent += cost;
                }
                const auto* item = maze_.getItemAt(pt.x, pt.y);
                if (item && isItemPlanned(item->id)) {
                    if (std::find(tempCollected.begin(), tempCollected.end(), item->id) == tempCollected.end()) {
                        tempCollected.push_back(item->id);
                        tempBp.push_back(*item);
                    }
                }
                HistoryFrame frame;
                frame.agentPosition = pt;
                frame.routeStepIndex = i;
                frame.collectedItemIds = tempCollected;
                frame.backpackItems = tempBp;
                frame.currentEnergy = simEnergy;
                frame.energySpent = simSpent;
                frame.stepsTaken = static_cast<int>(i);
                frame.status = (i == maxIdx && !isManualMode) ? status : AgentStatus::Collecting;
                frame.isManualMode = false;
                history_.push_back(frame);
            }

            if (isManualMode) {
                HistoryFrame manFrame;
                manFrame.agentPosition = agentPos;
                manFrame.currentEnergy = currentEnergy;
                manFrame.energySpent = energySpent;
                manFrame.stepsTaken = stepsTaken;
                manFrame.status = status;
                manFrame.backpackItems = backpackItems;
                manFrame.collectedItemIds = collectedItemIds;
                manFrame.routeStepIndex = currentStepIndex;
                manFrame.isManualMode = true;
                history_.push_back(manFrame);
            }
        } else {
            recordHistory();
        }
    }

    Agent& getAgent() noexcept { return agent_; }
    const Agent& getAgent() const noexcept { return agent_; }

    const algorithms::routing::RoutePlan& getPlan() const noexcept { return plan_; }

    [[nodiscard]] const std::vector<int>& getCollectedItemIds() const noexcept { return collectedItemIds_; }

    [[nodiscard]] bool isItemPlanned(int itemId) const noexcept {
        return plannedItemIds_.find(itemId) != plannedItemIds_.end();
    }

    [[nodiscard]] size_t getExtraCollectedCount() const noexcept {
        size_t extra = 0;
        for (int id : collectedItemIds_) {
            if (plannedItemIds_.find(id) == plannedItemIds_.end()) {
                ++extra;
            }
        }
        return extra;
    }

    void replenishEnergy(int amount) {
        agent_.replenishEnergy(amount);
        if (agent_.getStatus() == AgentStatus::Exhausted) {
            agent_.setStatus(AgentStatus::Collecting);
            isCompleted_ = false;
        }
    }

    [[nodiscard]] int getRemainingPlannedWeight() const {
        int rem = 0;
        for (int pId : plannedItemIds_) {
            if (std::find(collectedItemIds_.begin(), collectedItemIds_.end(), pId) == collectedItemIds_.end()) {
                auto opt = maze_.getItemById(pId);
                if (opt.has_value()) {
                    rem += opt->weight;
                }
            }
        }
        return rem;
    }

    [[nodiscard]] bool isManualMode() const noexcept { return isManualMode_; }
    void setManualMode(bool manual) noexcept { isManualMode_ = manual; }

    [[nodiscard]] bool isAtEvacuationCell() const noexcept {
        bool isExitDisabled = (config_.routeMode == core::RouteMode::RoundTrip);
        const auto& evacTarget = isExitDisabled ? maze_.getStartPoint() : maze_.getExitPoint();
        return (agent_.getPosition() == evacTarget) &&
               (!isExitDisabled || agent_.getStepsTaken() > 0);
    }

    [[nodiscard]] bool canMoveManual(int dx = 0, int dy = 0) const noexcept {
        if (agent_.getStatus() == AgentStatus::Success || isAtEvacuationCell()) return false;
        if (config_.energyLimitEnabled && agent_.isExhausted()) {
            if (dx != 0 || dy != 0) {
                bool isExitDisabled = (config_.routeMode == core::RouteMode::RoundTrip);
                const auto& evacTarget = isExitDisabled ? maze_.getStartPoint() : maze_.getExitPoint();
                core::Point targetPos{agent_.getPosition().x + dx, agent_.getPosition().y + dy};
                if (targetPos == evacTarget) {
                    return true;
                }
            }
            return false;
        }
        return true;
    }

    bool returnToLastAutoStep() {
        if (!isManualMode_ || history_.empty()) {
            return false;
        }

        while (history_.size() > 1 && history_.back().isManualMode) {
            history_.pop_back();
        }

        const auto& target = history_.back();
        agent_.setPosition(target.agentPosition);
        agent_.setStatus(target.status);
        agent_.restoreState(target.currentEnergy, target.energySpent, target.stepsTaken);
        currentStepIndex_ = target.routeStepIndex;
        collectedItemIds_ = target.collectedItemIds;
        isManualMode_ = target.isManualMode;
        emergencyEvacuationTriggered_ = (target.status == AgentStatus::Evacuating);

        auto& bp = agent_.getBackpack();
        bp.clear();
        bp.setUnlimited(!config_.backpackLimitEnabled);
        for (const auto& item : target.backpackItems) {
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

        isPaused_ = true;
        isCompleted_ = (target.status == AgentStatus::Success);
        return true;
    }

    bool moveAgentManual(int dx, int dy) {
        if (!canMoveManual(dx, dy)) return false;

        core::Point curPos = agent_.getPosition();
        core::Point nextPos{curPos.x + dx, curPos.y + dy};

        if (!maze_.isValid(nextPos) || !maze_.at(nextPos).isPassable()) {
            return false;
        }

        bool isDiag = (dx != 0 && dy != 0);

        if (isDiag) {
            // Check diagonal restrictions matching maze_.getNeighbors:
            // 1. Cannot move diagonally to/from Start or Exit (strict orthogonal airlock seal)
            auto currType = maze_.at(curPos).getType();
            auto nextType = maze_.at(nextPos).getType();
            if (currType == core::CellType::Start || currType == core::CellType::Exit ||
                nextType == core::CellType::Start || nextType == core::CellType::Exit) {
                if (dx != 0 && maze_.isValid(curPos.x + dx, curPos.y) && maze_.at(curPos.x + dx, curPos.y).isPassable()) {
                    return moveAgentManual(dx, 0);
                }
                if (dy != 0 && maze_.isValid(curPos.x, curPos.y + dy) && maze_.at(curPos.x, curPos.y + dy).isPassable()) {
                    return moveAgentManual(0, dy);
                }
                return false;
            }
            // 2. Strict no-corner-cutting: both adjacent orthogonal cells must be passable
            bool blockedSide1 = !maze_.isValid(curPos.x + dx, curPos.y) || !maze_.at(curPos.x + dx, curPos.y).isPassable();
            bool blockedSide2 = !maze_.isValid(curPos.x, curPos.y + dy) || !maze_.at(curPos.x, curPos.y + dy).isPassable();
            if (blockedSide1 || blockedSide2) {
                if (!blockedSide1) {
                    return moveAgentManual(dx, 0);
                }
                if (!blockedSide2) {
                    return moveAgentManual(0, dy);
                }
                return false;
            }
            // 3. Mud corner barrier: cannot jump diagonally across mud corners
            auto side1Type = maze_.at(curPos.x + dx, curPos.y).getType();
            auto side2Type = maze_.at(curPos.x, curPos.y + dy).getType();
            if (side1Type == core::CellType::Mud || side2Type == core::CellType::Mud) {
                if (currType != core::CellType::Mud || nextType != core::CellType::Mud) {
                    if (maze_.isValid(curPos.x + dx, curPos.y) && maze_.at(curPos.x + dx, curPos.y).isPassable()) {
                        return moveAgentManual(dx, 0);
                    }
                    if (maze_.isValid(curPos.x, curPos.y + dy) && maze_.at(curPos.x, curPos.y + dy).isPassable()) {
                        return moveAgentManual(0, dy);
                    }
                    return false;
                }
            }
        }

        isPaused_ = true;
        isManualMode_ = true;

        int stepCost = maze_.at(nextPos).getEnergyCost(isDiag);

        bool isExitDisabled = (config_.routeMode == core::RouteMode::RoundTrip);
        const auto& evacTarget = isExitDisabled ? maze_.getStartPoint() : maze_.getExitPoint();
        bool isEvacFinalStep = (nextPos == evacTarget && agent_.getStepsTaken() > 0);

        if (config_.energyLimitEnabled) {
            if (isEvacFinalStep && agent_.getCurrentEnergy() < stepCost) {
                // Penultimate step to evacuation cell: allow final push into safety even with insufficient energy!
                agent_.consumeEnergyForced(stepCost);
            } else if (!agent_.consumeEnergy(stepCost, true)) {
                isCompleted_ = true;
                agent_.setStatus(AgentStatus::Exhausted);
                recordHistory();
                return false;
            }
        } else {
            agent_.consumeEnergy(stepCost, false);
        }

        agent_.setPosition(nextPos);
        agent_.addStep();
        maze_.at(nextPos).setWalked(true);

        auto& cell = maze_.at(nextPos);
        if (cell.hasResource()) {
            int rId = cell.getResourceId();
            if (std::find(collectedItemIds_.begin(), collectedItemIds_.end(), rId) == collectedItemIds_.end()) {
                auto itemOpt = maze_.getItemById(rId);
                if (itemOpt.has_value()) {
                    if (agent_.getBackpack().canFit(*itemOpt)) {
                        agent_.getBackpack().addItem(*itemOpt);
                        collectedItemIds_.push_back(rId);
                    }
                }
            }
        }

        if (nextPos == evacTarget && agent_.getStepsTaken() > 0) {
            isCompleted_ = true;
            agent_.setStatus(AgentStatus::Success);
        } else if (agent_.getStatus() == AgentStatus::Success) {
            isCompleted_ = false;
            agent_.setStatus(AgentStatus::Collecting);
        }

        recordHistory();
        return true;
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
        frame.isManualMode = isManualMode_;
        history_.push_back(frame);
    }

    void checkScenarioA1Safety() {
        if (!config_.energyLimitEnabled) return;

        // Calculate realistic lower bound distance to target exit/start taking into account maze winding and mud
        core::Point dest = (config_.routeMode == core::RouteMode::PointAToPointB) ?
                           maze_.getExitPoint() : maze_.getStartPoint();

        int dx = std::abs(agent_.getPosition().x - dest.x);
        int dy = std::abs(agent_.getPosition().y - dest.y);
        // Factor in corridor tortuosity (~2.5x) and terrain penalties
        int minEnergyToSafety = (dx + dy) * 25 + 60;

        if (agent_.getCurrentEnergy() <= minEnergyToSafety) {
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
    bool isManualMode_ = false;
    bool emergencyEvacuationTriggered_ = false;
};

} // namespace simulation
