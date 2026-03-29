cmake_minimum_required(VERSION 3.16)
project(event_manager)

set(CMAKE_CXX_STANDARD 17)

find_package(Poco REQUIRED Net Util JSON)

add_executable(event_manager
    src/main.cpp
    src/db/Database.cpp
    src/middleware/AuthMiddleware.cpp
    src/handlers/AuthHandler.cpp
    src/handlers/UserHandler.cpp
    src/handlers/EventHandler.cpp
)

target_include_directories(event_manager PRIVATE src)
target_link_libraries(event_manager
    Poco::Net
    Poco::Util
    Poco::JSON
    sqlite3
)
