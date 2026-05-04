#pragma once
#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/mongo/pool.hpp>
#include "../cache/cache_component.hpp"
#include "../rate_limiter/rate_limiter_component.hpp"

namespace handlers {

class CreateUser : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-create-user";
    CreateUser(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
    cache::CacheManager* cache_manager_;
};

class GetUserByLogin : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-user-login";
    GetUserByLogin(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
    cache::CacheManager* cache_manager_;
    rate_limit::RateLimiter* rate_limiter_;
};

class SearchUsers : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-search-users";
    SearchUsers(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
    cache::CacheManager* cache_manager_;
    rate_limit::RateLimiter* rate_limiter_;
};

}