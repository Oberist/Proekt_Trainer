#ifndef WORKOUT_DAO_H
#define WORKOUT_DAO_H

#include <string>
#include <vector>
#include "sqlite3.h"

struct Workout {
    int id;
    int user_id;
    int training_day_id;
    std::string date;
    int total_volume;
    int duration;
};

class WorkoutDAO {
private:
    sqlite3* db;
public:
    WorkoutDAO(const std::string& dbPath);
    ~WorkoutDAO();

    bool addWorkout(const Workout& w);
    Workout getWorkoutById(int id);
    std::vector<Workout> getWorkoutsByUser(int user_id);
    bool updateWorkout(const Workout& w);
    bool deleteWorkout(int id);
};

#endif