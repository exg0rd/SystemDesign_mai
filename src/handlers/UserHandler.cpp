#include "UserHandler.hpp"

#include "../utils/Auth.hpp"

#include <userver/crypto/hash.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/exceptions.hpp>

namespace event_manager {

CreateUserHandler::CreateUserHandler(const userver::components::ComponentConfig& cfg,
                                     const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string CreateUserHandler::HandleRequest(userver::server::http::HttpRequest& req,
                                             userver::server::request::RequestContext&) const {
    req.GetHttpResponse().SetContentType("application/json");

    const auto body      = userver::formats::json::FromString(req.RequestBody());
    const auto login     = body["login"].As<std::string>();
    const auto password  = body["password"].As<std::string>();
    const auto firstName = body["first_name"].As<std::string>();
    const auto lastName  = body["last_name"].As<std::string>();
    const auto email     = body["email"].As<std::string>();
    const auto hash      = userver::crypto::hash::Sha256(password);

    try {
        auto res = pg_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "INSERT INTO users(login,password_hash,first_name,last_name,email) "
            "VALUES($1,$2,$3,$4,$5) RETURNING id",
            login, hash, firstName, lastName, email);

        const int newId = res.Front()[0].As<int>();
        req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
        userver::formats::json::ValueBuilder resp;
        resp["id"]    = newId;
        resp["login"] = login;
        return userver::formats::json::ToString(resp.ExtractValue());
    } catch (const userver::storages::postgres::UniqueViolation&) {
        req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kConflict);
        return R"({"error":"login or email already exists"})";
    }
}

GetUserByLoginHandler::GetUserByLoginHandler(const userver::components::ComponentConfig& cfg,
                                             const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string GetUserByLoginHandler::HandleRequest(userver::server::http::HttpRequest& req,
                                                 userver::server::request::RequestContext&) const {
    req.GetHttpResponse().SetContentType("application/json");

    auto uid = ValidateToken(req, pg_);
    if (!uid) { Unauthorized(req); return R"({"error":"unauthorized"})"; }

    const auto login = req.GetPathArg("login");
    auto res = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT id,login,first_name,last_name,email FROM users WHERE login=$1",
        login);

    if (res.IsEmpty()) {
        req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kNotFound);
        return R"({"error":"user not found"})";
    }

    const auto row = res.Front();
    userver::formats::json::ValueBuilder resp;
    resp["id"]         = row[0].As<int>();
    resp["login"]      = row[1].As<std::string>();
    resp["first_name"] = row[2].As<std::string>();
    resp["last_name"]  = row[3].As<std::string>();
    resp["email"]      = row[4].As<std::string>();
    return userver::formats::json::ToString(resp.ExtractValue());
}

SearchUsersHandler::SearchUsersHandler(const userver::components::ComponentConfig& cfg,
                                       const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string SearchUsersHandler::HandleRequest(userver::server::http::HttpRequest& req,
                                              userver::server::request::RequestContext&) const {
    req.GetHttpResponse().SetContentType("application/json");

    auto uid = ValidateToken(req, pg_);
    if (!uid) { Unauthorized(req); return R"({"error":"unauthorized"})"; }

    const auto fn = "%" + req.GetArg("first_name") + "%";
    const auto ln = "%" + req.GetArg("last_name")  + "%";

    auto res = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT id,login,first_name,last_name,email FROM users "
        "WHERE first_name ILIKE $1 AND last_name ILIKE $2",
        fn, ln);

    userver::formats::json::ValueBuilder arr(userver::formats::json::Type::kArray);
    for (const auto& row : res) {
        userver::formats::json::ValueBuilder u;
        u["id"]         = row[0].As<int>();
        u["login"]      = row[1].As<std::string>();
        u["first_name"] = row[2].As<std::string>();
        u["last_name"]  = row[3].As<std::string>();
        u["email"]      = row[4].As<std::string>();
        arr.PushBack(u.ExtractValue());
    }
    return userver::formats::json::ToString(arr.ExtractValue());
}

} 
