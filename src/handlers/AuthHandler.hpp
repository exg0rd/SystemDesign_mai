#pragma once

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace event_manager {

class LoginHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-login";
    LoginHandler(const userver::components::ComponentConfig& cfg,
                 const userver::components::ComponentContext& ctx);
    std::string HandleRequest(userver::server::http::HttpRequest& req,
                              userver::server::request::RequestContext&) const override;
private:
    userver::storages::postgres::ClusterPtr pg_;
};

class LogoutHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-logout";
    LogoutHandler(const userver::components::ComponentConfig& cfg,
                  const userver::components::ComponentContext& ctx);
    std::string HandleRequest(userver::server::http::HttpRequest& req,
                              userver::server::request::RequestContext&) const override;
private:
    userver::storages::postgres::ClusterPtr pg_;
};

} 
