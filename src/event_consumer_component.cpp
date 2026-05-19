#include "event_consumer_component.hpp"
#include <userver/components/component.hpp>
#include <userver/formats/bson.hpp>
#include <userver/formats/json.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/urabbitmq/component.hpp>
#include <chrono>

namespace event_consumer {

EventConsumerComponent::EventConsumerComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ConsumerComponentBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

EventConsumerComponent::~EventConsumerComponent() = default;

void EventConsumerComponent::Process(std::string message) {
    auto json = userver::formats::json::FromString(message);
    std::string event_id = json["event_id"].As<std::string>();
    std::string event_type = json["event_type"].As<std::string>();
    auto payload = json["payload"];

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (IsDuplicate(event_id)) return;
        MarkAsProcessed(event_id);
    }

    if (event_type == "user.created") {
        HandleUserCreated(payload);
    } else if (event_type == "user.updated") {
        HandleUserUpdated(payload);
    } else if (event_type == "user.deleted") {
        HandleUserDeleted(payload);
    } else if (event_type == "event.created") {
        HandleEventCreated(payload);
    } else if (event_type == "event.updated") {
        HandleEventUpdated(payload);
    } else if (event_type == "event.deleted") {
        HandleEventDeleted(payload);
    } else if (event_type == "participant.registered") {
        HandleParticipantRegistered(payload);
    } else if (event_type == "participant.unregistered") {
        HandleParticipantUnregistered(payload);
    }
}

bool EventConsumerComponent::IsDuplicate(const std::string& event_id) {
    return processed_events_.find(event_id) != processed_events_.end();
}

void EventConsumerComponent::MarkAsProcessed(const std::string& event_id) {
    processed_events_.insert(event_id);
}

void EventConsumerComponent::HandleUserCreated(const userver::formats::json::Value& payload) {
    auto login = payload["login"].As<std::string>();
    auto first_name = payload["first_name"].As<std::string>();
    auto last_name = payload["last_name"].As<std::string>();
    auto email = payload["email"].As<std::string>();

    auto coll = mongo_pool_->GetCollection("users");
    auto filter = userver::formats::bson::MakeDoc("login", login);
    if (!coll.FindOne(filter)) {
        coll.InsertOne(userver::formats::bson::MakeDoc(
            "login", login,
            "first_name", first_name,
            "last_name", last_name,
            "email", email,
            "created_at", std::chrono::system_clock::now()));
    }
}

void EventConsumerComponent::HandleUserUpdated(const userver::formats::json::Value& payload) {
    auto login = payload["login"].As<std::string>();
    auto first_name = payload["first_name"].As<std::string>();
    auto last_name = payload["last_name"].As<std::string>();
    auto email = payload["email"].As<std::string>();

    auto coll = mongo_pool_->GetCollection("users");
    coll.UpdateOne(
        userver::formats::bson::MakeDoc("login", login),
        userver::formats::bson::MakeDoc(
            "$set", userver::formats::bson::MakeDoc(
                "first_name", first_name,
                "last_name", last_name,
                "email", email,
                "updated_at", std::chrono::system_clock::now())));
}

void EventConsumerComponent::HandleUserDeleted(const userver::formats::json::Value& payload) {
    auto login = payload["login"].As<std::string>();
    auto coll = mongo_pool_->GetCollection("users");
    coll.DeleteOne(userver::formats::bson::MakeDoc("login", login));
}

void EventConsumerComponent::HandleEventCreated(const userver::formats::json::Value& payload) {
    auto event_id = payload["event_id"].As<std::string>();
    auto title = payload["title"].As<std::string>();
    auto description = payload["description"].As<std::string>("");
    auto date = payload["date"].As<std::string>();
    auto location = payload["location"].As<std::string>("");
    auto organizer_id = payload["organizer_id"].As<std::string>();

    auto coll = mongo_pool_->GetCollection("events");
    auto filter = userver::formats::bson::MakeDoc("event_id", event_id);
    if (!coll.FindOne(filter)) {
        coll.InsertOne(userver::formats::bson::MakeDoc(
            "event_id", event_id,
            "title", title,
            "description", description,
            "date", date,
            "location", location,
            "organizer_id", organizer_id,
            "created_at", std::chrono::system_clock::now()));
    }
}

void EventConsumerComponent::HandleEventUpdated(const userver::formats::json::Value& payload) {
    auto event_id = payload["event_id"].As<std::string>();
    auto title = payload["title"].As<std::string>();
    auto description = payload["description"].As<std::string>("");
    auto date = payload["date"].As<std::string>();
    auto location = payload["location"].As<std::string>("");

    auto coll = mongo_pool_->GetCollection("events");
    coll.UpdateOne(
        userver::formats::bson::MakeDoc("event_id", event_id),
        userver::formats::bson::MakeDoc(
            "$set", userver::formats::bson::MakeDoc(
                "title", title,
                "description", description,
                "date", date,
                "location", location,
                "updated_at", std::chrono::system_clock::now())));
}

void EventConsumerComponent::HandleEventDeleted(const userver::formats::json::Value& payload) {
    auto event_id = payload["event_id"].As<std::string>();
    auto coll = mongo_pool_->GetCollection("events");
    coll.DeleteOne(userver::formats::bson::MakeDoc("event_id", event_id));
}

void EventConsumerComponent::HandleParticipantRegistered(const userver::formats::json::Value& payload) {
    auto event_id = payload["event_id"].As<std::string>();
    auto user_id = payload["user_id"].As<std::string>();
    auto login = payload["login"].As<std::string>();
    auto first_name = payload["first_name"].As<std::string>();
    auto last_name = payload["last_name"].As<std::string>();
    auto email = payload["email"].As<std::string>();

    auto coll = mongo_pool_->GetCollection("events");
    coll.UpdateOne(
        userver::formats::bson::MakeDoc("event_id", event_id),
        userver::formats::bson::MakeDoc(
            "$push", userver::formats::bson::MakeDoc(
                "participants", userver::formats::bson::MakeDoc(
                    "user_id", user_id,
                    "login", login,
                    "first_name", first_name,
                    "last_name", last_name,
                    "email", email,
                    "registered_at", std::chrono::system_clock::now()))));
}

void EventConsumerComponent::HandleParticipantUnregistered(const userver::formats::json::Value& payload) {
    auto event_id = payload["event_id"].As<std::string>();
    auto user_id = payload["user_id"].As<std::string>();

    auto coll = mongo_pool_->GetCollection("events");
    coll.UpdateOne(
        userver::formats::bson::MakeDoc("event_id", event_id),
        userver::formats::bson::MakeDoc(
            "$pull", userver::formats::bson::MakeDoc(
                "participants", userver::formats::bson::MakeDoc(
                    "user_id", user_id))));
}

}  // namespace event_consumer
