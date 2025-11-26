#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "ExerciseDAO.h"
#include "json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static std::string normalizeType(const std::string& t) {
    std::string low = toLower(t);
    if (low == "базовые" || low == "базовое" || low == "basic" || low == "base") return "basic";
    if (low == "изолирующее" || low == "изолирующее" || low == "isolation" || low == "isolate") return "isolation";
    if (low == "кардио" || low == "cardio") return "cardio";
    if (low == "растяжка" || low == "stretch" || low == "stretching") return "stretch";
    return "basic";
}

static bool validType(const std::string& t) {
    std::string low = toLower(t);
    return low == "basic" || low == "isolation" || low == "cardio" || low == "stretch";
}

static std::string joinGroups(const json& node) {
    if (node.is_string()) {
        return node.get<std::string>();
    } else if (node.is_array()) {
        std::string out;
        bool first = true;
        for (const auto& el : node) {
            if (!el.is_string()) continue;
            if (!first) out += "; ";
            out += el.get<std::string>();
            first = false;
        }
        return out;
    }
    return std::string{};
}

int main(int argc, char** argv) {
    const std::string defaultDb = "F:/ProjectTrainer/Database/TrainingDiary.db";
    std::string dbPath = defaultDb;
    std::string jsonPath;

    if (argc >= 2) jsonPath = argv[1];
    else {
        std::cout << "Enter path to JSON file (e.g. F:/ProjectTrainer/Database/import/exercises.json): ";
        std::getline(std::cin, jsonPath);
    }

    if (jsonPath.empty()) {
        std::cerr << "No JSON path provided. Exiting.\n";
        return 1;
    }

    if (!fs::exists(jsonPath)) {
        std::cerr << "JSON file not found: " << jsonPath << "\n";
        return 1;
    }

    ExerciseDAO dao(dbPath);

    std::ifstream in(jsonPath, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonPath << "\n";
        return 1;
    }

    json j;
    try {
        in >> j;
    } catch (const std::exception& ex) {
        std::cerr << "JSON parse error: " << ex.what() << "\n";
        return 1;
    }

    if (!j.is_array()) {
        std::cerr << "JSON root must be an array of exercises.\n";
        return 1;
    }

    int inserted = 0;
    int skipped = 0;
    for (const auto& item : j) {
        if (!item.contains("name") || !item["name"].is_string()) {
            std::cerr << "Skipping entry without valid 'name'.\n";
            ++skipped;
            continue;
        }

        Exercise ex{};
        ex.name = item["name"].get<std::string>();
        ex.description = item.value("description", std::string{});
        ex.difficulty = item.value("difficulty", 1);
        if (ex.difficulty < 1) ex.difficulty = 1;
        if (ex.difficulty > 3) ex.difficulty = 3;

        ex.muscle_group_primary = item.value("muscle_group_primary", std::string{});

        if (item.contains("muscle_group_secondary")) {
            ex.muscle_group_secondary = joinGroups(item["muscle_group_secondary"]);
        } else {
            ex.muscle_group_secondary = std::string{};
        }

        std::string rawType = item.value("type", std::string{"basic"});
        ex.type = normalizeType(rawType);
        if (!validType(ex.type)) {
            std::cerr << "Invalid type for '" << ex.name << "': " << rawType << ". Normalizing to 'basic'.\n";
            ex.type = "basic";
        }

        ex.equipment = item.value("equipment", std::string{});
        ex.image_path = item.value("image_path", std::string{});

        if (!ex.image_path.empty()) {
            fs::path imgPath = ex.image_path;
            if (!imgPath.is_absolute()) {
                fs::path base = fs::path(dbPath).parent_path();
                imgPath = base / ex.image_path;
            }
            if (!fs::exists(imgPath)) {
                std::cerr << "Warning: image not found for '" << ex.name << "' at " << imgPath.string() << "\n";
            }
        }

        bool ok = dao.addExercise(ex);
        if (ok) {
            ++inserted;
            std::cout << "Inserted: " << ex.name << "\n";
        } else {
            ++skipped;
            std::cerr << "Failed to insert: " << ex.name << "\n";
        }
    }

    std::cout << "Import finished. Inserted: " << inserted << ", Skipped/Failed: " << skipped << "\n";
    return 0;
}