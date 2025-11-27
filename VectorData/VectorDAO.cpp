#include "VectorDAO.h"
#include "embedding.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <sqlite3.h>

namespace fs = std::filesystem;

VectorDAO::VectorDAO(const std::string& dbPath, const std::string& indexPath,
                     int dimParam, size_t maxElementsParam)
    : indexPath(indexPath),
      dim(0),
      maxElements(maxElementsParam),
      index(nullptr),
      space(nullptr),
      db(nullptr)
{
    if (dimParam > 0)
        this->dim = static_cast<size_t>(dimParam);

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        throw std::runtime_error("Cannot open SQLite DB");
    }
}

VectorDAO::~VectorDAO() {
    if (index) {
        delete index;
        index = nullptr;
    }
    if (space) {
        delete space;
        space = nullptr;
    }
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

std::string VectorDAO::metaPath() const {
    return indexPath + ".meta";
}

bool VectorDAO::addVectorText(int64_t id, const std::string& text) {
    auto embedding = generateEmbedding(text);

    if (embedding.empty())
        throw std::runtime_error("Embedding is empty");

    if (dim == 0) {
        dim = embedding.size();
        space = new hnswlib::L2Space(static_cast<int>(dim));
        index = new hnswlib::HierarchicalNSW<float>(space, maxElements);
    }

    if (embedding.size() != dim)
        throw std::runtime_error("Embedding dimension mismatch");

    index->addPoint(embedding.data(), static_cast<hnswlib::labeltype>(id));
    return true;
}

std::vector<std::pair<int64_t, float>>
VectorDAO::search(const std::vector<float>& queryEmbedding, int topK) {

    if (!index)
        throw std::runtime_error("Index not initialized");

    if (queryEmbedding.size() != dim)
        throw std::runtime_error("Query embedding dimension mismatch");

    std::vector<std::pair<int64_t, float>> results;

    auto res = index->searchKnn(queryEmbedding.data(), topK);
    while (!res.empty()) {
        auto p = res.top();
        results.emplace_back(static_cast<int64_t>(p.second), p.first);
        res.pop();
    }
    return results;
}

bool VectorDAO::saveIndex() {
    if (!index) {
        std::cerr << "No index to save\n";
        return false;
    }

    try {
        index->saveIndex(indexPath);

        std::ofstream meta(metaPath(), std::ios::out | std::ios::trunc);
        if (!meta.is_open()) {
            std::cerr << "Warning: cannot write meta file\n";
            return true;
        }

        meta << "dim=" << dim << "\n";
        meta << "max_elements=" << maxElements << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Exception while saving index: " << e.what() << "\n";
        return false;
    }

    return true;
}

bool VectorDAO::loadIndex() {
    std::string mpath = metaPath();
    if (fs::exists(mpath)) {
        std::ifstream meta(mpath);
        if (!meta.is_open()) {
            std::cerr << "Failed to open meta\n";
            return false;
        }
        std::string line;
        while (std::getline(meta, line)) {
            if (line.rfind("dim=", 0) == 0)
                dim = std::stoul(line.substr(4));
            else if (line.rfind("max_elements=", 0) == 0)
                maxElements = std::stoul(line.substr(13));
        }
    }

    if (dim == 0) {
        std::cerr << "Cannot load index: dim == 0\n";
        return false;
    }

    if (!space)
        space = new hnswlib::L2Space(static_cast<int>(dim));

    if (index) {
        delete index;
        index = nullptr;
    }

    try {
        index = new hnswlib::HierarchicalNSW<float>(space, maxElements);

        index->loadIndex(indexPath, space, maxElements);

    } catch (const std::exception& e) {
        std::cerr << "Exception while loading index: " << e.what() << "\n";
        if (index) delete index;
        index = nullptr;
        return false;
    }

    return true;
}

bool VectorDAO::saveMetadata(int64_t indexId, const std::string& sourceTable,
                             int64_t sourceId, const std::string& text) {

    const char* sql =
        "INSERT INTO VectorMetadata (index_id, source_table, source_id, text) "
        "VALUES (?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement: "
                  << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_int64(stmt, 1, indexId);
    sqlite3_bind_text(stmt, 2, sourceTable.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, sourceId);
    sqlite3_bind_text(stmt, 4, text.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Error inserting metadata: "
                  << sqlite3_errmsg(db) << "\n";
        return false;
    }

    return true;
}

bool VectorDAO::getMetadata(int64_t indexId, std::string& metadata) {
    const char* sql =
        "SELECT text FROM VectorMetadata WHERE index_id=?";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement: "
                  << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_int64(stmt, 1, indexId);

    int rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        const unsigned char* txt = sqlite3_column_text(stmt, 0);
        metadata = txt ? reinterpret_cast<const char*>(txt) : "";
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_ROW;
}


