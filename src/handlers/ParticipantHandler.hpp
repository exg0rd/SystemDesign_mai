#pragma once

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace event_manager {
// добавить примеры в описанеи апи
// POST /events/{event_id}/participants
class RegisterParticipantHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-register-participant";
    RegisterParticipantHandler(const userver::components::ComponentConfig& cfg,
                               const userver::components::ComponentContext& ctx);
    std::string HandleRequest(userver::server::http::HttpRequest& req,
                              userver::server::request::RequestContext&) const override;
private:
    userver::storages::postgres::ClusterPtr pg_;
};

// GET /events/{event_id}/participants
class GetParticipantsHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-participants";
    GetParticipantsHandler(const userver::components::ComponentConfig& cfg,
                           const userver::components::ComponentContext& ctx);
    std::string HandleRequest(userver::server::http::HttpRequest& req,
                              userver::server::request::RequestContext&) const override;
private:
    userver::storages::postgres::ClusterPtr pg_;
};

// GET /users/{user_id}/events
class GetUserEventsHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-user-events";
    GetUserEventsHandler(const userver::components::ComponentConfig& cfg,
                         const userver::components::ComponentContext& ctx);
    std::string HandleRequest(userver::server::http::HttpRequest& req,
                              userver::server::request::RequestContext&) const override;
private:
    userver::storages::postgres::ClusterPtr pg_;
};

// DELETE /events/{event_id}/participants/{user_id}
class CancelRegistrationHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-cancel-registration";
    CancelRegistrationHandler(const userver::components::ComponentConfig& cfg,
                              const userver::components::ComponentContext& ctx);
    std::string HandleRequest(userver::server::http::HttpRequest& req,
                              userver::server::request::RequestContext&) const override;
private:
    userver::storages::postgres::ClusterPtr pg_;
};

} 
