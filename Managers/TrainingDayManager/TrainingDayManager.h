#ifndef TRAINING_DAY_MANAGER_H
#define TRAINING_DAY_MANAGER_H

#include <string>
#include <vector>
#include "TrainingDayDAO.h"
#include "ExercisesEntryDAO.h"

struct TrainingDayFull {
    int id;
    std::string name;
    std::string date;
    std::string description; 
    std::vector<ExercisesEntry> entries; 
};

class TrainingDayManager {
public:
    explicit TrainingDayManager(const std::string& dbPath);

    bool createTrainingDay(const TrainingDay& td); 
    bool deleteTrainingDay(int trainingDayId);    

    bool addExerciseEntry(const ExercisesEntry& entry); 
    bool removeExerciseEntry(int entryId);              

    TrainingDayFull getTrainingDayById(int trainingDayId);
    std::vector<TrainingDayFull> getAllTrainingDays();

private:
    TrainingDayDAO tdDao_;
    ExercisesEntryDAO entryDao_;
};
#endif 