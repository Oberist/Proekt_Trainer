#ifndef PROGRAMTRAININGDAY_DAO_H
#define PROGRAMTRAININGDAY_DAO_H

#include <vector>
#include "sqlite3.h"

struct ProgramTrainingDay {
    int program_id;
    int training_day_id;
};

class ProgramTrainingDayDAO {
private:
    sqlite3* db;
public:
    ProgramTrainingDayDAO(const std::string& dbPath);
    ~ProgramTrainingDayDAO();

    bool addLink(const ProgramTrainingDay& link);
    bool deleteLink(int program_id, int training_day_id);
    std::vector<ProgramTrainingDay> getLinksByProgram(int program_id);
};

#endif