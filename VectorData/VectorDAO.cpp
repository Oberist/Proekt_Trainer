#include "VectorDAO.h"
#include <stdexcept>
#include <iostream>
#include <sqlite3.h>

VectorDAO::VectorDAO(const std::string& dbPath, const std::string& indexPath, int dim)
    : indexPath(indexPath), dim(dim)
{
    space = new hnswlib::L2Space(dim);
    index = new hnswlib::HierarchicalNSW<float>(space, 10000, 16, 200);

    if (sqlite3_open(dbPath.c_str(), &db)) {
        std::cerr << "Ошибка открытия базы данных: " << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error("Failed to open database");
    }
}

VectorDAO::~VectorDAO() {
    if (index) {
        delete index;
    }
    if (space) {
        delete space;
    }
    sqlite3_close(db);
}

bool VectorDAO::addVector(int64_t chunkId, const std::vector<float>& embedding) {
    if (embedding.size() != dim) {
        throw std::invalid_argument("Embedding dimension mismatch");
    }

    index->addPoint(embedding.data(), chunkId);

    return saveMetadata(chunkId, "DocumentChunks", chunkId, "Chunk text or metadata");
}

std::vector<std::pair<int64_t, float>> VectorDAO::search(const std::vector<float>& queryEmbedding, int topK) {
    std::vector<std::pair<int64_t, float>> results;

    auto res = index->searchKnn(queryEmbedding.data(), topK);
    while (!res.empty()) {
        auto p = res.top();
        res.pop();
        results.push_back({p.second, p.first});
    }

    return results;
}

bool VectorDAO::saveIndex() {
    index->saveIndex(indexPath);
    return true;
}

bool VectorDAO::loadIndex() {
    index->loadIndex(indexPath);
    return true;
}

bool VectorDAO::saveMetadata(int64_t indexId, const std::string& sourceTable, int64_t sourceId, const std::string& text) {
    std::string sql = "INSERT INTO VectorMetadata (index_id, source_table, source_id, text) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, indexId);
    sqlite3_bind_text(stmt, 2, sourceTable.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, sourceId);
    sqlite3_bind_text(stmt, 4, text.c_str(), -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool VectorDAO::getMetadata(int64_t indexId, std::string& metadata) {
    std::string sql = "SELECT metadata FROM VectorMetadata WHERE index_id = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, indexId);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        metadata = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_ROW;
}
