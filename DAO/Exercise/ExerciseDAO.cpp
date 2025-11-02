#include "ExerciseDAO.h"
#include <iostream>

ExerciseDAO::ExerciseDAO(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

ExerciseDAO::~ExerciseDAO() {
    if (db) sqlite3_close(db);
}

bool ExerciseDAO::addExercise(const Exercise& ex) {
    const char* sql = "INSERT INTO Exercise(name, description, difficulty, muscle_group_primary, muscle_group_secondary, type, equipment, image_path) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, ex.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ex.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, ex.difficulty);
    sqlite3_bind_text(stmt, 4, ex.muscle_group_primary.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, ex.muscle_group_secondary.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, ex.type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, ex.equipment.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, ex.image_path.c_str(), -1, SQLITE_STATIC);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

Exercise ExerciseDAO::getExerciseById(int id) {
    Exercise ex{};
    const char* sql = "SELECT * FROM Exercise WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        ex.id = sqlite3_column_int(stmt, 0);
        ex.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        ex.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        ex.difficulty = sqlite3_column_int(stmt, 3);
        ex.muscle_group_primary = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        ex.muscle_group_secondary = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        ex.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        ex.equipment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        ex.image_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
    }
    sqlite3_finalize(stmt);
    return ex;
}

std::vector<Exercise> ExerciseDAO::getAllExercises() {
    std::vector<Exercise> result;
    const char* sql = "SELECT * FROM Exercise;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Exercise ex;
        ex.id = sqlite3_column_int(stmt, 0);
        ex.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        ex.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        ex.difficulty = sqlite3_column_int(stmt, 3);
        ex.muscle_group_primary = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        ex.muscle_group_secondary = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        ex.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        ex.equipment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        ex.image_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        result.push_back(ex);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool ExerciseDAO::updateExercise(const Exercise& ex) {
    const char* sql = "UPDATE Exercise SET name=?, description=?, difficulty=?, muscle_group_primary=?, muscle_group_secondary=?, type=?, equipment=?, image_path=? WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, ex.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ex.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, ex.difficulty);
    sqlite3_bind_text(stmt, 4, ex.muscle_group_primary.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, ex.muscle_group_secondary.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, ex.type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, ex.equipment.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, ex.image_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 9, ex.id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool ExerciseDAO::deleteExercise(int id) {
    const char* sql = "DELETE FROM Exercise WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}