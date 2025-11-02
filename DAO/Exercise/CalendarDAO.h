#ifndef CALENDAR_DAO_H
#define CALENDAR_DAO_H

#include <string>
#include <vector>
#include "sqlite3.h"

struct Calendar {
    int id;
    int user_id;
    int training_day_id;
    std::string scheduled_date; 
    std::string status; 
    int program_id;
};

class CalendarDAO {
private:
    sqlite3* db;
public:
    CalendarDAO(const std::string& dbPath);
    ~CalendarDAO();

    bool addEvent(const Calendar& cal);
    Calendar getEventById(int id);
    std::vector<Calendar> getEventsByDate(const std::string& date);
    bool updateEvent(const Calendar& cal);
    bool deleteEvent(int id);
};

#endif