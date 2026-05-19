#pragma once
#include <memory>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/urabbitmq/client.hpp>

namespace event_producer {

class EventProducerComponent final : public userver::components::LoggableComponentBase {
public:
    static constexpr std::string_view kName = "event-producer";

    EventProducerComponent(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context);

    ~EventProducerComponent() override;

    void PublishUserCreated(const std::string& user_id, const std::string& login,
                            const std::string& first_name, const std::string& last_name,
                            const std::string& email);

    void PublishUserUpdated(const std::string& user_id, const std::string& login,
                            const std::string& first_name, const std::string& last_name,
                            const std::string& email);

    void PublishUserDeleted(const std::string& user_id, const std::string& login);

    void PublishEventCreated(const std::string& event_id, const std::string& title,
                             const std::string& description, const std::string& date,
                             const std::string& location, const std::string& organizer_id);

    void PublishEventUpdated(const std::string& event_id, const std::string& title,
                             const std::string& description, const std::string& date,
                             const std::string& location);

    void PublishEventDeleted(const std::string& event_id, const std::string& title);

    void PublishParticipantRegistered(const std::string& event_id, const std::string& user_id,
                                      const std::string& login, const std::string& first_name,
                                      const std::string& last_name, const std::string& email);

    void PublishParticipantUnregistered(const std::string& event_id, const std::string& user_id,
                                        const std::string& login);

private:
    void DeclareRabbitMQInfrastructure();
    void PublishEvent(const std::string& event_type, const std::string& routing_key,
                      const userver::formats::json::Value& payload,
                      const std::string& source_service);

    std::string GenerateId();
    std::shared_ptr<userver::urabbitmq::Client> client_;
};

}  // namespace event_producer
