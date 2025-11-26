#ifndef PROGRAM_DAO_H
#define PROGRAM_DAO_H

#include <string>
#include <vector>
#include "sqlite3.h"

struct Program {
    int id;
    std::string name;
    std::string description;
};

class ProgramDAO {
private:
    sqlite3* db;
public:
    ProgramDAO(const std::string& dbPath);
    ~ProgramDAO();

    bool addProgram(const Program& p);
    Program getProgramById(int id) const;
    std::vector<Program> getAllPrograms() const;
    bool updateProgram(const Program& p);
    bool deleteProgram(int id);
};

#endif
