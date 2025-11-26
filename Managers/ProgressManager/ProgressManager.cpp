#include "ProgressManager.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>
#include <iostream>

ProgressManager::ProgressManager(const std::string& dbPath)
    : dao_(dbPath) { }

bool ProgressManager::saveProgress(const Progress& p) {
    bool ok = dao_.addProgress(p);
    if (!ok) {
        std::cerr << "ProgressManager: failed to add progress for user " << p.user_id << " date " << p.date << "\n";
    }
    return ok;
}

Progress ProgressManager::getProgressById(int id) {
    return dao_.getProgressById(id);
}

bool ProgressManager::updateProgress(const Progress& p) {
    bool ok = dao_.updateProgress(p);
    if (!ok) {
        std::cerr << "ProgressManager: failed to update progress id=" << p.id << "\n";
    }
    return ok;
}

bool ProgressManager::deleteProgress(int id) {
    bool ok = dao_.deleteProgress(id);
    if (!ok) {
        std::cerr << "ProgressManager: failed to delete progress id=" << id << "\n";
    }
    return ok;
}

std::vector<Progress> ProgressManager::getAllProgressForUser(int userId,
                                                             const std::optional<std::string>& fromDate,
                                                             const std::optional<std::string>& toDate) {
    std::vector<Progress> out;
    std::vector<Progress> all = dao_.getAllProgress();
    out.reserve(all.size());
    for (const auto& p : all) {
        if (p.user_id != userId) continue;
        if (!dateInRange(p.date, fromDate, toDate)) continue;
        out.push_back(p);
    }
    std::sort(out.begin(), out.end(), [](const Progress& a, const Progress& b){ return a.date < b.date; });
    return out;
}

ProgressStats ProgressManager::computeStatistics(int userId,
                                                 const std::optional<std::string>& fromDate,
                                                 const std::optional<std::string>& toDate) {
    ProgressStats stats;
    stats.user_id = userId;
    stats.from_date = fromDate;
    stats.to_date = toDate;

    std::vector<Progress> records = getAllProgressForUser(userId, fromDate, toDate);
    stats.count = static_cast<int>(records.size());
    if (records.empty()) return stats;

    double sumWeight = 0.0;
    double sumWeightSq = 0.0;
    double minW = std::numeric_limits<double>::infinity();
    double maxW = -std::numeric_limits<double>::infinity();
    long long sumVolume = 0;

    for (const auto& r : records) {
        double w = r.weight;
        sumWeight += w;
        sumWeightSq += w * w;
        if (w < minW) minW = w;
        if (w > maxW) maxW = w;
        sumVolume += static_cast<long long>(r.total_volume);
    }

    stats.avg_weight = sumWeight / records.size();
    stats.min_weight = (minW == std::numeric_limits<double>::infinity()) ? 0.0 : minW;
    stats.max_weight = (maxW == -std::numeric_limits<double>::infinity()) ? 0.0 : maxW;
    if (records.size() > 1) {
        double mean = stats.avg_weight;
        double variance = (sumWeightSq - records.size() * mean * mean) / (records.size() - 1);
        stats.weight_stddev = (variance > 0.0) ? std::sqrt(variance) : 0.0;
    } else {
        stats.weight_stddev = 0.0;
    }

    stats.total_volume = sumVolume;
    stats.avg_daily_volume = static_cast<double>(sumVolume) / static_cast<double>(records.size());

    return stats;
}

bool ProgressManager::dateInRange(const std::string& date,
                                  const std::optional<std::string>& fromDate,
                                  const std::optional<std::string>& toDate) {
    if (fromDate && date < *fromDate) return false;
    if (toDate && date > *toDate) return false;
    return true;
}
