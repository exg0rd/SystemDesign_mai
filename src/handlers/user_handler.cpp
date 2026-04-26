#include "user_handler.hpp"
#include "auth_handler.hpp"
#include <userver/components/component.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/json.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/crypto/hash.hpp>

namespace handlers {

static bool CheckAuth(const userver::server::http::HttpRequest& request, std::string& user_id) {
    std::string auth = request.GetHeader("Authorization");
    if (auth.substr(0, 7) != "Bearer ") return false;
    return AuthMiddleware::Instance().ValidateToken(auth.substr(7), user_id);
}

CreateUser::CreateUser(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

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
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

std::string GetUserByLogin::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                              userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string login = request.GetPathArg("login");
    auto coll = mongo_pool_->GetCollection("users");
    auto filter = userver::formats::bson::MakeDoc("login", login);
    auto result = coll.FindOne(filter);

    if (!result) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "user not found"));
    }

    auto json = userver::formats::json::MakeObject(
        "id", (*result)["_id"].As<userver::formats::bson::Oid>().ToString(),
        "login", (*result)["login"].As<std::string>(),
        "first_name", (*result)["first_name"].As<std::string>(),
        "last_name", (*result)["last_name"].As<std::string>(),
        "email", (*result)["email"].As<std::string>()
    );
    return userver::formats::json::ToString(json);
}

SearchUsers::SearchUsers(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      mongo_pool_(context.FindComponent<userver::components::Mongo>("mongo").GetPool()) {}

std::string SearchUsers::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                           userver::server::request::RequestContext&) const {
    std::string user_id;
    if (!CheckAuth(request, user_id)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        return userver::formats::json::ToString(
            userver::formats::json::MakeObject("error", "unauthorized"));
    }

    std::string first_name = request.GetArg("first_name");
    std::string last_name = request.GetArg("last_name");

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

    return userver::formats::json::ToString(builder.ExtractValue());
}

}
