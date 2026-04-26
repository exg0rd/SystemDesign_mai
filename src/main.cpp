#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/auth_handler.hpp"
#include "handlers/user_handler.hpp"
#include "handlers/event_handler.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
        .Append<userver::server::handlers::Ping>()
        .Append<userver::components::DefaultSecdistProvider>()
        .Append<userver::components::Secdist>()
        .Append<userver::components::Mongo>("mongo")
        .Append<userver::clients::dns::Component>()
        .Append<handlers::Login>()
        .Append<handlers::Logout>()
        .Append<handlers::CreateUser>()
        .Append<handlers::GetUserByLogin>()
        .Append<handlers::SearchUsers>()
        .Append<handlers::CreateEvent>()
        .Append<handlers::GetEvents>()
        .Append<handlers::SearchEventsByDate>()
        .Append<handlers::RegisterParticipant>()
        .Append<handlers::GetEventParticipants>()
        .Append<handlers::GetUserEvents>()
        .Append<handlers::UnregisterParticipant>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
