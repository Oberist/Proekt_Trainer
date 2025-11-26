#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <string>
#include <functional>
#include "UsersDAO.h" 

class UserManager {
public:
    explicit UserManager(const std::string& dbPath);

    // CRUD
    bool createUser(const User& user);
    User getUserById(int id);
    bool updateUser(const User& user);
    bool deleteUser(int id);

    bool setName(int userId, const std::string& name);
    bool setWeight(int userId, double weight);
    bool setHeight(int userId, double height);

private:
    UsersDAO dao_;

    bool modifyAndSave(int userId, const std::function<void(User&)>& modifier);
};

#endif 