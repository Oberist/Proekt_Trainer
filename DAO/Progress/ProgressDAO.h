#ifndef PROGRESS_DAO_H
#define PROGRESS_DAO_H

#include <string>
#include <vector>
#include "sqlite3.h"

struct Progress {
    int id;
    int user_id;
    std::string date;
    double weight;
    int total_volume;
    int calendar_id;
};

class ProgressDAO {
private:
    sqlite3* db;

public:
    ProgressDAO(const std::string& dbPath);
    ~ProgressDAO();

    bool addProgress(const Progress& p);
    Progress getProgressById(int id);
    bool updateProgress(const Progress& p);
    bool deleteProgress(int id);
    std::vector<Progress> getAllProgress(); 
};

#endif