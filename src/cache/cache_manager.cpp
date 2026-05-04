#include "cache_manager.hpp"

namespace cache {

std::string CacheManager::Get(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        misses_++;
        return "";
    }
    auto now = std::chrono::steady_clock::now();
    auto expiry_it = expiry_.find(key);
    if (expiry_it != expiry_.end() && now >= expiry_it->second) {
        cache_.erase(it);
        expiry_.erase(expiry_it);
        misses_++;
        return "";
    }
    hits_++;
    return it->second;
}

void CacheManager::Set(const std::string& key, const std::string& value, std::chrono::seconds ttl) {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_[key] = value;
    expiry_[key] = std::chrono::steady_clock::now() + ttl;
}

void CacheManager::Delete(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.erase(key);
    expiry_.erase(key);
}

void CacheManager::DeleteByPattern(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = cache_.begin(); it != cache_.end();) {
        if (it->first.find(pattern) != std::string::npos) {
            it = cache_.erase(it);
            expiry_.erase(it->first);
        } else {
            ++it;
        }
    }
}

}