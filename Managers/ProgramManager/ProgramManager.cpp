#include "ProgramManager.h"
#include <iostream>

ProgramManager::ProgramManager(const std::string& dbPath)
    : programDao_(dbPath), ptDao_(dbPath) {}

int ProgramManager::findProgramIdByNameDesc(const std::string& name, const std::string& description) const {
    std::vector<Program> all = programDao_.getAllPrograms();
    int bestId = 0;
    for (const auto& p : all) {
        if (p.name == name && p.description == description) {
            if (p.id > bestId) bestId = p.id;
        }
    }
    return bestId;
}

bool ProgramManager::createProgram(const Program& program, const std::vector<int>& trainingDayIds) {
    bool ok = programDao_.addProgram(program);
    if (!ok) {
        std::cerr << "ProgramManager: failed to add program\n";
        return false;
    }

    int newId = findProgramIdByNameDesc(program.name, program.description);
    if (newId <= 0) {
        std::cerr << "ProgramManager: cannot determine new program id after insert\n";
        return false;
    }

    for (int tdId : trainingDayIds) {
        ProgramTrainingDay link{ newId, tdId };
        if (!ptDao_.addLink(link)) {
            std::cerr << "ProgramManager: failed to link training day " << tdId << " to program " << newId << "\n";
        }
    }
    return true;
}

bool ProgramManager::removeAllTrainingDayLinks(int programId) {
    std::vector<ProgramTrainingDay> links = ptDao_.getLinksByProgram(programId);
    bool allOk = true;
    for (const auto& link : links) {
        if (!ptDao_.deleteLink(link.program_id, link.training_day_id)) {
            std::cerr << "ProgramManager: failed to delete link program=" << link.program_id
                      << " training_day=" << link.training_day_id << "\n";
            allOk = false;
        }
    }
    return allOk;
}

bool ProgramManager::deleteProgram(int programId) {

    if (!removeAllTrainingDayLinks(programId)) {
        std::cerr << "ProgramManager: warning — some links may not have been removed for program " << programId << "\n";
    }
    bool ok = programDao_.deleteProgram(programId);
    if (!ok) {
        std::cerr << "ProgramManager: failed to delete program id=" << programId << "\n";
    }
    return ok;
}

bool ProgramManager::addTrainingDayToProgram(int programId, int trainingDayId) {
    ProgramTrainingDay link{ programId, trainingDayId };
    return ptDao_.addLink(link);
}

bool ProgramManager::removeTrainingDayFromProgram(int programId, int trainingDayId) {
    return ptDao_.deleteLink(programId, trainingDayId);
}

ProgramFull ProgramManager::getProgramById(int programId) const {
    ProgramFull out;
    Program p = programDao_.getProgramById(programId);
    if (p.id <= 0) return out;
    out.id = p.id;
    out.name = p.name;
    out.description = p.description;

    std::vector<ProgramTrainingDay> links = ptDao_.getLinksByProgram(programId);
    out.training_day_ids.reserve(links.size());
    for (const auto& l : links) out.training_day_ids.push_back(l.training_day_id);

    return out;
}

std::vector<ProgramFull> ProgramManager::getAllPrograms() const {
    std::vector<ProgramFull> out;
    std::vector<Program> all = programDao_.getAllPrograms();
    out.reserve(all.size());
    for (const auto& p : all) {
        ProgramFull pf;
        pf.id = p.id;
        pf.name = p.name;
        pf.description = p.description;
        std::vector<ProgramTrainingDay> links = ptDao_.getLinksByProgram(p.id);
        pf.training_day_ids.reserve(links.size());
        for (const auto& l : links) pf.training_day_ids.push_back(l.training_day_id);
        out.push_back(std::move(pf));
    }
    return out;
}