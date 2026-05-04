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
#include <random>
#include <sstream>
#include <iomanip>

namespace handlers {

AuthMiddleware& AuthMiddleware::Instance() {
    static AuthMiddleware instance;
    return instance;
}

std::string AuthMiddleware::CreateToken(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << dis(gen)
        << std::setw(16) << std::setfill('0') << dis(gen);
    std::string token = oss.str();
    tokens_[token] = user_id;
    return token;
}

bool AuthMiddleware::ValidateToken(const std::string& token, std::string& user_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = tokens_.find(token);
    if (it == tokens_.end()) return false;
    user_id = it->second;
    return true;
}

void AuthMiddleware::RemoveToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(mtx_);
    tokens_.erase(token);
}

Login::Login(const userver::components::ComponentConfig& config,
             const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()),
      cache_manager_(&context.FindComponent<cache::CacheComponent>("cache-manager").GetCacheManager()),
      rate_limiter_(&context.FindComponent<rate_limit::RateLimiterComponent>("rate-limiter").GetRateLimiter()) {}

std::string Login::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                     userver::server::request::RequestContext&) const {
    auto body = userver::formats::json::FromString(request.RequestBody());
    std::string login = body["login"].As<std::string>();
    std::string password = body["password"].As<std::string>();
    std::string hash = userver::crypto::hash::Sha256(password);

    auto coll = mongo_pool_->GetCollection("users");
    auto filter = userver::formats::bson::MakeDoc("login", login, "password_hash", hash);
    auto result = coll.FindOne(filter);

    if (!result) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "invalid credentials"));
    }

    std::string user_id = (*result)["_id"].As<userver::formats::bson::Oid>().ToString();
    std::string token = AuthMiddleware::Instance().CreateToken(user_id);

    return userver::formats::json::ToString(
        userver::formats::json::MakeObject("token", token));
}

Logout::Logout(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string Logout::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                      userver::server::request::RequestContext&) const {
    std::string auth = request.GetHeader("Authorization");
    if (auth.substr(0, 7) == "Bearer ") {
        AuthMiddleware::Instance().RemoveToken(auth.substr(7));
    }
    return "{}";
}

}