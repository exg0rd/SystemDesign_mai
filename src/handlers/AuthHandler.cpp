#include "AuthHandler.h"
#include "../db/Database.h"
#include "../middleware/AuthMiddleware.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPResponse.h>
#include <sstream>
#include <sqlite3.h>

static std::string hashPassword(const std::string& pwd) {
    size_t h = std::hash<std::string>{}(pwd);
    std::ostringstream oss;
    oss << std::hex << h;
    return oss.str();
}

void LoginHandler::handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& resp) {
    resp.setContentType("application/json");
    if (req.getMethod() != "POST") {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
        resp.send() << "{}";
        return;
    }
    std::istream& body = req.stream();
    std::string bodyStr((std::istreambuf_iterator<char>(body)), std::istreambuf_iterator<char>());

    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result;
    try {
        result = parser.parse(bodyStr);
    } catch (...) {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        resp.send() << "{\"error\":\"invalid json\"}";
        return;
    }

    auto obj = result.extract<Poco::JSON::Object::Ptr>();
    std::string login = obj->getValue<std::string>("login");
    std::string password = obj->getValue<std::string>("password");
    std::string hash = hashPassword(password);

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::instance().get(),
        "SELECT id FROM users WHERE login=? AND password_hash=?", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_TRANSIENT);

    int userId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        userId = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (userId == -1) {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
        resp.send() << "{\"error\":\"invalid credentials\"}";
        return;
    }

    std::string token = AuthMiddleware::instance().createToken(userId, login);
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    resp.send() << "{\"token\":\"" << token << "\"}";
}

void LogoutHandler::handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& resp) {
    resp.setContentType("application/json");
    std::string auth = req.get("Authorization", "");
    if (auth.substr(0, 7) == "Bearer ")
        AuthMiddleware::instance().removeToken(auth.substr(7));
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    resp.send() << "{}";
}
