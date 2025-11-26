#ifndef USERS_DAO_H
#define USERS_DAO_H

#include <string>
#include "sqlite3.h"

struct User {
    int id;
    std::string name;
    double weight;
    double height;
};

class UsersDAO {
private:
    sqlite3* db;
public:
    UsersDAO(const std::string& dbPath);
    ~UsersDAO();

    bool addUser(const User& user);
    User getUserById(int id);
    bool updateUser(const User& user);
    bool deleteUser(int id);
};

#endif
