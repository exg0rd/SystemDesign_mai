cmake_minimum_required(VERSION 3.16)
project(event_manager CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(userversource/userver)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/handlers/user_handler.cpp
    src/handlers/event_handler.cpp
    src/handlers/auth_handler.cpp
)

target_link_libraries(${PROJECT_NAME} userver::mongo)
