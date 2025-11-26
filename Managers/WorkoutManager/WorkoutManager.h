#ifndef WORKOUT_MANAGER_H
#define WORKOUT_MANAGER_H

#include <string>
#include <vector>
#include "WorkoutDAO.h"
#include "ExercisesEntryDAO.h"

struct WorkoutFull {
    Workout workout;
    std::vector<ExercisesEntry> entries; 
};

class WorkoutManager {
public:
    explicit WorkoutManager(const std::string& dbPath);

    bool saveWorkout(Workout w);

    Workout getWorkoutById(int id);

    WorkoutFull getWorkoutFullById(int id);

    std::vector<Workout> getWorkoutsByUser(int userId);

    static int computeTonnage(const std::vector<ExercisesEntry>& entries);

private:
    WorkoutDAO workoutDao_;
    ExercisesEntryDAO entryDao_;
};
#endif 
