#include "TrainingDayDAO.h"
#include <iostream>

TrainingDayDAO::TrainingDayDAO(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

TrainingDayDAO::~TrainingDayDAO() {
    if (db) sqlite3_close(db);
}

bool TrainingDayDAO::addTrainingDay(const TrainingDay& td) {
    const char* sql = "INSERT INTO TrainingDay(name, date) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, td.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, td.date.c_str(), -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

TrainingDay TrainingDayDAO::getTrainingDayById(int id) {
    TrainingDay td{};
    const char* sql = "SELECT * FROM TrainingDay WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        td.id = sqlite3_column_int(stmt, 0);
        td.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        td.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    }
    sqlite3_finalize(stmt);
    return td;
}

std::vector<TrainingDay> TrainingDayDAO::getAllTrainingDays() {
    std::vector<TrainingDay> result;
    const char* sql = "SELECT * FROM TrainingDay;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TrainingDay td;
        td.id = sqlite3_column_int(stmt, 0);
        td.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        td.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.push_back(td);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool TrainingDayDAO::updateTrainingDay(const TrainingDay& td) {
    const char* sql = "UPDATE TrainingDay SET name=?, date=? WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, td.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, td.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, td.id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool TrainingDayDAO::deleteTrainingDay(int id) {
    const char* sql = "DELETE FROM TrainingDay WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}