#include "UsersDao.h"
#include <iostream>
#include <string>

int main() {
    std::string dbPath = "F:/ProjectTrainer/Database/TrainingDiary.db"; 
    UsersDAO dao(dbPath);

    User newUser;
    std::cout << "Enter Name: ";
    std::getline(std::cin, newUser.name);
    std::cout << "Enter Weight: ";
    std::cin >> newUser.weight;
    std::cout << "Enter Height: ";
    std::cin >> newUser.height;

    if (dao.addUser(newUser)) {
        std::cout << "Success" << std::endl;
    } else {
        std::cerr << "Error" << std::endl;
    }

    return 0;
}