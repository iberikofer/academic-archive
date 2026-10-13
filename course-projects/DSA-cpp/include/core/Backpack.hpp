#pragma once

#include "Item.hpp"
#include <vector>
#include <algorithm>

namespace core {

class Backpack {
public:
    explicit Backpack(int maxCapacity = 50)
        : maxCapacity_(maxCapacity), currentWeight_(0), currentValue_(0) {}

    [[nodiscard]] int getMaxCapacity() const noexcept { return maxCapacity_; }
    void setMaxCapacity(int capacity) noexcept {
        maxCapacity_ = std::max(1, capacity);
    }

    [[nodiscard]] int getCurrentWeight() const noexcept { return currentWeight_; }
    [[nodiscard]] int getCurrentValue() const noexcept { return currentValue_; }
    [[nodiscard]] const std::vector<Item>& getItems() const noexcept { return items_; }

    [[nodiscard]] bool canFit(const Item& item) const noexcept {
        return (currentWeight_ + item.weight) <= maxCapacity_;
    }

    bool addItem(const Item& item) {
        if (!canFit(item)) {
            return false;
        }
        items_.push_back(item);
        currentWeight_ += item.weight;
        currentValue_ += item.value;
        return true;
    }

    bool removeItem(int itemId) {
        auto it = std::find_if(items_.begin(), items_.end(), [itemId](const Item& item) {
            return item.id == itemId;
        });
        if (it != items_.end()) {
            currentWeight_ -= it->weight;
            currentValue_ -= it->value;
            items_.erase(it);
            return true;
        }
        return false;
    }

    void clear() noexcept {
        items_.clear();
        currentWeight_ = 0;
        currentValue_ = 0;
    }

    [[nodiscard]] bool isFull() const noexcept {
        return currentWeight_ >= maxCapacity_;
    }

    [[nodiscard]] double getUtilizationPercent() const noexcept {
        return maxCapacity_ > 0 ? (static_cast<double>(currentWeight_) / maxCapacity_) * 100.0 : 0.0;
    }

private:
    int maxCapacity_;
    int currentWeight_;
    int currentValue_;
    std::vector<Item> items_;
};

} // namespace core
