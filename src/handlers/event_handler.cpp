#include "event_handler.hpp"
#include "auth_handler.hpp"
#include "../cache/cache_manager.hpp"
#include "../rate_limiter/rate_limiter.hpp"
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
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()) {}

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
    cache_manager_->Delete("events:all");
    cache_manager_->DeleteByPattern("events:user:*");
    return userver::formats::json::ToString(
        userver::formats::json::MakeObject("id", new_id.ToString(), "title", title));
}

GetEvents::GetEvents(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()),
      rate_limiter_(&context.FindComponent<rate_limit::RateLimiterComponent>("rate-limiter").GetRateLimiter()) {}

std::string GetEvents::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                         userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    auto remote_addr = request.GetRemoteAddress();
    std::string client_ip = remote_addr.PrimaryAddressString() + ":" + std::to_string(remote_addr.Port());
    rate_limit::RateLimitConfig config = {100, std::chrono::minutes(1)};
    auto result = rate_limiter_->Allow(client_ip, config);
    
    request.GetHttpResponse().SetHeader(std::string("X-RateLimit-Limit"), std::to_string(config.limit));
    request.GetHttpResponse().SetHeader(std::string("X-RateLimit-Remaining"), std::to_string(result.remaining));
    request.GetHttpResponse().SetHeader(std::string("X-RateLimit-Reset"), 
        std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            result.reset_time.time_since_epoch()).count()));
    
    if (!result.allowed) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kTooManyRequests);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "rate limit exceeded"));
    }

    std::string cache_key = "events:all";
    std::string cached = cache_manager_->Get(cache_key);
    if (!cached.empty()) {
        return cached;
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

    std::string result_json = userver::formats::json::ToString(builder.ExtractValue());
    cache_manager_->Set(cache_key, result_json, std::chrono::seconds(300));
    return result_json;
}

SearchEventsByDate::SearchEventsByDate(const userver::components::ComponentConfig& config,
                                       const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()) {}

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
    std::string cache_key = "events:search:" + date_from + ":" + date_to;
    std::string cached = cache_manager_->Get(cache_key);
    if (!cached.empty()) {
        return cached;
    }

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

    std::string result_json = userver::formats::json::ToString(builder.ExtractValue());
    cache_manager_->Set(cache_key, result_json, std::chrono::seconds(120));
    return result_json;
}

RegisterParticipant::RegisterParticipant(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()) {}

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
    cache_manager_->Delete("events:participants:" + event_id);
    cache_manager_->DeleteByPattern("events:user:*");
    return userver::formats::json::ToString(
        userver::formats::json::MakeObject("status", "registered"));
}

GetEventParticipants::GetEventParticipants(const userver::components::ComponentConfig& config,
                                           const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()) {}

std::string GetEventParticipants::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                                    userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string event_id = request.GetPathArg("event_id");
    std::string cache_key = "events:participants:" + event_id;
    std::string cached = cache_manager_->Get(cache_key);
    if (!cached.empty()) {
        return cached;
    }

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

    std::string result_json = userver::formats::json::ToString(builder.ExtractValue());
    cache_manager_->Set(cache_key, result_json, std::chrono::seconds(300));
    return result_json;
}

GetUserEvents::GetUserEvents(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()) {}

std::string GetUserEvents::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                             userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string cache_key = "events:user:" + user_id;
    std::string cached = cache_manager_->Get(cache_key);
    if (!cached.empty()) {
        return cached;
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

    std::string result_json = userver::formats::json::ToString(builder.ExtractValue());
    cache_manager_->Set(cache_key, result_json, std::chrono::seconds(180));
    return result_json;
}

UnregisterParticipant::UnregisterParticipant(const userver::components::ComponentConfig& config,
                                             const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()) {}

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
    cache_manager_->Delete("events:participants:" + event_id);
    cache_manager_->DeleteByPattern("events:user:*");
    return userver::formats::json::ToString(
        userver::formats::json::MakeObject("status", "unregistered"));
}

}