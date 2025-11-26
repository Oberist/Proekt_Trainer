#include "UserManager.h"
#include <iostream>

UserManager::UserManager(const std::string& dbPath)
    : dao_(dbPath) {}

bool UserManager::createUser(const User& user) {
    bool ok = dao_.addUser(user);
    if (!ok) {
        std::cerr << "UserManager: failed to add user\n";
    }
    return ok;
}

User UserManager::getUserById(int id) {
    return dao_.getUserById(id);
}

bool UserManager::updateUser(const User& user) {
    bool ok = dao_.updateUser(user);
    if (!ok) {
        std::cerr << "UserManager: failed to update user id=" << user.id << "\n";
    }
    return ok;
}

bool UserManager::deleteUser(int id) {
    bool ok = dao_.deleteUser(id);
    if (!ok) {
        std::cerr << "UserManager: failed to delete user id=" << id << "\n";
    }
    return ok;
}

bool UserManager::modifyAndSave(int userId, const std::function<void(User&)>& modifier) {
    User u = dao_.getUserById(userId);
    if (u.id <= 0) {
        std::cerr << "UserManager: user not found id=" << userId << "\n";
        return false;
    }
    modifier(u);
    return updateUser(u);
}

bool UserManager::setName(int userId, const std::string& name) {
    if (name.empty()) {
        std::cerr << "UserManager: invalid name\n";
        return false;
    }
    return modifyAndSave(userId, [&name](User& u){ u.name = name; });
}

bool UserManager::setWeight(int userId, double weight) {
    if (weight <= 0.0) {
        std::cerr << "UserManager: invalid weight\n";
        return false;
    }
    return modifyAndSave(userId, [weight](User& u){ u.weight = weight; });
}

bool UserManager::setHeight(int userId, double height) {
    if (height <= 0.0) {
        std::cerr << "UserManager: invalid height\n";
        return false;
    }
    return modifyAndSave(userId, [height](User& u){ u.height = height; });
}