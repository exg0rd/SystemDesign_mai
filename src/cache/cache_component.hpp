#pragma once
#include <userver/components/component_base.hpp>
#include "cache_manager.hpp"

namespace cache {

class CacheComponent : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "cache-manager";
    CacheComponent(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);
    ~CacheComponent();

    CacheManager& GetCacheManager() { return cache_manager_; }

private:
    CacheManager cache_manager_;
};

}