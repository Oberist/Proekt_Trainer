#ifndef VECTOR_DAO_H
#define VECTOR_DAO_H

#include <string>
#include <vector>
#include "hnswlib.h"
#include "sqlite3.h"

class VectorDAO {
private:
    hnswlib::L2Space* space;
    hnswlib::HierarchicalNSW<float>* index;
    sqlite3* db;  
    std::string indexPath;
    int dim;

public:
    VectorDAO(const std::string& dbPath, const std::string& indexPath, int dim);
    ~VectorDAO();

    bool addVector(int64_t chunkId, const std::vector<float>& embedding);
    std::vector<std::pair<int64_t, float>> search(const std::vector<float>& queryEmbedding, int topK);
    bool saveIndex();
    bool loadIndex();
    bool saveMetadata(int64_t indexId, const std::string& sourceTable, int64_t sourceId, const std::string& text);
    bool getMetadata(int64_t indexId, std::string& metadata);
};

#endif