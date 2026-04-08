#include "ParticipantHandler.hpp"

#include "../utils/Auth.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/exceptions.hpp>

namespace event_manager {

RegisterParticipantHandler::RegisterParticipantHandler(
    const userver::components::ComponentConfig& cfg,
    const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string RegisterParticipantHandler::HandleRequest(
    userver::server::http::HttpRequest& req,
    userver::server::request::RequestContext&) const
{
    req.GetHttpResponse().SetContentType("application/json");

    auto uid = ValidateToken(req, pg_);
    if (!uid) { Unauthorized(req); return R"({"error":"unauthorized"})"; }

    const auto& eventIdStr = req.GetPathArg("event_id");
    if (eventIdStr.empty()) {
        req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kBadRequest);
        return R"({"error":"event_id required"})";
    }
    const int eventId = std::stoi(eventIdStr);

    try {
        pg_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "INSERT INTO participants(event_id, user_id) VALUES($1, $2)",
            eventId, *uid);
    } catch (const userver::storages::postgres::UniqueViolation&) {
        req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kConflict);
        return R"({"error":"already registered"})";
    }

    req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
    userver::formats::json::ValueBuilder resp;
    resp["event_id"] = eventId;
    resp["user_id"]  = *uid;
    return userver::formats::json::ToString(resp.ExtractValue());
}

GetParticipantsHandler::GetParticipantsHandler(
    const userver::components::ComponentConfig& cfg,
    const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string GetParticipantsHandler::HandleRequest(
    userver::server::http::HttpRequest& req,
    userver::server::request::RequestContext&) const
{
    req.GetHttpResponse().SetContentType("application/json");

    auto uid = ValidateToken(req, pg_);
    if (!uid) { Unauthorized(req); return R"({"error":"unauthorized"})"; }

    const auto& eventIdStr = req.GetPathArg("event_id");
    if (eventIdStr.empty()) {
        req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kBadRequest);
        return R"({"error":"event_id required"})";
    }
    const int eventId = std::stoi(eventIdStr);

    auto res = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT u.id, u.login, u.first_name, u.last_name, u.email, p.registered_at::text "
        "FROM participants p JOIN users u ON u.id = p.user_id "
        "WHERE p.event_id = $1",
        eventId);

    userver::formats::json::ValueBuilder arr(userver::formats::json::Type::kArray);
    for (const auto& row : res) {
        userver::formats::json::ValueBuilder u;
        u["id"]              = row[0].As<int>();
        u["login"]           = row[1].As<std::string>();
        u["first_name"]      = row[2].As<std::string>();
        u["last_name"]       = row[3].As<std::string>();
        u["email"]           = row[4].As<std::string>();
        u["registered_at"]   = row[5].As<std::string>();
        arr.PushBack(u.ExtractValue());
    }
    return userver::formats::json::ToString(arr.ExtractValue());
}

GetUserEventsHandler::GetUserEventsHandler(
    const userver::components::ComponentConfig& cfg,
    const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string GetUserEventsHandler::HandleRequest(
    userver::server::http::HttpRequest& req,
    userver::server::request::RequestContext&) const
{
    req.GetHttpResponse().SetContentType("application/json");

    auto uid = ValidateToken(req, pg_);
    if (!uid) { Unauthorized(req); return R"({"error":"unauthorized"})"; }

    const int targetUserId = std::stoi(req.GetPathArg("user_id"));

    auto res = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT DISTINCT e.id, e.title, e.description, e.event_date::text, e.location, e.organizer_id "
        "FROM events e "
        "LEFT JOIN participants p ON p.event_id = e.id "
        "WHERE p.user_id = $1 OR e.organizer_id = $1 "
        "ORDER BY e.event_date::text",
        targetUserId);

    userver::formats::json::ValueBuilder arr(userver::formats::json::Type::kArray);
    for (const auto& row : res) {
        userver::formats::json::ValueBuilder e;
        e["id"]           = row[0].As<int>();
        e["title"]        = row[1].As<std::string>();
        e["description"]  = row[2].As<std::optional<std::string>>().value_or("");
        e["event_date"]   = row[3].As<std::string>();
        e["location"]     = row[4].As<std::optional<std::string>>().value_or("");
        e["organizer_id"] = row[5].As<int>();
        arr.PushBack(e.ExtractValue());
    }
    return userver::formats::json::ToString(arr.ExtractValue());
}

CancelRegistrationHandler::CancelRegistrationHandler(
    const userver::components::ComponentConfig& cfg,
    const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string CancelRegistrationHandler::HandleRequest(
    userver::server::http::HttpRequest& req,
    userver::server::request::RequestContext&) const
{
    req.GetHttpResponse().SetContentType("application/json");

    auto uid = ValidateToken(req, pg_);
    if (!uid) { Unauthorized(req); return R"({"error":"unauthorized"})"; }

    const auto& eventIdStr = req.GetPathArg("event_id");
    if (eventIdStr.empty()) {
        req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kBadRequest);
        return R"({"error":"event_id required"})";
    }
    const int eventId = std::stoi(eventIdStr);

    pg_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "DELETE FROM participants WHERE event_id=$1 AND user_id=$2",
        eventId, *uid);

    return "{}";
}

}
