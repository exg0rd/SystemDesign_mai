#pragma once
#include <userver/components/component_base.hpp>
#include "rate_limiter.hpp"

namespace rate_limit {

class RateLimiterComponent : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "rate-limiter";
    RateLimiterComponent(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);
    ~RateLimiterComponent();

    RateLimiter& GetRateLimiter() { return rate_limiter_; }

private:
    RateLimiter rate_limiter_;
};

}