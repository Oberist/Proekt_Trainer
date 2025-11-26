#ifndef EXERCISE_ENTRY_MANAGER_H
#define EXERCISE_ENTRY_MANAGER_H

#include <string>
#include <vector>
#include <functional>
#include "ExercisesEntryDAO.h" 

class ExerciseEntryManager {
public:
    explicit ExerciseEntryManager(const std::string& dbPath);

    bool createEntry(const ExercisesEntry& entry);
    ExercisesEntry getEntryById(int id) const;
    std::vector<ExercisesEntry> getEntriesByTrainingDay(int trainingDayId) const;
    bool updateEntry(const ExercisesEntry& entry);
    bool deleteEntry(int id);

    bool setSets(int entryId, int sets);
    bool setReps(int entryId, int reps);
    bool setWeight(int entryId, double weight);
    bool setRestTime(int entryId, int restTimeSec);
    bool setNote(int entryId, const std::string& note);

private:
    ExercisesEntryDAO dao_;

    bool modifyAndSave(int entryId, const std::function<void(ExercisesEntry&)>& modifier);
};

#endif 