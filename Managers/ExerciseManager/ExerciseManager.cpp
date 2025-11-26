#include "ExerciseManager.h"
#include <algorithm>

ExerciseManager::ExerciseManager(const std::string& dbPath)
    : dao(dbPath)
{
    loadExercises(); 
}

void ExerciseManager::loadExercises() {
    exercises = dao.getAllExercises();
}

const std::vector<Exercise>& ExerciseManager::getAllExercises() const {
    return exercises;
}

std::vector<Exercise> ExerciseManager::searchByName(const std::string& name) const {
    std::vector<Exercise> result;
    for (const auto& ex : exercises) {
        if (ex.name.find(name) != std::string::npos) { 
            result.push_back(ex);
        }
    }
    return result;
}

std::vector<Exercise> ExerciseManager::filterByDifficulty(int difficulty) const {
    std::vector<Exercise> result;
    for (const auto& ex : exercises) {
        if (ex.difficulty == difficulty) {
            result.push_back(ex);
        }
    }
    return result;
}

std::vector<Exercise> ExerciseManager::filterByType(const std::string& type) const {
    std::vector<Exercise> result;
    for (const auto& ex : exercises) {
        if (ex.type == type) {
            result.push_back(ex);
        }
    }
    return result;
}

std::vector<Exercise> ExerciseManager::filterByMuscleGroup(const std::string& muscleGroup) const {
    std::vector<Exercise> result;
    for (const auto& ex : exercises) {
        if (ex.muscle_group_primary == muscleGroup || ex.muscle_group_secondary == muscleGroup) {
            result.push_back(ex);
        }
    }
    return result;
}