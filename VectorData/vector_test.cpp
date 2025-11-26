#include "VectorDAO.h"
#include "embedding.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main() {
    // Создаем объект VectorDAO без фиксированной размерности (-1 => автоопределение)
    VectorDAO dao(
        "vector_meta.db",      // Путь к базе данных для метаданных
        "vector_index.bin",    // Путь к файлу с индексом
        -1                     // Размерность будет определена динамически
    );

    // Читаем текст из файла benchpress.txt
    std::ifstream file("benchpress.txt", std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: benchpress.txt" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();  // Считываем весь файл
    std::string text = buffer.str();
    file.close();

    if (text.empty()) {
        std::cerr << "File is empty: benchpress.txt" << std::endl;
        return 1;
    }

    std::cout << "Text loaded from file:\n" << text << std::endl;

    // Добавляем текст в индекс
    try {
        if (!dao.addVectorText(1, text)) {
            std::cerr << "Failed to add text to index: unknown error" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to add text to index: " << e.what() << std::endl;
        return 1;
    }

    // Сохраняем индекс
    if (!dao.saveIndex()) {
        std::cerr << "Failed to save index." << std::endl;
        return 1;
    }

    // Загружаем индекс
    if (!dao.loadIndex()) {
        std::cerr << "Failed to load index." << std::endl;
        return 1;
    }

    // Генерируем эмбеддинг для запроса
    std::string query = "грудные мышцы";
    std::vector<float> q;
    try {
        q = generateEmbedding(query);
        if (q.empty()) {
            std::cerr << "Query embedding is empty!" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to generate embedding for query: " << e.what() << std::endl;
        return 1;
    }

    // Выполняем поиск по запросу
    auto results = dao.search(q, 3);

    // Выводим результаты поиска
    for (auto& r : results) {
        std::string meta;
        if (!dao.getMetadata(r.first, meta)) {
            meta = "(no metadata)";
        }

        std::cout << "ID = " << r.first
                  << " Dist = " << r.second
                  << "\nMetadata: " << meta << std::endl;
    }

    return 0;
}