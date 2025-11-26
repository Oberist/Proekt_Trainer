#ifndef PROGRAM_MANAGER_H
#define PROGRAM_MANAGER_H

#include <string>
#include <vector>
#include "ProgramDAO.h"
#include "ProgramTrainingDayDAO.h"

struct ProgramFull {
    int id;
    std::string name;
    std::string description;
    std::vector<int> training_day_ids;
};

class ProgramManager {
public:
    explicit ProgramManager(const std::string& dbPath);

    bool createProgram(const Program& program, const std::vector<int>& trainingDayIds = {});
    bool deleteProgram(int programId);

    bool addTrainingDayToProgram(int programId, int trainingDayId);
    bool removeTrainingDayFromProgram(int programId, int trainingDayId);

    ProgramFull getProgramById(int programId) const;
    std::vector<ProgramFull> getAllPrograms() const;

private:
    ProgramDAO programDao_;
    ProgramTrainingDayDAO ptDao_;

    int findProgramIdByNameDesc(const std::string& name, const std::string& description) const;
    bool removeAllTrainingDayLinks(int programId);
};

#endif // PROGRAM_MANAGER_H