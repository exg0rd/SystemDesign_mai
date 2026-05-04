#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <deque>
#include <chrono>

namespace rate_limit {

struct RateLimitConfig {
    size_t limit;
    std::chrono::seconds window;
};

class RateLimiter {
public:
    RateLimiter() = default;
    
    struct Result {
        bool allowed;
        size_t remaining;
        std::chrono::steady_clock::time_point reset_time;
    };
    
    Result Allow(const std::string& client_id, const RateLimitConfig& config);
    Result GetInfo(const std::string& client_id, const RateLimitConfig& config);

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::deque<std::chrono::steady_clock::time_point>> requests_;
};

}