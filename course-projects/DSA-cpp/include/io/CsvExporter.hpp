#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace io {

struct BenchmarkRow {
    std::string category;
    std::string algorithmName;
    int mapDimension = 0;
    int steps = 0;
    int energyCost = 0;
    int visitedNodes = 0;
    long long timeMicrosec = 0;
    int itemCount = 0;
    int weightKg = 0;
    int valueCredits = 0;
    double qualityPercent = 0.0;
};

class CsvExporter {
public:
    static bool exportToCsv(const std::string& filepath, const std::vector<BenchmarkRow>& rows) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        file << "Категорія,Алгоритм,Розмір_карти,Кроки,Енергія,Відвідано_вершин,Час_мс,Час_мкс,Предмети,Вага_кг,Цінність,Якість_відсоток\n";

        for (const auto& r : rows) {
            file << "\"" << r.category << "\","
                 << "\"" << r.algorithmName << "\","
                 << r.mapDimension << ","
                 << r.steps << ","
                 << r.energyCost << ","
                 << r.visitedNodes << ","
                 << (static_cast<double>(r.timeMicrosec) / 1000.0) << ","
                 << r.timeMicrosec << ","
                 << r.itemCount << ","
                 << r.weightKg << ","
                 << r.valueCredits << ","
                 << r.qualityPercent << "\n";
        }

        file.close();
        return true;
    }
};

} // namespace io
