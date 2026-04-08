#include "EventHandler.hpp"

#include "../utils/Auth.hpp"

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

namespace event_manager {

namespace {

userver::formats::json::Value EventRowToJson(const userver::storages::postgres::Row& row) {
    userver::formats::json::ValueBuilder e;
    e["id"]           = row[0].As<int>();
    e["title"]        = row[1].As<std::string>();
    e["description"]  = row[2].As<std::optional<std::string>>().value_or("");
    e["event_date"]   = row[3].As<std::string>();
    e["location"]     = row[4].As<std::optional<std::string>>().value_or("");
    e["organizer_id"] = row[5].As<int>();
    return e.ExtractValue();
}

} 

CreateEventHandler::CreateEventHandler(const userver::components::ComponentConfig& cfg,
                                       const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string CreateEventHandler::HandleRequest(userver::server::http::HttpRequest& req,
                                              userver::server::request::RequestContext&) const {
    req.GetHttpResponse().SetContentType("application/json");

    auto uid = ValidateToken(req, pg_);
    if (!uid) { Unauthorized(req); return R"({"error":"unauthorized"})"; }

    const auto body        = userver::formats::json::FromString(req.RequestBody());
    const auto title       = body["title"].As<std::string>();
    const auto description = body["description"].As<std::optional<std::string>>().value_or("");
    const auto eventDate   = body["event_date"].As<std::string>();
    const auto location    = body["location"].As<std::optional<std::string>>().value_or("");

    auto res = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "INSERT INTO events(title,description,event_date,location,organizer_id) "
        "VALUES($1,$2,$3::date,$4,$5) RETURNING id",
        title, description, eventDate, location, *uid);

    const int newId = res.Front()[0].As<int>();
    req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
    userver::formats::json::ValueBuilder resp;
    resp["id"]    = newId;
    resp["title"] = title;
    return userver::formats::json::ToString(resp.ExtractValue());
}

GetEventsHandler::GetEventsHandler(const userver::components::ComponentConfig& cfg,
                                   const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string GetEventsHandler::HandleRequest(userver::server::http::HttpRequest& req,
                                            userver::server::request::RequestContext&) const {
    req.GetHttpResponse().SetContentType("application/json");

    auto uid = ValidateToken(req, pg_);
    if (!uid) { Unauthorized(req); return R"({"error":"unauthorized"})"; }

    auto res = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT id,title,description,event_date::text,location,organizer_id "
        "FROM events ORDER BY event_date");

    userver::formats::json::ValueBuilder arr(userver::formats::json::Type::kArray);
    for (const auto& row : res) arr.PushBack(EventRowToJson(row));
    return userver::formats::json::ToString(arr.ExtractValue());
}

SearchEventsByDateHandler::SearchEventsByDateHandler(const userver::components::ComponentConfig& cfg,
                                                     const userver::components::ComponentContext& ctx)
    : HttpHandlerBase(cfg, ctx),
      pg_(ctx.FindComponent<userver::components::Postgres>("event-db").GetCluster()) {}

std::string SearchEventsByDateHandler::HandleRequest(userver::server::http::HttpRequest& req,
                                                     userver::server::request::RequestContext&) const {
    req.GetHttpResponse().SetContentType("application/json");

    auto uid = ValidateToken(req, pg_);
    if (!uid) { Unauthorized(req); return R"({"error":"unauthorized"})"; }

    const auto date = req.GetArg("date");
    if (date.empty()) {
        req.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kBadRequest);
        return R"({"error":"date query param required"})";
    }

    auto res = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT id,title,description,event_date::text,location,organizer_id "
        "FROM events WHERE event_date=$1::date",
        date);

    userver::formats::json::ValueBuilder arr(userver::formats::json::Type::kArray);
    for (const auto& row : res) arr.PushBack(EventRowToJson(row));
    return userver::formats::json::ToString(arr.ExtractValue());
}

}
