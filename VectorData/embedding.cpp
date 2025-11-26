#include "embedding.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <json.hpp>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <chrono>
#include <thread>

using json = nlohmann::json;
namespace fs = std::filesystem;

static std::string make_temp_filename() {
    auto t = std::chrono::system_clock::now().time_since_epoch().count();
    std::ostringstream ss;
    ss << "tmp_input_" << t << ".txt";
    return ss.str();
}

static inline std::string trim_copy(const std::string& s) {
    size_t a = 0;
    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    size_t b = s.size();
    while (b > a && std::isspace(static_cast<unsigned char>(s[b-1]))) --b;
    return s.substr(a, b-a);
}

std::vector<float> generateEmbedding(const std::string& text) {
    std::string inputPath;
    bool createdTemp = false;
    if (!text.empty() && fs::exists(text)) {
        inputPath = text;
    } else {
        inputPath = make_temp_filename();
        std::ofstream out(inputPath, std::ios::out | std::ios::binary);
        if (!out.is_open()) {
            throw std::runtime_error("Failed to create temporary input file for embedding: " + inputPath);
        }
        out << text;
        out.close();
        createdTemp = true;
    }

    std::string command = "cmd /C \"chcp 65001 > NUL && python F:/ProjectTrainer/Models/embedding.py \\\"";
    command += inputPath;
    command += "\\\"\"";

    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        if (createdTemp) fs::remove(inputPath);
        throw std::runtime_error("Failed to start Python script for embedding.");
    }

    char buffer[4096];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    _pclose(pipe);

    if (createdTemp) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::error_code ec;
        fs::remove(inputPath, ec);
    }

    std::string raw = trim_copy(result);

    std::vector<float> emb;
    try {
        json j = json::parse(raw);
        if (!j.is_array()) throw std::runtime_error("Embedding output is not array");
        for (const auto& e : j) {
            if (!e.is_number()) throw std::runtime_error("Non-numeric element in embedding");
            emb.push_back(e.get<float>());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing embedding output: " << e.what() << "\nRaw output: '" << raw << "'" << std::endl;
        throw;
    }

    std::cerr << "DEBUG: Parsed embedding length = " << emb.size() << std::endl;

    static int expected_dim = -1;
    if (expected_dim == -1) {
        expected_dim = (int)emb.size();
        std::cerr << "DEBUG: Setting expected embedding dimension = " << expected_dim << std::endl;
    } else if ((int)emb.size() != expected_dim) {
        throw std::runtime_error("Embedding dimension mismatch: got " +
            std::to_string(emb.size()) + ", expected " + std::to_string(expected_dim));
    }

    return emb;
}