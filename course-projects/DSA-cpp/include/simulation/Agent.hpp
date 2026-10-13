#pragma once

#include "core/Types.hpp"
#include "core/Item.hpp"
#include "core/Backpack.hpp"
#include <string>

namespace simulation {

enum class AgentStatus {
    Idle = 0,
    Collecting = 1,
    Returning = 2,
    Evacuating = 3,
    Success = 4,
    Exhausted = 5
};

inline std::string agentStatusToUkrainian(AgentStatus status) {
    switch (status) {
        case AgentStatus::Idle:       return "Очікування";
        case AgentStatus::Collecting: return "Збір ресурсів";
        case AgentStatus::Returning:  return "Повернення на базу";
        case AgentStatus::Evacuating: return "Евакуація через вихід";
        case AgentStatus::Success:    return "Місія успішно виконана!";
        case AgentStatus::Exhausted:  return "Критично: Енергія вичерпана!";
        default:                      return "Невідомий стан";
    }
}

class Agent {
public:
    explicit Agent(int initialEnergy = 600, int backpackCapacity = 50)
        : maxEnergy_(initialEnergy), currentEnergy_(initialEnergy), backpack_(backpackCapacity) {}

    void reset(const core::Point& startPos, int energy, int backpackCap) {
        position_ = startPos;
        maxEnergy_ = energy;
        currentEnergy_ = energy;
        backpack_.setMaxCapacity(backpackCap);
        backpack_.clear();
        status_ = AgentStatus::Idle;
        stepsTaken_ = 0;
        energySpent_ = 0;
    }

    [[nodiscard]] const core::Point& getPosition() const noexcept { return position_; }
    void setPosition(const core::Point& pos) noexcept { position_ = pos; }

    [[nodiscard]] int getMaxEnergy() const noexcept { return maxEnergy_; }
    [[nodiscard]] int getCurrentEnergy() const noexcept { return currentEnergy_; }
    [[nodiscard]] int getEnergySpent() const noexcept { return energySpent_; }
    [[nodiscard]] int getStepsTaken() const noexcept { return stepsTaken_; }

    void restoreState(int currentEnergy, int energySpent, int stepsTaken) noexcept {
        currentEnergy_ = currentEnergy;
        energySpent_ = energySpent;
        stepsTaken_ = stepsTaken;
    }

    [[nodiscard]] core::Backpack& getBackpack() noexcept { return backpack_; }
    [[nodiscard]] const core::Backpack& getBackpack() const noexcept { return backpack_; }

    [[nodiscard]] AgentStatus getStatus() const noexcept { return status_; }
    void setStatus(AgentStatus s) noexcept { status_ = s; }

    [[nodiscard]] std::string getStatusString() const {
        return agentStatusToUkrainian(status_);
    }

    [[nodiscard]] bool isExhausted() const noexcept {
        return currentEnergy_ <= 0;
    }

    [[nodiscard]] double getEnergyPercentage() const noexcept {
        return maxEnergy_ > 0 ? (static_cast<double>(currentEnergy_) / maxEnergy_) * 100.0 : 0.0;
    }

    bool consumeEnergy(int cost, bool limitEnabled = true) {
        energySpent_ += cost;
        if (limitEnabled) {
            currentEnergy_ -= cost;
            if (currentEnergy_ <= 0) {
                currentEnergy_ = 0;
                status_ = AgentStatus::Exhausted;
                return false;
            }
        }
        return true;
    }

    void replenishEnergy(int amount) {
        maxEnergy_ += amount;
        currentEnergy_ += amount;
        if (status_ == AgentStatus::Exhausted) {
            status_ = AgentStatus::Collecting;
        }
    }

    void addStep() noexcept {
        ++stepsTaken_;
    }

private:
    core::Point position_{0, 0};
    int maxEnergy_ = 600;
    int currentEnergy_ = 600;
    int energySpent_ = 0;
    int stepsTaken_ = 0;
    core::Backpack backpack_;
    AgentStatus status_ = AgentStatus::Idle;
};

} // namespace simulation
