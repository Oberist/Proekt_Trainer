#include "ExerciseManager.h"
#include <iostream>

int main() {
    ExerciseManager manager("test.db");

    std::cout << "All exercises:" << std::endl;
    for (const auto& ex : manager.getAllExercises()) {
        std::cout << ex.id << ": " << ex.name << " (" << ex.muscle_group_primary << ")" << std::endl;
    }

    std::cout << "\nSearch by name 'Squat':" << std::endl;
    auto searchResult = manager.searchByName("Squat");
    for (const auto& ex : searchResult) {
        std::cout << ex.id << ": " << ex.name << std::endl;
    }

    std::cout << "\nFilter by difficulty 2:" << std::endl;
    auto filtered = manager.filterByDifficulty(2);
    for (const auto& ex : filtered) {
        std::cout << ex.id << ": " << ex.name << std::endl;
    }

    return 0;
}