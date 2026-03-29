#include "UserHandler.h"
#include "../db/Database.h"
#include "../middleware/AuthMiddleware.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <sqlite3.h>
#include <sstream>

static std::string hashPassword(const std::string& pwd) {
    size_t h = std::hash<std::string>{}(pwd);
    std::ostringstream oss;
    oss << std::hex << h;
    return oss.str();
}

static bool checkAuth(Poco::Net::HTTPServerRequest& req, int& userId) {
    std::string auth = req.get("Authorization", "");
    if (auth.substr(0, 7) != "Bearer ") return false;
    return AuthMiddleware::instance().validateToken(auth.substr(7), userId);
}

void CreateUserHandler::handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& resp) {
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
    std::string firstName = obj->getValue<std::string>("first_name");
    std::string lastName = obj->getValue<std::string>("last_name");
    std::string email = obj->getValue<std::string>("email");
    std::string hash = hashPassword(password);

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::instance().get(),
        "INSERT INTO users(login,password_hash,first_name,last_name,email) VALUES(?,?,?,?,?)",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, firstName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, lastName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, email.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_CONSTRAINT) {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_CONFLICT);
        resp.send() << "{\"error\":\"login already exists\"}";
        return;
    }

    int newId = (int)sqlite3_last_insert_rowid(Database::instance().get());
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
    resp.send() << "{\"id\":" << newId << ",\"login\":\"" << login << "\"}";
}

void GetUserByLoginHandler::handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& resp) {
    resp.setContentType("application/json");
    int userId;
    if (!checkAuth(req, userId)) {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
        resp.send() << "{\"error\":\"unauthorized\"}";
        return;
    }

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::instance().get(),
        "SELECT id,login,first_name,last_name,email FROM users WHERE login=?",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, login_.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        resp.send() << "{\"error\":\"user not found\"}";
        return;
    }

    int id = sqlite3_column_int(stmt, 0);
    std::string login = (const char*)sqlite3_column_text(stmt, 1);
    std::string fn = (const char*)sqlite3_column_text(stmt, 2);
    std::string ln = (const char*)sqlite3_column_text(stmt, 3);
    std::string em = (const char*)sqlite3_column_text(stmt, 4);
    sqlite3_finalize(stmt);

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    resp.send() << "{\"id\":" << id << ",\"login\":\"" << login
                << "\",\"first_name\":\"" << fn << "\",\"last_name\":\"" << ln
                << "\",\"email\":\"" << em << "\"}";
}

void SearchUsersByNameHandler::handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& resp) {
    resp.setContentType("application/json");
    int userId;
    if (!checkAuth(req, userId)) {
        resp.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
        resp.send() << "{\"error\":\"unauthorized\"}";
        return;
    }

    Poco::URI uri(req.getURI());
    std::string firstName, lastName;
    for (auto& p : uri.getQueryParameters()) {
        if (p.first == "first_name") firstName = p.second;
        if (p.first == "last_name") lastName = p.second;
    }

    std::string fnMask = "%" + firstName + "%";
    std::string lnMask = "%" + lastName + "%";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::instance().get(),
        "SELECT id,login,first_name,last_name,email FROM users WHERE first_name LIKE ? AND last_name LIKE ?",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, fnMask.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, lnMask.c_str(), -1, SQLITE_TRANSIENT);

    std::ostringstream oss;
    oss << "[";
    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) oss << ",";
        first = false;
        int id = sqlite3_column_int(stmt, 0);
        std::string login = (const char*)sqlite3_column_text(stmt, 1);
        std::string fn = (const char*)sqlite3_column_text(stmt, 2);
        std::string ln = (const char*)sqlite3_column_text(stmt, 3);
        std::string em = (const char*)sqlite3_column_text(stmt, 4);
        oss << "{\"id\":" << id << ",\"login\":\"" << login
            << "\",\"first_name\":\"" << fn << "\",\"last_name\":\"" << ln
            << "\",\"email\":\"" << em << "\"}";
    }
    oss << "]";
    sqlite3_finalize(stmt);

    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    resp.send() << oss.str();
}
