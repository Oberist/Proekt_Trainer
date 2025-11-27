#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <sqlite3.h>
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

static bool upsertExerciseSql(sqlite3* db, const Exercise& ex) {
    const char* selectSql = "SELECT id FROM Exercise WHERE name = ?";
    sqlite3_stmt* selStmt = nullptr;
    int rc = sqlite3_prepare_v2(db, selectSql, -1, &selStmt, nullptr);
    if (rc != SQLITE_OK) {
        if (selStmt) sqlite3_finalize(selStmt);
        return false;
    }
    sqlite3_bind_text(selStmt, 1, ex.name.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(selStmt);
    bool exists = (rc == SQLITE_ROW);
    int64_t existingId = 0;
    if (exists) existingId = sqlite3_column_int64(selStmt, 0);
    sqlite3_finalize(selStmt);

    if (exists) {
        const char* updateSql =
            "UPDATE Exercise SET description = ?, difficulty = ?, muscle_group_primary = ?, muscle_group_secondary = ?, type = ?, equipment = ?, image_path = ? WHERE id = ?";
        sqlite3_stmt* updStmt = nullptr;
        if (sqlite3_prepare_v2(db, updateSql, -1, &updStmt, nullptr) != SQLITE_OK) {
            if (updStmt) sqlite3_finalize(updStmt);
            return false;
        }
        sqlite3_bind_text(updStmt, 1, ex.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(updStmt, 2, ex.difficulty);
        sqlite3_bind_text(updStmt, 3, ex.muscle_group_primary.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(updStmt, 4, ex.muscle_group_secondary.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(updStmt, 5, ex.type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(updStmt, 6, ex.equipment.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(updStmt, 7, ex.image_path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(updStmt, 8, existingId);
        rc = sqlite3_step(updStmt);
        sqlite3_finalize(updStmt);
        return rc == SQLITE_DONE;
    } else {
        const char* insertSql =
            "INSERT INTO Exercise (name, description, difficulty, muscle_group_primary, muscle_group_secondary, type, equipment, image_path) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
        sqlite3_stmt* insStmt = nullptr;
        if (sqlite3_prepare_v2(db, insertSql, -1, &insStmt, nullptr) != SQLITE_OK) {
            if (insStmt) sqlite3_finalize(insStmt);
            return false;
        }
        sqlite3_bind_text(insStmt, 1, ex.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insStmt, 2, ex.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(insStmt, 3, ex.difficulty);
        sqlite3_bind_text(insStmt, 4, ex.muscle_group_primary.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insStmt, 5, ex.muscle_group_secondary.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insStmt, 6, ex.type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insStmt, 7, ex.equipment.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insStmt, 8, ex.image_path.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(insStmt);
        sqlite3_finalize(insStmt);
        return rc == SQLITE_DONE;
    }
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
    in.close();

    if (!j.is_array()) {
        std::cerr << "JSON root must be an array of exercises.\n";
        return 1;
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        std::cerr << "Failed to open DB: " << dbPath << "\n";
        return 1;
    }

    char* errMsg = nullptr;
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
    }

    int inserted = 0;
    int updated = 0;
    int skipped = 0;

    for (const auto& item : j) {
        if (!item.contains("name") || !item["name"].is_string()) {
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
        if (!validType(ex.type)) ex.type = "basic";
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

        bool ok = upsertExerciseSql(db, ex);
        if (ok) {
            const char* checkSql = "SELECT COUNT(1) FROM Exercise WHERE name = ?";
            sqlite3_stmt* chk = nullptr;
            if (sqlite3_prepare_v2(db, checkSql, -1, &chk, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(chk, 1, ex.name.c_str(), -1, SQLITE_TRANSIENT);
                if (sqlite3_step(chk) == SQLITE_ROW) {
                    int cnt = sqlite3_column_int(chk, 0);
                    if (cnt > 0) {
                        ++inserted;
                        std::cout << "Inserted/Updated: " << ex.name << "\n";
                    } else {
                        ++skipped;
                    }
                }
                sqlite3_finalize(chk);
            } else {
                ++skipped;
            }
        } else {
            ++skipped;
            std::cerr << "Failed to upsert: " << ex.name << "\n";
        }
    }

    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
    }

    sqlite3_close(db);

    std::cout << "Import finished. Processed: " << (inserted + skipped) << ", Inserted/Updated: " << inserted << ", Skipped/Failed: " << skipped << "\n";
    return 0;
}
