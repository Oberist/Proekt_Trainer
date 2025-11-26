#ifndef EXERCISE_DAO_H
#define EXERCISE_DAO_H

#include <string>
#include <vector>
#include "sqlite3.h"

struct Exercise {
    int id;
    std::string name;
    std::string description;
    int difficulty;
    std::string muscle_group_primary;
    std::string muscle_group_secondary;
    std::string type;
    std::string equipment;
    std::string image_path;
};

class ExerciseDAO {
private:
    sqlite3* db;
public:
    ExerciseDAO(const std::string& dbPath);
    ~ExerciseDAO();

    bool addExercise(const Exercise& ex);
    Exercise getExerciseById(int id);
    std::vector<Exercise> getAllExercises();
    bool updateExercise(const Exercise& ex);
    bool deleteExercise(int id);
};

#endif
