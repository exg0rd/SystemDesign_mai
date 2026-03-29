#pragma once
#include <string>
#include <sqlite3.h>

class Database {
public:
    static Database& instance();
    sqlite3* get();
    void init();

private:
    Database();
    ~Database();
    sqlite3* db_;
};
