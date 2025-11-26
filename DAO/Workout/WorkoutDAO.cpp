#include "WorkoutDAO.h"
#include <iostream>

WorkoutDAO::WorkoutDAO(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

WorkoutDAO::~WorkoutDAO() {
    if (db) sqlite3_close(db);
}

bool WorkoutDAO::addWorkout(const Workout& w) {
    const char* sql = "INSERT INTO Workout(user_id, training_day_id, date, total_volume, duration) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, w.user_id);
    sqlite3_bind_int(stmt, 2, w.training_day_id);
    sqlite3_bind_text(stmt, 3, w.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, w.total_volume);
    sqlite3_bind_int(stmt, 5, w.duration);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

Workout WorkoutDAO::getWorkoutById(int id) {
    Workout w{};
    const char* sql = "SELECT * FROM Workout WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        w.id = sqlite3_column_int(stmt, 0);
        w.user_id = sqlite3_column_int(stmt, 1);
        w.training_day_id = sqlite3_column_int(stmt, 2);
        w.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        w.total_volume = sqlite3_column_int(stmt, 4);
        w.duration = sqlite3_column_int(stmt, 5);
    }
    sqlite3_finalize(stmt);
    return w;
}

std::vector<Workout> WorkoutDAO::getWorkoutsByUser(int user_id) {
    std::vector<Workout> result;
    const char* sql = "SELECT * FROM Workout WHERE user_id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Workout w;
        w.id = sqlite3_column_int(stmt, 0);
        w.user_id = sqlite3_column_int(stmt, 1);
        w.training_day_id = sqlite3_column_int(stmt, 2);
        w.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        w.total_volume = sqlite3_column_int(stmt, 4);
        w.duration = sqlite3_column_int(stmt, 5);
        result.push_back(w);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool WorkoutDAO::updateWorkout(const Workout& w) {
    const char* sql = "UPDATE Workout SET user_id=?, training_day_id=?, date=?, total_volume=?, duration=? WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, w.user_id);
    sqlite3_bind_int(stmt, 2, w.training_day_id);
    sqlite3_bind_text(stmt, 3, w.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, w.total_volume);
    sqlite3_bind_int(stmt, 5, w.duration);
    sqlite3_bind_int(stmt, 6, w.id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool WorkoutDAO::deleteWorkout(int id) {
    const char* sql = "DELETE FROM Workout WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}
