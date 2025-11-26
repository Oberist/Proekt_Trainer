#include "TrainingDayManager.h"
#include <iostream>

TrainingDayManager::TrainingDayManager(const std::string& dbPath)
    : tdDao_(dbPath), entryDao_(dbPath) {}

bool TrainingDayManager::createTrainingDay(const TrainingDay& td) {
    bool ok = tdDao_.addTrainingDay(td);
    if (!ok) {
        std::cerr << "TrainingDayManager: failed to add training day\n";
    }
    return ok;
}

bool TrainingDayManager::deleteTrainingDay(int trainingDayId) {
    std::vector<ExercisesEntry> entries = entryDao_.getEntriesByTrainingDay(trainingDayId);
    for (const auto& e : entries) {
        if (!entryDao_.deleteEntry(e.id)) {
            std::cerr << "TrainingDayManager: failed to delete entry id=" << e.id << " for training day " << trainingDayId << "\n";
        }
    }

    bool ok = tdDao_.deleteTrainingDay(trainingDayId);
    if (!ok) {
        std::cerr << "TrainingDayManager: failed to delete training day id=" << trainingDayId << "\n";
    }
    return ok;
}

bool TrainingDayManager::addExerciseEntry(const ExercisesEntry& entry) {
    TrainingDay td = tdDao_.getTrainingDayById(entry.training_day_id);
    if (td.id <= 0) {
        std::cerr << "TrainingDayManager: training day not found id=" << entry.training_day_id << "\n";
        return false;
    }

    bool ok = entryDao_.addEntry(entry);
    if (!ok) {
        std::cerr << "TrainingDayManager: failed to add exercise entry for training day " << entry.training_day_id << "\n";
    }
    return ok;
}

bool TrainingDayManager::removeExerciseEntry(int entryId) {
    bool ok = entryDao_.deleteEntry(entryId);
    if (!ok) {
        std::cerr << "TrainingDayManager: failed to delete exercise entry id=" << entryId << "\n";
    }
    return ok;
}

TrainingDayFull TrainingDayManager::getTrainingDayById(int trainingDayId) {
    TrainingDayFull result;
    TrainingDay td = tdDao_.getTrainingDayById(trainingDayId);
    if (td.id <= 0) {
        return result;
    }
    result.id = td.id;
    result.name = td.name;
    result.date = td.date;
    result.description = ""; 

    result.entries = entryDao_.getEntriesByTrainingDay(trainingDayId);
    return result;
}

std::vector<TrainingDayFull> TrainingDayManager::getAllTrainingDays() {
    std::vector<TrainingDayFull> out;
    std::vector<TrainingDay> days = tdDao_.getAllTrainingDays();
    out.reserve(days.size());
    for (const auto& td : days) {
        TrainingDayFull full;
        full.id = td.id;
        full.name = td.name;
        full.date = td.date;
        full.description = ""; 
        full.entries = entryDao_.getEntriesByTrainingDay(td.id);
        out.push_back(std::move(full));
    }
    return out;
}
