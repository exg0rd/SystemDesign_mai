#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/AuthHandler.hpp"
#include "handlers/UserHandler.hpp"
#include "handlers/EventHandler.hpp"
#include "handlers/ParticipantHandler.hpp"

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::components::Postgres>("event-db")
            .Append<userver::clients::dns::Component>()
            .Append<userver::components::TestsuiteSupport>()
            .Append<event_manager::LoginHandler>()
            .Append<event_manager::LogoutHandler>()
            .Append<event_manager::CreateUserHandler>()
            .Append<event_manager::GetUserByLoginHandler>()
            .Append<event_manager::SearchUsersHandler>()
            .Append<event_manager::CreateEventHandler>()
            .Append<event_manager::GetEventsHandler>()
            .Append<event_manager::SearchEventsByDateHandler>()
            .Append<event_manager::RegisterParticipantHandler>()
            .Append<event_manager::GetParticipantsHandler>()
            .Append<event_manager::GetUserEventsHandler>()
            .Append<event_manager::CancelRegistrationHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
