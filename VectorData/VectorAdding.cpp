#include "VectorDAO.h"
#include "embedding.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

static fs::path getExecutableDir(const char* argv0) {
    try {
        fs::path p(argv0);
        if (p.is_relative()) p = fs::current_path() / p;
        p = fs::weakly_canonical(p);
        return p.parent_path();
    } catch (...) {
        return fs::current_path();
    }
}

static int64_t getNextIndexId(const std::string& dbPath) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        throw std::runtime_error("Cannot open SQLite DB to get next index_id");
    }

    const char* sql = "SELECT COALESCE(MAX(index_id), 0) FROM VectorMetadata";
    sqlite3_stmt* stmt = nullptr;
    int64_t nextId = 1;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            int64_t maxId = sqlite3_column_int64(stmt, 0);
            nextId = maxId + 1;
        }
        sqlite3_finalize(stmt);
    } else {
        sqlite3_close(db);
        throw std::runtime_error("Failed to prepare statement to get next index_id");
    }

    sqlite3_close(db);
    return nextId;
}

int main(int argc, char** argv) {
    std::string inputPath;
    if (argc >= 2) {
        inputPath = argv[1];
    } else {
        std::cout << "Enter full path to the text file to add:\n> ";
        std::getline(std::cin, inputPath);
    }

    if (inputPath.empty()) {
        std::cerr << "No path provided. Exiting.\n";
        return 1;
    }

    fs::path exeDir = getExecutableDir(argv[0]);
    fs::path projectRoot = exeDir.parent_path();
    fs::path dbPath   = projectRoot / "Database" / "TrainingDiaryVector.db";
    fs::path indexBin = projectRoot / "Database" / "import" / "vector_index.bin";

    VectorDAO dao(dbPath.string(), indexBin.string(), -1);

    std::ifstream file(inputPath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << inputPath << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();
    file.close();

    if (text.empty()) {
        std::cerr << "File is empty: " << inputPath << std::endl;
        return 1;
    }

    int64_t newId = 0;
    try {
        newId = getNextIndexId(dbPath.string());
    } catch (const std::exception& e) {
        std::cerr << "Failed to get next index_id: " << e.what() << std::endl;
        return 1;
    }

    try {
        if (!dao.addVectorText(newId, text)) {
            std::cerr << "Failed to add text to index." << std::endl;
            return 1;
        }

        if (!dao.saveMetadata(newId, "Files", newId, text)) {
            std::cerr << "Warning: saveMetadata returned false." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to add text to index: " << e.what() << std::endl;
        return 1;
    }

    if (!dao.saveIndex()) {
        std::cerr << "Failed to save index to: " << indexBin.string() << std::endl;
        return 1;
    }

    if (!dao.loadIndex()) {
        std::cerr << "Failed to load index from: " << indexBin.string() << std::endl;
        return 1;
    }

    return 0;
}
