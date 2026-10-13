#pragma once

#include "Types.hpp"
#include <string>
#include <vector>
#include <limits>

namespace core {

enum class ItemCategory {
    LightScrap = 0,    // 1..3 kg (one-handed light scrap)
    MediumScrap = 1,   // 4..8 kg (household scrap)
    HeavyScrap = 2,    // 10..18 kg (two-handed heavy scrap)

    // Compatibility aliases with previous code
    LightGem = LightScrap,
    MediumRelic = MediumScrap,
    HeavyArtifact = HeavyScrap
};

inline std::string itemCategoryToString(ItemCategory cat) {
    switch (cat) {
        case ItemCategory::LightScrap:  return "Дрібний брухт (1-3 кг)";
        case ItemCategory::MediumScrap: return "Побутовий брухт (4-8 кг)";
        case ItemCategory::HeavyScrap:  return "Важкий брухт (10-18 кг)";
        default: return "Брухт";
    }
}

struct ScrapTemplate {
    std::string name;
    int baseWeight;
    int baseValue;
    ItemCategory category;
};

// Complete catalog of 60 authentic Lethal Company scrap items
inline const std::vector<ScrapTemplate>& getLethalScrapTemplates() {
    static const std::vector<ScrapTemplate> templates = {
        // --- 1. Light Scrap (1-3 kg) - 20 items ---
        {"Гумове каченя", 1, 45, ItemCategory::LightScrap},
        {"Пластикова рибка", 1, 35, ItemCategory::LightScrap},
        {"Пульт дистанційного керування", 1, 50, ItemCategory::LightScrap},
        {"Латунний дзвіночок", 2, 60, ItemCategory::LightScrap},
        {"Магічна куля", 2, 55, ItemCategory::LightScrap},
        {"Лупа інспектора", 2, 65, ItemCategory::LightScrap},
        {"Заводна щелепа", 1, 70, ItemCategory::LightScrap},
        {"Аудіокасета з записом", 1, 40, ItemCategory::LightScrap},
        {"Флакон парфумів", 2, 75, ItemCategory::LightScrap},
        {"Золота чашка", 3, 90, ItemCategory::LightScrap},
        {"Повітряний ріжок", 2, 65, ItemCategory::LightScrap},
        {"Клоунський гудок", 2, 60, ItemCategory::LightScrap},
        {"Дзвіночок ресепшена", 2, 50, ItemCategory::LightScrap},
        {"Кухонний ніж", 1, 45, ItemCategory::LightScrap},
        {"Зубна паста", 1, 40, ItemCategory::LightScrap},
        {"Флакон медикаментів", 1, 35, ItemCategory::LightScrap},
        {"Банка газованки", 1, 25, ItemCategory::LightScrap},
        {"Великоднє яйце", 1, 55, ItemCategory::LightScrap},
        {"Портативна рація", 1, 30, ItemCategory::LightScrap},
        {"Корінь мандрагори", 2, 70, ItemCategory::LightScrap},

        // --- 2. Medium Scrap (4-8 kg) - 20 items ---
        {"Старовинний годинник", 5, 70, ItemCategory::MediumScrap},
        {"Фен для волосся", 4, 50, ItemCategory::MediumScrap},
        {"Кавоварка комплексу", 4, 55, ItemCategory::MediumScrap},
        {"Лазерний вказівник", 4, 85, ItemCategory::MediumScrap},
        {"Робот-іграшка", 6, 95, ItemCategory::MediumScrap},
        {"Радіоприймач Бумбокс", 6, 80, ItemCategory::MediumScrap},
        {"Череп гуманоїда", 5, 110, ItemCategory::MediumScrap},
        {"Електричний чайник", 5, 60, ItemCategory::MediumScrap},
        {"Картина в рамці", 7, 120, ItemCategory::MediumScrap},
        {"Антикварна лампа", 8, 100, ItemCategory::MediumScrap},
        {"Маска Трагедії", 4, 85, ItemCategory::MediumScrap},
        {"Маска Комедії", 4, 80, ItemCategory::MediumScrap},
        {"Вогнегасник безпеки", 7, 75, ItemCategory::MediumScrap},
        {"Ліхтарик-маяк", 5, 60, ItemCategory::MediumScrap},
        {"Ручний сканер", 4, 70, ItemCategory::MediumScrap},
        {"Іграшковий вертоліт", 5, 85, ItemCategory::MediumScrap},
        {"Подарункова коробка", 4, 50, ItemCategory::MediumScrap},
        {"Дорожній конус", 4, 25, ItemCategory::MediumScrap},
        {"Пластиковий контейнер", 6, 45, ItemCategory::MediumScrap},
        {"Настінний годинник", 6, 65, ItemCategory::MediumScrap},

        // --- 3. Heavy Scrap (10-18 kg) - 20 items ---
        {"Апарат живлення", 14, 180, ItemCategory::HeavyScrap},
        {"V-подібний двигун", 16, 140, ItemCategory::HeavyScrap},
        {"Велика металева вісь", 15, 110, ItemCategory::HeavyScrap},
        {"Касовий апарат", 17, 160, ItemCategory::HeavyScrap},
        {"Злиток чистого золота", 18, 210, ItemCategory::HeavyScrap},
        {"Хімічний контейнер", 12, 125, ItemCategory::HeavyScrap},
        {"Турбінне колесо", 13, 115, ItemCategory::HeavyScrap},
        {"Промисловий акумулятор", 12, 135, ItemCategory::HeavyScrap},
        {"Вентиляційна турбіна", 11, 95, ItemCategory::HeavyScrap},
        {"Реакторне ядро комплексу", 18, 240, ItemCategory::HeavyScrap},
        {"Балон із пропаном", 12, 90, ItemCategory::HeavyScrap},
        {"Зубчасте колесо механізму", 14, 85, ItemCategory::HeavyScrap},
        {"Броньований сейф", 18, 195, ItemCategory::HeavyScrap},
        {"Промисловий паливний бак", 15, 130, ItemCategory::HeavyScrap},
        {"Балон високого тиску", 13, 105, ItemCategory::HeavyScrap},
        {"Промисловий глушник", 14, 95, ItemCategory::HeavyScrap},
        {"Механічний домкрат", 16, 115, ItemCategory::HeavyScrap},
        {"Гідравлічний насос", 15, 125, ItemCategory::HeavyScrap},
        {"Дворучний сталевий лист", 11, 65, ItemCategory::HeavyScrap},
        {"Електрогенератор струму", 17, 175, ItemCategory::HeavyScrap}
    };
    return templates;
}

struct Item {
    int id = 0;
    std::string name;
    int weight = 1;
    int value = 10;
    Point position{0, 0};
    ItemCategory category = ItemCategory::MediumScrap;
    bool collected = false;

    // Density ratio for Greedy Knapsack algorithm: V_i / W_i
    [[nodiscard]] double getValueDensity() const noexcept {
        if (weight <= 0) {
            return value > 0 ? std::numeric_limits<double>::infinity() : 0.0;
        }
        return static_cast<double>(value) / static_cast<double>(weight);
    }

    bool operator==(const Item& other) const noexcept {
        return id == other.id;
    }
};

} // namespace core
