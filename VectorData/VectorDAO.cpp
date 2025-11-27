#include "VectorDAO.h"
#include "embedding.h"
#include <stdexcept>
#include <iostream>
#include <sqlite3.h>

VectorDAO::VectorDAO(const std::string& dbPath, const std::string& indexPath, int dim)
    : indexPath(indexPath), dim(0), index(nullptr), space(nullptr), db(nullptr)
{
    if (dim > 0) this->dim = static_cast<std::size_t>(dim);
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        throw std::runtime_error("Cannot open SQLite DB");
    }
}

VectorDAO::~VectorDAO() {
    if (index) delete index;
    if (space) delete space;
    if (db) sqlite3_close(db);
}

bool VectorDAO::addVectorText(int64_t id, const std::string& text) {
    auto embedding = generateEmbedding(text);

    if (embedding.empty()) {
        throw std::runtime_error("Embedding is empty, model did not return vector");
    }

    if (dim == 0) {
        dim = embedding.size();
        space = new hnswlib::L2Space(static_cast<int>(dim));
        index = new hnswlib::HierarchicalNSW<float>(space, 10000, 16, 200);
    }

    if (embedding.size() != dim) {
        throw std::runtime_error("Embedding dimension mismatch: got " +
            std::to_string(embedding.size()) + ", expected " + std::to_string(dim));
    }

    index->addPoint(embedding.data(), id);
    return saveMetadata(id, "Documents", id, text);
}

std::vector<std::pair<int64_t, float>> VectorDAO::search(const std::vector<float>& queryEmbedding, int topK) {
    if (!index) throw std::runtime_error("Index not initialized yet");

    if (queryEmbedding.size() != dim) {
        throw std::runtime_error("Query embedding dimension mismatch");
    }

    std::vector<std::pair<int64_t, float>> results;
    auto res = index->searchKnn(queryEmbedding.data(), topK);
    while (!res.empty()) {
        results.push_back({res.top().second, res.top().first});
        res.pop();
    }
    return results;
}

bool VectorDAO::saveIndex() {
    if (!index) return false;
    index->saveIndex(indexPath);
    return true;
}

bool VectorDAO::loadIndex() {
    if (dim == 0) {
        throw std::runtime_error("Dimension unknown, cannot initialize space to load index");
    }
    if (!space) {
        space = new hnswlib::L2Space(static_cast<int>(dim));
    }
    if (!index) {
        index = new hnswlib::HierarchicalNSW<float>(space, 10000, 16, 200);
    }
    index->loadIndex(indexPath, space, 10000);
    return true;
}

bool VectorDAO::saveMetadata(int64_t indexId, const std::string& sourceTable, int64_t sourceId, const std::string& text) {
    const char* sql =
        "INSERT INTO VectorMetadata (index_id, source_table, source_id, text) VALUES (?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int64(stmt, 1, indexId);
    sqlite3_bind_text(stmt, 2, sourceTable.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, sourceId);
    sqlite3_bind_text(stmt, 4, text.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Error inserting metadata: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}

bool VectorDAO::getMetadata(int64_t indexId, std::string& metadata) {
    const char* sql =
        "SELECT text FROM VectorMetadata WHERE index_id = ?";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int64(stmt, 1, indexId);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* txt = sqlite3_column_text(stmt, 0);
        metadata = txt ? reinterpret_cast<const char*>(txt) : std::string();
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_ROW;
}
