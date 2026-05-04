#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>

namespace cache {

class CacheManager {
public:
    CacheManager() = default;
    
    std::string Get(const std::string& key) const;
    void Set(const std::string& key, const std::string& value, std::chrono::seconds ttl);
    void Delete(const std::string& key);
    void DeleteByPattern(const std::string& pattern);
    size_t GetHits() const { return hits_; }
    size_t GetMisses() const { return misses_; }
    void ResetStats() { hits_ = 0; misses_ = 0; }

private:
    mutable std::unordered_map<std::string, std::string> cache_;
    mutable std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiry_;
    mutable std::mutex mutex_;
    mutable size_t hits_ = 0;
    mutable size_t misses_ = 0;
};

}