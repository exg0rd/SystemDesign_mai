#include "cache_component.hpp"

namespace cache {

CacheComponent::CacheComponent(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {}

CacheComponent::~CacheComponent() = default;

}