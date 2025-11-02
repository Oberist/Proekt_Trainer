#include "ExercisesEntryDAO.h"
#include <iostream>

ExercisesEntryDAO::ExercisesEntryDAO(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

ExercisesEntryDAO::~ExercisesEntryDAO() {
    if (db) sqlite3_close(db);
}

bool ExercisesEntryDAO::addEntry(const ExercisesEntry& entry) {
    const char* sql = "INSERT INTO ExercisesEntry(training_day_id, exercise_id, sets, reps, weight, rest_time_sec, note) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, entry.training_day_id);
    sqlite3_bind_int(stmt, 2, entry.exercise_id);
    sqlite3_bind_int(stmt, 3, entry.sets);
    sqlite3_bind_int(stmt, 4, entry.reps);
    sqlite3_bind_double(stmt, 5, entry.weight);
    sqlite3_bind_int(stmt, 6, entry.rest_time_sec);
    sqlite3_bind_text(stmt, 7, entry.note.c_str(), -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

ExercisesEntry ExercisesEntryDAO::getEntryById(int id) {
    ExercisesEntry e{};
    const char* sql = "SELECT * FROM ExercisesEntry WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        e.id = sqlite3_column_int(stmt, 0);
        e.training_day_id = sqlite3_column_int(stmt, 1);
        e.exercise_id = sqlite3_column_int(stmt, 2);
        e.sets = sqlite3_column_int(stmt, 3);
        e.reps = sqlite3_column_int(stmt, 4);
        e.weight = sqlite3_column_double(stmt, 5);
        e.rest_time_sec = sqlite3_column_int(stmt, 6);
        e.note = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    }
    sqlite3_finalize(stmt);
    return e;
}

std::vector<ExercisesEntry> ExercisesEntryDAO::getEntriesByTrainingDay(int training_day_id) {
    std::vector<ExercisesEntry> result;
    const char* sql = "SELECT * FROM ExercisesEntry WHERE training_day_id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, training_day_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ExercisesEntry e;
        e.id = sqlite3_column_int(stmt, 0);
        e.training_day_id = sqlite3_column_int(stmt, 1);
        e.exercise_id = sqlite3_column_int(stmt, 2);
        e.sets = sqlite3_column_int(stmt, 3);
        e.reps = sqlite3_column_int(stmt, 4);
        e.weight = sqlite3_column_double(stmt, 5);
        e.rest_time_sec = sqlite3_column_int(stmt, 6);
        e.note = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        result.push_back(e);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool ExercisesEntryDAO::updateEntry(const ExercisesEntry& entry) {
    const char* sql = "UPDATE ExercisesEntry SET training_day_id=?, exercise_id=?, sets=?, reps=?, weight=?, rest_time_sec=?, note=? WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, entry.training_day_id);
    sqlite3_bind_int(stmt, 2, entry.exercise_id);
    sqlite3_bind_int(stmt, 3, entry.sets);
    sqlite3_bind_int(stmt, 4, entry.reps);
    sqlite3_bind_double(stmt, 5, entry.weight);
    sqlite3_bind_int(stmt, 6, entry.rest_time_sec);
    sqlite3_bind_text(stmt, 7, entry.note.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, entry.id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool ExercisesEntryDAO::deleteEntry(int id) {
    const char* sql = "DELETE FROM ExercisesEntry WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}