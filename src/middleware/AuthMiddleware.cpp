#include "AuthMiddleware.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>

AuthMiddleware& AuthMiddleware::instance() {
    static AuthMiddleware inst;
    return inst;
}

std::string AuthMiddleware::generateToken() {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(now);
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << dist(rng)
        << std::setw(16) << std::setfill('0') << dist(rng);
    return oss.str();
}

std::string AuthMiddleware::createToken(int userId, const std::string&) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::string token = generateToken();
    tokens_[token] = userId;
    return token;
}

bool AuthMiddleware::validateToken(const std::string& token, int& userId) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = tokens_.find(token);
    if (it == tokens_.end()) return false;
    userId = it->second;
    return true;
}

void AuthMiddleware::removeToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(mtx_);
    tokens_.erase(token);
}
