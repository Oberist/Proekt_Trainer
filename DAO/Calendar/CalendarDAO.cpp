#include "CalendarDAO.h"
#include <iostream>

CalendarDAO::CalendarDAO(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

CalendarDAO::~CalendarDAO() {
    if (db) sqlite3_close(db);
}

bool CalendarDAO::addEvent(const Calendar& cal) {
    const char* sql = "INSERT INTO Calendar(user_id, training_day_id, scheduled_date, status, program_id) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, cal.user_id);
    sqlite3_bind_int(stmt, 2, cal.training_day_id);
    sqlite3_bind_text(stmt, 3, cal.scheduled_date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, cal.status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, cal.program_id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

Calendar CalendarDAO::getEventById(int id) {
    Calendar cal{};
    const char* sql = "SELECT * FROM Calendar WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cal.id = sqlite3_column_int(stmt, 0);
        cal.user_id = sqlite3_column_int(stmt, 1);
        cal.training_day_id = sqlite3_column_int(stmt, 2);
        cal.scheduled_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        cal.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        cal.program_id = sqlite3_column_int(stmt, 5);
    }
    sqlite3_finalize(stmt);
    return cal;
}

std::vector<Calendar> CalendarDAO::getEventsByDate(const std::string& date) {
    std::vector<Calendar> result;
    const char* sql = "SELECT * FROM Calendar WHERE scheduled_date=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, date.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Calendar cal;
        cal.id = sqlite3_column_int(stmt, 0);
        cal.user_id = sqlite3_column_int(stmt, 1);
        cal.training_day_id = sqlite3_column_int(stmt, 2);
        cal.scheduled_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        cal.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        cal.program_id = sqlite3_column_int(stmt, 5);
        result.push_back(cal);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool CalendarDAO::updateEvent(const Calendar& cal) {
    const char* sql = "UPDATE Calendar SET user_id=?, training_day_id=?, scheduled_date=?, status=?, program_id=? WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, cal.user_id);
    sqlite3_bind_int(stmt, 2, cal.training_day_id);
    sqlite3_bind_text(stmt, 3, cal.scheduled_date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, cal.status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, cal.program_id);
    sqlite3_bind_int(stmt, 6, cal.id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool CalendarDAO::deleteEvent(int id) {
    const char* sql = "DELETE FROM Calendar WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}
