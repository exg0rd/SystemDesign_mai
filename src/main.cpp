#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/urabbitmq/component.hpp>
#include <userver/utils/daemon_run.hpp>
#include <cstdlib>
#include <iostream>

#include "handlers/auth_handler.hpp"
#include "handlers/user_handler.hpp"
#include "handlers/event_handler.hpp"
#include "cache/cache_component.hpp"
#include "rate_limiter/rate_limiter_component.hpp"
#include "event_producer_component.hpp"
#include "event_consumer_component.hpp"

int main(int argc, char* argv[]) {
    bool consumer_mode = false;
    const char* service_type_env = std::getenv("SERVICE_TYPE");
    if (service_type_env && std::string(service_type_env) == "consumer") {
        consumer_mode = true;
    }

    auto component_list = userver::components::MinimalServerComponentList()
        .Append<userver::server::handlers::Ping>()
        .Append<userver::components::DefaultSecdistProvider>()
        .Append<userver::components::Secdist>()
        .Append<userver::components::Mongo>("mongo")
        .Append<userver::clients::dns::Component>()
        .Append<userver::components::RabbitMQ>("my-rabbit")
        .Append<event_consumer::EventConsumerComponent>();

    if (!consumer_mode) {
        component_list
            .Append<cache::CacheComponent>()
            .Append<rate_limit::RateLimiterComponent>()
            .Append<event_producer::EventProducerComponent>()
            .Append<handlers::Login>()
            .Append<handlers::Logout>()
            .Append<handlers::CreateUser>()
            .Append<handlers::GetUserByLogin>()
            .Append<handlers::SearchUsers>()
            .Append<handlers::UpdateUser>()
            .Append<handlers::DeleteUser>()
            .Append<handlers::CreateEvent>()
            .Append<handlers::GetEvents>()
            .Append<handlers::SearchEventsByDate>()
            .Append<handlers::RegisterParticipant>()
            .Append<handlers::GetEventParticipants>()
            .Append<handlers::GetUserEvents>()
            .Append<handlers::UnregisterParticipant>()
            .Append<handlers::UpdateEvent>()
            .Append<handlers::DeleteEvent>();
    }

    return userver::utils::DaemonMain(argc, argv, component_list);
}
