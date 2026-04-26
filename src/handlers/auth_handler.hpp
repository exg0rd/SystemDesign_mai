#pragma once
#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/mongo/pool.hpp>
#include <map>
#include <mutex>

namespace handlers {

class AuthMiddleware {
public:
    static AuthMiddleware& Instance();
    std::string CreateToken(const std::string& user_id);
    bool ValidateToken(const std::string& token, std::string& user_id);
    void RemoveToken(const std::string& token);

private:
    std::map<std::string, std::string> tokens_;
    std::mutex mtx_;
};

class Login : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-login";
    Login(const userver::components::ComponentConfig& config,
          const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;

private:
    userver::storages::mongo::PoolPtr mongo_pool_;
};

class Logout : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-logout";
    Logout(const userver::components::ComponentConfig& config,
           const userver::components::ComponentContext& context);
    std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                   userver::server::request::RequestContext&) const override;
};

}
