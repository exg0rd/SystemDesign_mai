#pragma once
#include <string>
#include <map>
#include <mutex>

class AuthMiddleware {
public:
    static AuthMiddleware& instance();
    std::string createToken(int userId, const std::string& login);
    bool validateToken(const std::string& token, int& userId);
    void removeToken(const std::string& token);

private:
    std::map<std::string, int> tokens_;
    std::mutex mtx_;
    std::string generateToken();
};
