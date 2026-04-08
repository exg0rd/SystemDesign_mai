#pragma once

#include <optional>
#include <string>

#include <userver/server/http/http_request.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace event_manager {

inline std::optional<int> ValidateToken(
    const userver::server::http::HttpRequest& request,
    userver::storages::postgres::ClusterPtr pg)
{
    const auto& auth = request.GetHeader("Authorization");
    if (auth.size() < 8 || auth.substr(0, 7) != "Bearer ") return std::nullopt;
    const std::string token = auth.substr(7);

    auto res = pg->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT user_id FROM sessions WHERE token = $1",
        token);
    if (res.IsEmpty()) return std::nullopt;
    return res.Front()[0].As<int>();
}

inline void Unauthorized(userver::server::http::HttpRequest& request) {
    request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kUnauthorized);
}

} 
