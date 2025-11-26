#include "UsersDao.h"
#include <iostream>

UsersDAO::UsersDAO(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

UsersDAO::~UsersDAO() {
    if (db) sqlite3_close(db);
}

bool UsersDAO::addUser(const User& user) {
    const char* sql = "INSERT INTO Users(name, weight, height) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, user.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, user.weight);
    sqlite3_bind_double(stmt, 3, user.height);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

User UsersDAO::getUserById(int id) {
    const char* sql = "SELECT * FROM Users WHERE id = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    User user{};
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.weight = sqlite3_column_double(stmt, 2);
        user.height = sqlite3_column_double(stmt, 3);
    }
    sqlite3_finalize(stmt);
    return user;
}

bool UsersDAO::updateUser(const User& user) {
    const char* sql = "UPDATE Users SET name=?, weight=?, height=? WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, user.weight);
    sqlite3_bind_double(stmt, 3, user.height);
    sqlite3_bind_int(stmt, 4, user.id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool UsersDAO::deleteUser(int id) {
    const char* sql = "DELETE FROM Users WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}
