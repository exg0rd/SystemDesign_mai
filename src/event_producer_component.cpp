#include "event_producer_component.hpp"
#include <userver/components/component.hpp>
#include <userver/formats/json.hpp>
#include <userver/urabbitmq/component.hpp>
#include <userver/urabbitmq/typedefs.hpp>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace event_producer {

EventProducerComponent::EventProducerComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context),
      client_(context.FindComponent<userver::components::RabbitMQ>("my-rabbit").GetClient()) {
    DeclareRabbitMQInfrastructure();
}

EventProducerComponent::~EventProducerComponent() = default;

void EventProducerComponent::DeclareRabbitMQInfrastructure() {
    constexpr auto kDeadline = std::chrono::seconds(5);
    auto deadline = userver::engine::Deadline::FromDuration(kDeadline);

    auto durable = userver::utils::Flags<userver::urabbitmq::Exchange::Flags>(
        userver::urabbitmq::Exchange::Flags::kDurable);

    auto durable_queue = userver::utils::Flags<userver::urabbitmq::Queue::Flags>(
        userver::urabbitmq::Queue::Flags::kDurable);

    client_->DeclareExchange(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Exchange::Type::kDirect, durable, deadline);

    client_->DeclareExchange(
        userver::urabbitmq::Exchange("events.fanout"),
        userver::urabbitmq::Exchange::Type::kFanOut, durable, deadline);

    client_->DeclareExchange(
        userver::urabbitmq::Exchange("events.topic"),
        userver::urabbitmq::Exchange::Type::kTopic, durable, deadline);

    client_->DeclareQueue(
        userver::urabbitmq::Queue("notifications"), durable_queue, deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("notifications"),
        "user.created", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("notifications"),
        "user.updated", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("notifications"),
        "user.deleted", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("notifications"),
        "event.created", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("notifications"),
        "event.updated", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("notifications"),
        "event.deleted", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("notifications"),
        "participant.registered", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("notifications"),
        "participant.unregistered", deadline);

    client_->DeclareQueue(
        userver::urabbitmq::Queue("analytics"), durable_queue, deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.fanout"),
        userver::urabbitmq::Queue("analytics"),
        "", deadline);

    client_->DeclareQueue(
        userver::urabbitmq::Queue("cache-invalidation"), durable_queue, deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("cache-invalidation"),
        "event.created", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("cache-invalidation"),
        "event.updated", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("cache-invalidation"),
        "event.deleted", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("cache-invalidation"),
        "user.created", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("cache-invalidation"),
        "user.deleted", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("cache-invalidation"),
        "participant.registered", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("cache-invalidation"),
        "participant.unregistered", deadline);

    client_->DeclareQueue(
        userver::urabbitmq::Queue("search-index"), durable_queue, deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("search-index"),
        "user.created", deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.direct"),
        userver::urabbitmq::Queue("search-index"),
        "event.created", deadline);

    client_->DeclareQueue(
        userver::urabbitmq::Queue("reporting"), durable_queue, deadline);
    client_->BindQueue(
        userver::urabbitmq::Exchange("events.fanout"),
        userver::urabbitmq::Queue("reporting"),
        "", deadline);

    client_->DeclareQueue(
        userver::urabbitmq::Queue("notifications-dlq"), durable_queue, deadline);
}

std::string EventProducerComponent::GenerateId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << dis(gen)
        << std::setw(16) << std::setfill('0') << dis(gen);
    return oss.str();
}

void EventProducerComponent::PublishEvent(
    const std::string& event_type, const std::string& routing_key,
    const userver::formats::json::Value& payload,
    const std::string& source_service) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()).count();

    auto event = userver::formats::json::MakeObject(
        "event_id", GenerateId(),
        "event_type", event_type,
        "timestamp", std::to_string(timestamp),
        "payload", payload,
        "metadata", userver::formats::json::MakeObject(
            "source_service", source_service,
            "correlation_id", GenerateId(),
            "trace_id", GenerateId(),
            "version", "1.0"));

    std::string body = userver::formats::json::ToString(event);
    constexpr auto kDeadline = std::chrono::seconds(5);

    client_->PublishReliable(
        userver::urabbitmq::Exchange("events.direct"),
        routing_key, body,
        userver::urabbitmq::MessageType::kPersistent,
        userver::engine::Deadline::FromDuration(kDeadline));
}

void EventProducerComponent::PublishUserCreated(
    const std::string& user_id, const std::string& login,
    const std::string& first_name, const std::string& last_name,
    const std::string& email) {
    auto payload = userver::formats::json::MakeObject(
        "user_id", user_id,
        "login", login,
        "first_name", first_name,
        "last_name", last_name,
        "email", email);
    PublishEvent("user.created", "user.created", payload, "user-service");
}

void EventProducerComponent::PublishUserUpdated(
    const std::string& user_id, const std::string& login,
    const std::string& first_name, const std::string& last_name,
    const std::string& email) {
    auto payload = userver::formats::json::MakeObject(
        "user_id", user_id,
        "login", login,
        "first_name", first_name,
        "last_name", last_name,
        "email", email);
    PublishEvent("user.updated", "user.updated", payload, "user-service");
}

void EventProducerComponent::PublishUserDeleted(
    const std::string& user_id, const std::string& login) {
    auto payload = userver::formats::json::MakeObject(
        "user_id", user_id,
        "login", login);
    PublishEvent("user.deleted", "user.deleted", payload, "user-service");
}

void EventProducerComponent::PublishEventCreated(
    const std::string& event_id, const std::string& title,
    const std::string& description, const std::string& date,
    const std::string& location, const std::string& organizer_id) {
    auto payload = userver::formats::json::MakeObject(
        "event_id", event_id,
        "title", title,
        "description", description,
        "date", date,
        "location", location,
        "organizer_id", organizer_id);
    PublishEvent("event.created", "event.created", payload, "event-service");
}

void EventProducerComponent::PublishEventUpdated(
    const std::string& event_id, const std::string& title,
    const std::string& description, const std::string& date,
    const std::string& location) {
    auto payload = userver::formats::json::MakeObject(
        "event_id", event_id,
        "title", title,
        "description", description,
        "date", date,
        "location", location);
    PublishEvent("event.updated", "event.updated", payload, "event-service");
}

void EventProducerComponent::PublishEventDeleted(
    const std::string& event_id, const std::string& title) {
    auto payload = userver::formats::json::MakeObject(
        "event_id", event_id,
        "title", title);
    PublishEvent("event.deleted", "event.deleted", payload, "event-service");
}

void EventProducerComponent::PublishParticipantRegistered(
    const std::string& event_id, const std::string& user_id,
    const std::string& login, const std::string& first_name,
    const std::string& last_name, const std::string& email) {
    auto payload = userver::formats::json::MakeObject(
        "event_id", event_id,
        "user_id", user_id,
        "login", login,
        "first_name", first_name,
        "last_name", last_name,
        "email", email);
    PublishEvent("participant.registered", "participant.registered", payload, "registration-service");
}

void EventProducerComponent::PublishParticipantUnregistered(
    const std::string& event_id, const std::string& user_id,
    const std::string& login) {
    auto payload = userver::formats::json::MakeObject(
        "event_id", event_id,
        "user_id", user_id,
        "login", login);
    PublishEvent("participant.unregistered", "participant.unregistered", payload, "registration-service");
}

}  // namespace event_producer
