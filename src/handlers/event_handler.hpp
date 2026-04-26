#pragma once
#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/mongo/pool.hpp>

namespace handlers {

class CreateEvent : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-create-event";
    CreateEvent(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
};

class GetEvents : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-events";
    GetEvents(const userver::components::ComponentConfig& config,
              const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
};

class SearchEventsByDate : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-search-events-date";
    SearchEventsByDate(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
};

class RegisterParticipant : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-register-participant";
    RegisterParticipant(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
};

class GetEventParticipants : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-event-participants";
    GetEventParticipants(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
};

class GetUserEvents : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-user-events";
    GetUserEvents(const userver::components::ComponentConfig& config,
                  const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
};

class UnregisterParticipant : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-unregister-participant";
    UnregisterParticipant(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
};

}
