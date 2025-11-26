#pragma once
#include "hnswlib.h"
#include <string>
#include <vector>
#include <sqlite3.h>

class VectorDAO {
private:
    hnswlib::HierarchicalNSW<float>* index;  
    hnswlib::SpaceInterface<float>* space;   
    sqlite3* db;                             

    std::string indexPath;  
    int dim;               

public:
    VectorDAO(const std::string& dbPath, const std::string& indexPath, int dim = -1);
    ~VectorDAO();

    bool addVectorText(int64_t id, const std::string& text);  
    std::vector<std::pair<int64_t, float>> search(const std::vector<float>& queryEmbedding, int topK);

    bool saveIndex();  
    bool loadIndex();  

    bool saveMetadata(int64_t indexId, const std::string& sourceTable, int64_t sourceId, const std::string& text);
    bool getMetadata(int64_t indexId, std::string& metadata);
};
