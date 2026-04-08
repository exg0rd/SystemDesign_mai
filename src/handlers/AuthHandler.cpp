#include "AuthHandler.hpp"

#include <iomanip>
#include <random>
#include <sstream>

#include <userver/crypto/hash.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace event_manager {

namespace {

std::string GenerateToken() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << dist(gen)
        << std::setw(16) << std::setfill('0') << dist(gen);
    return oss.str();
}

}  // namespace

LoginHandler::LoginHandler(const userver::components::ComponentConfig& cfg,
                           const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string LoginHandler::HandleRequest(userver::server::http::HttpRequest& req,
                                        userver::server::request::RequestContext&) const {
    req.GetHttpResponse().SetContentType("application/json");

    const auto body = userver::formats::json::FromString(req.RequestBody());
    const auto login    = body["login"].As<std::string>();
    const auto password = body["password"].As<std::string>();
    const auto hash     = userver::crypto::hash::Sha256(password);

    auto res = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT id FROM users WHERE login=$1 AND password_hash=$2",
        login, hash);

    if (res.IsEmpty()) {
        req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kUnauthorized);
        return R"({"error":"invalid credentials"})";
    }

    const int userId = res.Front()[0].As<int>();
    const std::string token = GenerateToken();

    pg_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "INSERT INTO sessions(token, user_id) VALUES($1, $2)",
        token, userId);

    userver::formats::json::ValueBuilder resp;
    resp["token"] = token;
    return userver::formats::json::ToString(resp.ExtractValue());
}

LogoutHandler::LogoutHandler(const userver::components::ComponentConfig& cfg,
                             const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string LogoutHandler::HandleRequest(userver::server::http::HttpRequest& req,
                                         userver::server::request::RequestContext&) const {
    req.GetHttpResponse().SetContentType("application/json");
    const auto& auth = req.GetHeader("Authorization");
    if (auth.size() > 7 && auth.substr(0, 7) == "Bearer ") {
        pg_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "DELETE FROM sessions WHERE token=$1",
            auth.substr(7));
    }
    return "{}";
}

}  
