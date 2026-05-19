#include "user_handler.hpp"
#include "auth_handler.hpp"
#include "../cache/cache_manager.hpp"
#include "../rate_limiter/rate_limiter.hpp"
#include <userver/components/component.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/json.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/crypto/hash.hpp>
#include "../event_producer_component.hpp"

namespace handlers {

static bool CheckAuth(const userver::server::http::HttpRequest& request, std::string& user_id) {
    std::string auth = request.GetHeader("Authorization");
    if (auth.substr(0, 7) != "Bearer ") return false;
    return AuthMiddleware::Instance().ValidateToken(auth.substr(7), user_id);
}

CreateUser::CreateUser(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()),
      producer_(&context.FindComponent<event_producer::EventProducerComponent>()) {}

std::string CreateUser::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                          userver::server::request::RequestContext&) const {
    auto body = userver::formats::json::FromString(request.RequestBody());
    std::string login = body["login"].As<std::string>();
    std::string password = body["password"].As<std::string>();
    std::string first_name = body["first_name"].As<std::string>();
    std::string last_name = body["last_name"].As<std::string>();
    std::string email = body["email"].As<std::string>();
    std::string hash = userver::crypto::hash::Sha256(password);

    auto new_id = userver::formats::bson::Oid();
    auto coll = mongo_pool_->GetCollection("users");
    auto doc = userver::formats::bson::MakeDoc(
        "_id", new_id,
        "login", login,
        "password_hash", hash,
        "first_name", first_name,
        "last_name", last_name,
        "email", email,
        "created_at", std::chrono::system_clock::now()
    );

    try {
        coll.InsertOne(doc);
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        cache_manager_->Delete("user:login:" + login);
        producer_->PublishUserCreated(new_id.ToString(), login, first_name, last_name, email);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("id", new_id.ToString(), "login", login));
    } catch (...) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "login already exists"));
    }
}

GetUserByLogin::GetUserByLogin(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()),
      rate_limiter_(&context.FindComponent<rate_limit::RateLimiterComponent>("rate-limiter").GetRateLimiter()) {}

std::string GetUserByLogin::HandleRequestThrow(const userver::server::http::HttpRequest& request,
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

    std::string login = request.GetPathArg("login");
    std::string cache_key = "user:login:" + login;
    std::string cached = cache_manager_->Get(cache_key);
    if (!cached.empty()) {
        return cached;
    }

    auto coll = mongo_pool_->GetCollection("users");
    auto filter = userver::formats::bson::MakeDoc("login", login);
    auto doc = coll.FindOne(filter);

    if (!doc) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "user not found"));
    }

    auto json = userver::formats::json::MakeObject(
        "id", (*doc)["_id"].As<userver::formats::bson::Oid>().ToString(),
        "login", (*doc)["login"].As<std::string>(),
        "first_name", (*doc)["first_name"].As<std::string>(),
        "last_name", (*doc)["last_name"].As<std::string>(),
        "email", (*doc)["email"].As<std::string>()
    );
    std::string result_json = userver::formats::json::ToString(json);
    cache_manager_->Set(cache_key, result_json, std::chrono::seconds(600));
    return result_json;
}

SearchUsers::SearchUsers(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()),
      rate_limiter_(&context.FindComponent<rate_limit::RateLimiterComponent>("rate-limiter").GetRateLimiter()) {}

std::string SearchUsers::HandleRequestThrow(const userver::server::http::HttpRequest& request,
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

    std::string first_name = request.GetArg("first_name");
    std::string last_name = request.GetArg("last_name");
    std::string cache_key = "users:search:" + first_name + ":" + last_name;
    std::string cached = cache_manager_->Get(cache_key);
    if (!cached.empty()) {
        return cached;
    }

    auto coll = mongo_pool_->GetCollection("users");
    auto filter = userver::formats::bson::MakeDoc(
        "first_name", userver::formats::bson::MakeDoc("$regex", first_name, "$options", "i"),
        "last_name", userver::formats::bson::MakeDoc("$regex", last_name, "$options", "i")
    );

    auto cursor = coll.Find(filter);
    userver::formats::json::ValueBuilder builder(userver::formats::json::Type::kArray);

    for (auto doc : cursor) {
        builder.PushBack(userver::formats::json::MakeObject(
            "id", doc["_id"].As<userver::formats::bson::Oid>().ToString(),
            "login", doc["login"].As<std::string>(),
            "first_name", doc["first_name"].As<std::string>(),
            "last_name", doc["last_name"].As<std::string>(),
            "email", doc["email"].As<std::string>()
        ));
    }

    std::string result_json = userver::formats::json::ToString(builder.ExtractValue());
    cache_manager_->Set(cache_key, result_json, std::chrono::seconds(120));
    return result_json;
}

UpdateUser::UpdateUser(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()),
      producer_(&context.FindComponent<event_producer::EventProducerComponent>()) {}

std::string UpdateUser::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                           userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string target_login = request.GetPathArg("login");
    auto body = userver::formats::json::FromString(request.RequestBody());
    std::string first_name = body["first_name"].As<std::string>();
    std::string last_name = body["last_name"].As<std::string>();
    std::string email = body["email"].As<std::string>();

    auto coll = mongo_pool_->GetCollection("users");
    auto filter = userver::formats::bson::MakeDoc("login", target_login);
    auto update = userver::formats::bson::MakeDoc(
        "$set", userver::formats::bson::MakeDoc(
            "first_name", first_name,
            "last_name", last_name,
            "email", email,
            "updated_at", std::chrono::system_clock::now()));

    auto result = coll.UpdateOne(filter, update);
    if (result.MatchedCount() == 0) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "user not found"));
    }

    cache_manager_->Delete("user:login:" + target_login);
    producer_->PublishUserUpdated(target_login, target_login, first_name, last_name, email);
    return userver::formats::json::ToString(
        userver::formats::json::MakeObject("status", "updated"));
}

DeleteUser::DeleteUser(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()),
      producer_(&context.FindComponent<event_producer::EventProducerComponent>()) {}

std::string DeleteUser::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                           userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string target_login = request.GetPathArg("login");
    auto coll = mongo_pool_->GetCollection("users");
    auto filter = userver::formats::bson::MakeDoc("login", target_login);
    auto result = coll.DeleteOne(filter);

    if (result.DeletedCount() == 0) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "user not found"));
    }

    cache_manager_->Delete("user:login:" + target_login);
    cache_manager_->DeleteByPattern("users:search:*");
    producer_->PublishUserDeleted(target_login, target_login);
    return userver::formats::json::ToString(
        userver::formats::json::MakeObject("status", "deleted"));
}

}