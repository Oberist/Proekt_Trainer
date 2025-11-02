#ifndef TRAININGDAY_DAO_H
#define TRAININGDAY_DAO_H

#include <string>
#include <vector>
#include "sqlite3.h"

struct TrainingDay {
    int id;
    std::string name;
    std::string date;
};

class TrainingDayDAO {
private:
    sqlite3* db;
public:
    TrainingDayDAO(const std::string& dbPath);
    ~TrainingDayDAO();

    bool addTrainingDay(const TrainingDay& td);
    TrainingDay getTrainingDayById(int id);
    std::vector<TrainingDay> getAllTrainingDays();
    bool updateTrainingDay(const TrainingDay& td);
    bool deleteTrainingDay(int id);
};

#endif