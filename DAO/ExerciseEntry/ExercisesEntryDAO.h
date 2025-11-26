#ifndef EXERCISESENTRY_DAO_H
#define EXERCISESENTRY_DAO_H

#include <string>
#include <vector>
#include "sqlite3.h"

struct ExercisesEntry {
    int id;
    int training_day_id;
    int exercise_id;
    int sets;
    int reps;
    double weight;
    int rest_time_sec;
    std::string note;
};

class ExercisesEntryDAO {
private:
    sqlite3* db;
public:
    ExercisesEntryDAO(const std::string& dbPath);
    ~ExercisesEntryDAO();

    bool addEntry(const ExercisesEntry& entry);
    ExercisesEntry getEntryById(int id) const;
    std::vector<ExercisesEntry> getEntriesByTrainingDay(int training_day_id) const;
    bool updateEntry(const ExercisesEntry& entry);
    bool deleteEntry(int id);
};

#endif
