#include "rate_limiter.hpp"

namespace rate_limit {

RateLimiter::Result RateLimiter::Allow(const std::string& client_id, const RateLimitConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto& timestamps = requests_[client_id];
    auto cutoff = now - config.window;
    while (!timestamps.empty() && timestamps.front() < cutoff) {
        timestamps.pop_front();
    }
    size_t current_count = timestamps.size();
    if (current_count >= config.limit) {
        return {false, 0, timestamps.front() + config.window};
    }
    timestamps.push_back(now);
    return {true, config.limit - timestamps.size(), timestamps.back() + config.window};
}

RateLimiter::Result RateLimiter::GetInfo(const std::string& client_id, const RateLimitConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto& timestamps = requests_[client_id];
    auto cutoff = now - config.window;
    while (!timestamps.empty() && timestamps.front() < cutoff) {
        timestamps.pop_front();
    }
    size_t remaining = config.limit - timestamps.size();
    std::chrono::steady_clock::time_point reset_time;
    if (!timestamps.empty()) {
        reset_time = timestamps.front() + config.window;
    } else {
        reset_time = now + config.window;
    }
    return {timestamps.size() < config.limit, remaining, reset_time};
}

}