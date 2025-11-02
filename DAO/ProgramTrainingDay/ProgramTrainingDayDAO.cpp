#include "ProgramTrainingDayDAO.h"
#include <iostream>

ProgramTrainingDayDAO::ProgramTrainingDayDAO(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

ProgramTrainingDayDAO::~ProgramTrainingDayDAO() {
    if (db) sqlite3_close(db);
}

bool ProgramTrainingDayDAO::addLink(const ProgramTrainingDay& link) {
    const char* sql = "INSERT INTO ProgramTrainingDay(program_id, training_day_id) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, link.program_id);
    sqlite3_bind_int(stmt, 2, link.training_day_id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool ProgramTrainingDayDAO::deleteLink(int program_id, int training_day_id) {
    const char* sql = "DELETE FROM ProgramTrainingDay WHERE program_id=? AND training_day_id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, program_id);
    sqlite3_bind_int(stmt, 2, training_day_id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<ProgramTrainingDay> ProgramTrainingDayDAO::getLinksByProgram(int program_id) {
    std::vector<ProgramTrainingDay> result;
    const char* sql = "SELECT program_id, training_day_id FROM ProgramTrainingDay WHERE program_id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, program_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ProgramTrainingDay link;
        link.program_id = sqlite3_column_int(stmt, 0);
        link.training_day_id = sqlite3_column_int(stmt, 1);
        result.push_back(link);
    }
    sqlite3_finalize(stmt);
    return result;
}