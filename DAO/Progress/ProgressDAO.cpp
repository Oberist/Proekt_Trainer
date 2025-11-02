#include "ProgressDAO.h"
#include <iostream>

ProgressDAO::ProgressDAO(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

ProgressDAO::~ProgressDAO() {
    if (db) sqlite3_close(db);
}

bool ProgressDAO::addProgress(const Progress& p) {
    const char* sql = "INSERT INTO Progress(user_id, date, weight, total_volume, calendar_id) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, p.user_id);
    sqlite3_bind_text(stmt, 2, p.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, p.weight);
    sqlite3_bind_int(stmt, 4, p.total_volume);
    sqlite3_bind_int(stmt, 5, p.calendar_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Ошибка выполнения запроса: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

Progress ProgressDAO::getProgressById(int id) {
    Progress p{};
    const char* sql = "SELECT id, user_id, date, weight, total_volume, calendar_id FROM Progress WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db) << std::endl;
        return p;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        p.id = sqlite3_column_int(stmt, 0);
        p.user_id = sqlite3_column_int(stmt, 1);
        p.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.weight = sqlite3_column_double(stmt, 3);
        p.total_volume = sqlite3_column_int(stmt, 4);
        p.calendar_id = sqlite3_column_int(stmt, 5);
    }

    sqlite3_finalize(stmt);
    return p;
}

bool ProgressDAO::updateProgress(const Progress& p) {
    const char* sql = "UPDATE Progress SET user_id = ?, date = ?, weight = ?, total_volume = ?, calendar_id = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, p.user_id);
    sqlite3_bind_text(stmt, 2, p.date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, p.weight);
    sqlite3_bind_int(stmt, 4, p.total_volume);
    sqlite3_bind_int(stmt, 5, p.calendar_id);
    sqlite3_bind_int(stmt, 6, p.id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Ошибка выполнения запроса: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ProgressDAO::deleteProgress(int id) {
    const char* sql = "DELETE FROM Progress WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Ошибка выполнения запроса: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

std::vector<Progress> ProgressDAO::getAllProgress() {
    std::vector<Progress> result;
    const char* sql = "SELECT id, user_id, date, weight, total_volume, calendar_id FROM Progress;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Progress p;
        p.id = sqlite3_column_int(stmt, 0);
        p.user_id = sqlite3_column_int(stmt, 1);
        p.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.weight = sqlite3_column_double(stmt, 3);
        p.total_volume = sqlite3_column_int(stmt, 4);
        p.calendar_id = sqlite3_column_int(stmt, 5);
        result.push_back(p);
    }

    sqlite3_finalize(stmt);
    return result;
}