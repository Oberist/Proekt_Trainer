#ifndef PROGRESS_MANAGER_H
#define PROGRESS_MANAGER_H

#include <string>
#include <vector>
#include <optional>
#include "ProgressDAO.h"

struct ProgressStats {
    int user_id;
    std::optional<std::string> from_date; 
    std::optional<std::string> to_date;   

    double avg_weight = 0.0;
    double min_weight = 0.0;
    double max_weight = 0.0;
    double weight_stddev = 0.0; 
    long long total_volume = 0; 
    double avg_daily_volume = 0.0; 
    int count = 0; 
};

class ProgressManager {
public:
    explicit ProgressManager(const std::string& dbPath);

    bool saveProgress(const Progress& p);      
    Progress getProgressById(int id);
    bool updateProgress(const Progress& p);
    bool deleteProgress(int id);

    std::vector<Progress> getAllProgressForUser(int userId,
                                               const std::optional<std::string>& fromDate = std::nullopt,
                                               const std::optional<std::string>& toDate = std::nullopt);

    ProgressStats computeStatistics(int userId,
                                    const std::optional<std::string>& fromDate = std::nullopt,
                                    const std::optional<std::string>& toDate = std::nullopt);

private:
    ProgressDAO dao_;

    static bool dateInRange(const std::string& date,
                            const std::optional<std::string>& fromDate,
                            const std::optional<std::string>& toDate);
};
#endif
