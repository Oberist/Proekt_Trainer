#include "WorkoutManager.h"
#include <iostream>
#include <limits>  
#include <cmath>    
WorkoutManager::WorkoutManager(const std::string& dbPath)
    : workoutDao_(dbPath), entryDao_(dbPath) {}

int WorkoutManager::computeTonnage(const std::vector<ExercisesEntry>& entries) {
    double total = 0.0;
    for (const auto& e : entries) {
        total += static_cast<double>(e.sets) * static_cast<double>(e.reps) * e.weight;
    }

    double intMax = static_cast<double>(std::numeric_limits<int>::max());
    if (total >= intMax) {
        std::cerr << "WorkoutManager::computeTonnage: total volume overflow, capping to INT_MAX\n";
        return std::numeric_limits<int>::max();
    }

    return static_cast<int>(total);
}

bool WorkoutManager::saveWorkout(Workout w) {
    std::vector<ExercisesEntry> entries = entryDao_.getEntriesByTrainingDay(w.training_day_id);
    int totalVolume = computeTonnage(entries);
    w.total_volume = totalVolume;

    bool ok = workoutDao_.addWorkout(w);
    if (!ok) {
        std::cerr << "WorkoutManager: failed to add workout for user " << w.user_id << " on " << w.date << "\n";
    }
    return ok;
}

Workout WorkoutManager::getWorkoutById(int id) {
    return workoutDao_.getWorkoutById(id);
}

WorkoutFull WorkoutManager::getWorkoutFullById(int id) {
    WorkoutFull out;
    out.workout = workoutDao_.getWorkoutById(id);
    if (out.workout.id <= 0) return out;
    out.entries = entryDao_.getEntriesByTrainingDay(out.workout.training_day_id);
    return out;
}

std::vector<Workout> WorkoutManager::getWorkoutsByUser(int userId) {
    return workoutDao_.getWorkoutsByUser(userId);
}
