#include "ExerciseEntryManager.h"
#include <iostream>

ExerciseEntryManager::ExerciseEntryManager(const std::string& dbPath)
    : dao_(dbPath) {}

bool ExerciseEntryManager::createEntry(const ExercisesEntry& entry) {
    bool ok = dao_.addEntry(entry);
    if (!ok) {
        std::cerr << "ExerciseEntryManager: failed to add entry\n";
    }
    return ok;
}

ExercisesEntry ExerciseEntryManager::getEntryById(int id) const {
    return dao_.getEntryById(id);
}

std::vector<ExercisesEntry> ExerciseEntryManager::getEntriesByTrainingDay(int trainingDayId) const {
    return dao_.getEntriesByTrainingDay(trainingDayId);
}

bool ExerciseEntryManager::updateEntry(const ExercisesEntry& entry) {
    bool ok = dao_.updateEntry(entry);
    if (!ok) {
        std::cerr << "ExerciseEntryManager: failed to update entry id=" << entry.id << "\n";
    }
    return ok;
}

bool ExerciseEntryManager::deleteEntry(int id) {
    bool ok = dao_.deleteEntry(id);
    if (!ok) {
        std::cerr << "ExerciseEntryManager: failed to delete entry id=" << id << "\n";
    }
    return ok;
}

bool ExerciseEntryManager::modifyAndSave(int entryId, const std::function<void(ExercisesEntry&)>& modifier) {
    ExercisesEntry e = dao_.getEntryById(entryId);
    if (e.id <= 0) {
        std::cerr << "ExerciseEntryManager: entry not found id=" << entryId << "\n";
        return false;
    }
    modifier(e);
    return updateEntry(e);
}

bool ExerciseEntryManager::setSets(int entryId, int sets) {
    if (sets < 0) {
        std::cerr << "ExerciseEntryManager: invalid sets value\n";
        return false;
    }
    return modifyAndSave(entryId, [sets](ExercisesEntry& e){ e.sets = sets; });
}

bool ExerciseEntryManager::setReps(int entryId, int reps) {
    if (reps < 0) {
        std::cerr << "ExerciseEntryManager: invalid reps value\n";
        return false;
    }
    return modifyAndSave(entryId, [reps](ExercisesEntry& e){ e.reps = reps; });
}

bool ExerciseEntryManager::setWeight(int entryId, double weight) {
    if (weight < 0.0) {
        std::cerr << "ExerciseEntryManager: invalid weight value\n";
        return false;
    }
    return modifyAndSave(entryId, [weight](ExercisesEntry& e){ e.weight = weight; });
}

bool ExerciseEntryManager::setRestTime(int entryId, int restTimeSec) {
    if (restTimeSec < 0) {
        std::cerr << "ExerciseEntryManager: invalid rest time value\n";
        return false;
    }
    return modifyAndSave(entryId, [restTimeSec](ExercisesEntry& e){ e.rest_time_sec = restTimeSec; });
}

bool ExerciseEntryManager::setNote(int entryId, const std::string& note) {
    return modifyAndSave(entryId, [&note](ExercisesEntry& e){ e.note = note; });
}