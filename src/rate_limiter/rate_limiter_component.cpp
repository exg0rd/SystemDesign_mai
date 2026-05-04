#include "rate_limiter_component.hpp"

namespace rate_limit {

RateLimiterComponent::RateLimiterComponent(const userver::components::ComponentConfig& config,
                                           const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {}

RateLimiterComponent::~RateLimiterComponent() = default;

}