#ifndef EXERCISE_MANAGER_H
#define EXERCISE_MANAGER_H

#include "ExerciseDAO.h"
#include <vector>
#include <string>

class ExerciseManager {
private:
    ExerciseDAO dao;                 
    std::vector<Exercise> exercises;  

public:
    ExerciseManager(const std::string& dbPath);

    void loadExercises();

    const std::vector<Exercise>& getAllExercises() const;

    std::vector<Exercise> searchByName(const std::string& name) const;

    std::vector<Exercise> filterByDifficulty(int difficulty) const;

    std::vector<Exercise> filterByType(const std::string& type) const;

    std::vector<Exercise> filterByMuscleGroup(const std::string& muscleGroup) const;
};

#endif