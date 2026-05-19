#pragma once
#include <mutex>
#include <string>
#include <unordered_set>
#include <userver/storages/mongo/pool.hpp>
#include <userver/urabbitmq/consumer_component_base.hpp>

namespace event_consumer {

class EventConsumerComponent final : public userver::urabbitmq::ConsumerComponentBase {
public:
    static constexpr std::string_view kName = "event-consumer";

    EventConsumerComponent(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context);

    ~EventConsumerComponent() override;

private:
    void Process(std::string message) override;

    void HandleUserCreated(const userver::formats::json::Value& payload);
    void HandleUserUpdated(const userver::formats::json::Value& payload);
    void HandleUserDeleted(const userver::formats::json::Value& payload);
    void HandleEventCreated(const userver::formats::json::Value& payload);
    void HandleEventUpdated(const userver::formats::json::Value& payload);
    void HandleEventDeleted(const userver::formats::json::Value& payload);
    void HandleParticipantRegistered(const userver::formats::json::Value& payload);
    void HandleParticipantUnregistered(const userver::formats::json::Value& payload);

    bool IsDuplicate(const std::string& event_id);
    void MarkAsProcessed(const std::string& event_id);

    userver::storages::mongo::PoolPtr mongo_pool_;

    mutable std::mutex mutex_;
    std::unordered_set<std::string> processed_events_;
};

}  // namespace event_consumer
