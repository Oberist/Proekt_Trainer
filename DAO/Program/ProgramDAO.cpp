#include "ProgramDAO.h"
#include <iostream>

ProgramDAO::ProgramDAO(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

ProgramDAO::~ProgramDAO() {
    if (db) sqlite3_close(db);
}

bool ProgramDAO::addProgram(const Program& p) {
    const char* sql = "INSERT INTO Program(name, description) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, p.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, p.description.c_str(), -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

Program ProgramDAO::getProgramById(int id) {
    Program p{};
    const char* sql = "SELECT * FROM Program WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        p.id = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    }
    sqlite3_finalize(stmt);
    return p;
}

std::vector<Program> ProgramDAO::getAllPrograms() {
    std::vector<Program> result;
    const char* sql = "SELECT * FROM Program;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Program p;
        p.id = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.push_back(p);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool ProgramDAO::updateProgram(const Program& p) {
    const char* sql = "UPDATE Program SET name=?, description=? WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, p.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, p.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, p.id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool ProgramDAO::deleteProgram(int id) {
    const char* sql = "DELETE FROM Program WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}