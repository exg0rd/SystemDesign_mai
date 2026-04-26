#include "event_handler.hpp"
#include "auth_handler.hpp"
#include <userver/components/component.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/json.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/utils/datetime.hpp>

namespace handlers {

static bool CheckAuth(const userver::server::http::HttpRequest& request, std::string& user_id) {
    std::string auth = request.GetHeader("Authorization");
    if (auth.substr(0, 7) != "Bearer ") return false;
    return AuthMiddleware::Instance().ValidateToken(auth.substr(7), user_id);
}

CreateEvent::CreateEvent(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

std::string CreateEvent::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                           userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    auto body = userver::formats::json::FromString(request.RequestBody());
    std::string title = body["title"].As<std::string>();
    std::string description = body["description"].As<std::string>("");
    std::string date_str = body["date"].As<std::string>();
    std::string location = body["location"].As<std::string>("");

    auto date = userver::utils::datetime::Stringtime(date_str, "UTC", "%Y-%m-%d");
    auto new_id = userver::formats::bson::Oid();

    auto coll = mongo_pool_->GetCollection("events");
    auto doc = userver::formats::bson::MakeDoc(
        "_id", new_id,
        "title", title,
        "description", description,
        "date", date,
        "location", location,
        "organizer_id", userver::formats::bson::Oid(user_id),
        "participants", userver::formats::bson::MakeArray(),
        "created_at", std::chrono::system_clock::now()
    );

    coll.InsertOne(doc);
    request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
    return userver::formats::json::ToString(
        userver::formats::json::MakeObject("id", new_id.ToString(), "title", title));
}

GetEvents::GetEvents(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

std::string GetEvents::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                         userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    auto coll = mongo_pool_->GetCollection("events");
    auto cursor = coll.Find(userver::formats::bson::MakeDoc());

    userver::formats::json::ValueBuilder builder(userver::formats::json::Type::kArray);
    for (auto doc : cursor) {
        builder.PushBack(userver::formats::json::MakeObject(
            "id", doc["_id"].As<userver::formats::bson::Oid>().ToString(),
            "title", doc["title"].As<std::string>(),
            "description", doc["description"].As<std::string>(""),
            "date", userver::utils::datetime::Timestring(doc["date"].As<std::chrono::system_clock::time_point>(), "UTC", "%Y-%m-%d"),
            "location", doc["location"].As<std::string>(""),
            "organizer_id", doc["organizer_id"].As<userver::formats::bson::Oid>().ToString()
        ));
    }

    return userver::formats::json::ToString(builder.ExtractValue());
}

SearchEventsByDate::SearchEventsByDate(const userver::components::ComponentConfig& config,
                                       const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

std::string SearchEventsByDate::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                                  userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string date_from = request.GetArg("date_from");
    std::string date_to = request.GetArg("date_to");

    auto from = userver::utils::datetime::Stringtime(date_from, "UTC", "%Y-%m-%d");
    auto to = userver::utils::datetime::Stringtime(date_to, "UTC", "%Y-%m-%d");

    auto coll = mongo_pool_->GetCollection("events");
    auto filter = userver::formats::bson::MakeDoc(
        "date", userver::formats::bson::MakeDoc(
            "$gte", from,
            "$lte", to
        )
    );

    auto cursor = coll.Find(filter);
    userver::formats::json::ValueBuilder builder(userver::formats::json::Type::kArray);

    for (auto doc : cursor) {
        builder.PushBack(userver::formats::json::MakeObject(
            "id", doc["_id"].As<userver::formats::bson::Oid>().ToString(),
            "title", doc["title"].As<std::string>(),
            "date", userver::utils::datetime::Timestring(doc["date"].As<std::chrono::system_clock::time_point>(), "UTC", "%Y-%m-%d")
        ));
    }

    return userver::formats::json::ToString(builder.ExtractValue());
}

RegisterParticipant::RegisterParticipant(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

std::string RegisterParticipant::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                                   userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string event_id = request.GetPathArg("event_id");

    auto coll = mongo_pool_->GetCollection("events");
    auto filter = userver::formats::bson::MakeDoc("_id", userver::formats::bson::Oid(event_id));
    auto update = userver::formats::bson::MakeDoc(
        "$push", userver::formats::bson::MakeDoc(
            "participants", userver::formats::bson::MakeDoc(
                "user_id", userver::formats::bson::Oid(user_id),
                "registered_at", std::chrono::system_clock::now()
            )
        )
    );

    coll.UpdateOne(filter, update);
    return userver::formats::json::ToString(
        userver::formats::json::MakeObject("status", "registered"));
}

GetEventParticipants::GetEventParticipants(const userver::components::ComponentConfig& config,
                                           const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

std::string GetEventParticipants::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                                    userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string event_id = request.GetPathArg("event_id");

    auto coll = mongo_pool_->GetCollection("events");
    auto filter = userver::formats::bson::MakeDoc("_id", userver::formats::bson::Oid(event_id));
    auto result = coll.FindOne(filter);

    if (!result) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "event not found"));
    }

    auto participants = (*result)["participants"];
    userver::formats::json::ValueBuilder builder(userver::formats::json::Type::kArray);

    for (auto p : participants) {
        builder.PushBack(userver::formats::json::MakeObject(
            "user_id", p["user_id"].As<userver::formats::bson::Oid>().ToString(),
            "registered_at", userver::utils::datetime::Timestring(
                p["registered_at"].As<std::chrono::system_clock::time_point>(),
                "UTC", "%Y-%m-%d %H:%M:%S")
        ));
    }

    return userver::formats::json::ToString(builder.ExtractValue());
}

GetUserEvents::GetUserEvents(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

std::string GetUserEvents::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                             userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    auto coll = mongo_pool_->GetCollection("events");
    auto filter = userver::formats::bson::MakeDoc(
        "participants.user_id", userver::formats::bson::Oid(user_id)
    );

    auto cursor = coll.Find(filter);
    userver::formats::json::ValueBuilder builder(userver::formats::json::Type::kArray);

    for (auto doc : cursor) {
        builder.PushBack(userver::formats::json::MakeObject(
            "id", doc["_id"].As<userver::formats::bson::Oid>().ToString(),
            "title", doc["title"].As<std::string>(),
            "date", userver::utils::datetime::Timestring(doc["date"].As<std::chrono::system_clock::time_point>(), "UTC", "%Y-%m-%d")
        ));
    }

    return userver::formats::json::ToString(builder.ExtractValue());
}

UnregisterParticipant::UnregisterParticipant(const userver::components::ComponentConfig& config,
                                             const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

std::string UnregisterParticipant::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                                     userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string event_id = request.GetPathArg("event_id");

    auto coll = mongo_pool_->GetCollection("events");
    auto filter = userver::formats::bson::MakeDoc("_id", userver::formats::bson::Oid(event_id));
    auto update = userver::formats::bson::MakeDoc(
        "$pull", userver::formats::bson::MakeDoc(
            "participants", userver::formats::bson::MakeDoc(
                "user_id", userver::formats::bson::Oid(user_id)
            )
        )
    );

    coll.UpdateOne(filter, update);
    return userver::formats::json::ToString(
        userver::formats::json::MakeObject("status", "unregistered"));
}

}
