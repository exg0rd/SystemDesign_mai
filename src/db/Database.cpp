#include "Database.h"
#include <stdexcept>

Database& Database::instance() {
    static Database inst;
    return inst;
}

Database::Database() : db_(nullptr) {}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

sqlite3* Database::get() {
    return db_;
}

void Database::init() {
    if (sqlite3_open(":memory:", &db_) != SQLITE_OK)
        throw std::runtime_error("Cannot open database");

    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "login TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "first_name TEXT NOT NULL,"
        "last_name TEXT NOT NULL,"
        "email TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS events ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "description TEXT,"
        "date TEXT NOT NULL,"
        "location TEXT,"
        "organizer_id INTEGER NOT NULL,"
        "FOREIGN KEY(organizer_id) REFERENCES users(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS participants ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "event_id INTEGER NOT NULL,"
        "user_id INTEGER NOT NULL,"
        "FOREIGN KEY(event_id) REFERENCES events(id),"
        "FOREIGN KEY(user_id) REFERENCES users(id)"
        ");";

    char* err = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg(err);
        sqlite3_free(err);
        throw std::runtime_error(msg);
    }
}
