#include "EventHandler.h"
#include "../db/Database.h"
#include "../middleware/AuthMiddleware.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPResponse.h>
#include <sqlite3.h>
#include <sstream>

static bool checkAuth(Poco::Net::HTTPServerRequest& req, int& userId) {
    std::string auth = req.get("Authorization", "");
    if (auth.substr(0, 7) != "Bearer ") return false;
    return AuthMiddleware::instance().validateToken(auth.substr(7), userId);
}

void CreateEventHandler::handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& resp) {
    resp.setContentType("application/json");
    int userId;
    if (!checkAuth(req, userId)) {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
        resp.send() << "{\"error\":\"unauthorized\"}";
        return;
    }
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
    std::string title = obj->getValue<std::string>("title");
    std::string description = obj->optValue<std::string>("description", "");
    std::string date = obj->getValue<std::string>("date");
    std::string location = obj->optValue<std::string>("location", "");

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::instance().get(),
        "INSERT INTO events(title,description,date,location,organizer_id) VALUES(?,?,?,?,?)",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, location.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, userId);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    int newId = (int)sqlite3_last_insert_rowid(Database::instance().get());
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
    resp.send() << "{\"id\":" << newId << ",\"title\":\"" << title << "\"}";
}

void GetEventsHandler::handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& resp) {
    resp.setContentType("application/json");
    int userId;
    if (!checkAuth(req, userId)) {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
        resp.send() << "{\"error\":\"unauthorized\"}";
        return;
    }

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::instance().get(),
        "SELECT id,title,description,date,location,organizer_id FROM events",
        -1, &stmt, nullptr);

    std::ostringstream oss;
    oss << "[";
    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) oss << ",";
        first = false;
        int id = sqlite3_column_int(stmt, 0);
        std::string title = (const char*)sqlite3_column_text(stmt, 1);
        std::string desc = sqlite3_column_text(stmt, 2) ? (const char*)sqlite3_column_text(stmt, 2) : "";
        std::string date = (const char*)sqlite3_column_text(stmt, 3);
        std::string loc = sqlite3_column_text(stmt, 4) ? (const char*)sqlite3_column_text(stmt, 4) : "";
        int orgId = sqlite3_column_int(stmt, 5);
        oss << "{\"id\":" << id << ",\"title\":\"" << title
            << "\",\"description\":\"" << desc << "\",\"date\":\"" << date
            << "\",\"location\":\"" << loc << "\",\"organizer_id\":" << orgId << "}";
    }
    oss << "]";
    sqlite3_finalize(stmt);

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    resp.send() << oss.str();
}
