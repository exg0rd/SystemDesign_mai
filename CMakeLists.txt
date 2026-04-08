cmake_minimum_required(VERSION 3.16)
project(event_manager CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(userversource/userver)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/handlers/AuthHandler.cpp
    src/handlers/UserHandler.cpp
    src/handlers/EventHandler.cpp
    src/handlers/ParticipantHandler.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} userver::postgresql)
