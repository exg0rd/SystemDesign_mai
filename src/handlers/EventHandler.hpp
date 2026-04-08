#pragma once

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace event_manager {

class CreateEventHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-create-event";
    CreateEventHandler(const userver::components::ComponentConfig& cfg,
                       const userver::components::ComponentContext& ctx);
    std::string HandleRequest(userver::server::http::HttpRequest& req,
                              userver::server::request::RequestContext&) const override;
private:
    userver::storages::postgres::ClusterPtr pg_;
};

class GetEventsHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-events";
    GetEventsHandler(const userver::components::ComponentConfig& cfg,
                     const userver::components::ComponentContext& ctx);
    std::string HandleRequest(userver::server::http::HttpRequest& req,
                              userver::server::request::RequestContext&) const override;
private:
    userver::storages::postgres::ClusterPtr pg_;
};

class SearchEventsByDateHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-search-events-date";
    SearchEventsByDateHandler(const userver::components::ComponentConfig& cfg,
                              const userver::components::ComponentContext& ctx);
    std::string HandleRequest(userver::server::http::HttpRequest& req,
                              userver::server::request::RequestContext&) const override;
private:
    userver::storages::postgres::ClusterPtr pg_;
};

}
