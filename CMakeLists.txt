cmake_minimum_required(VERSION 3.16)
project(event_manager CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(userver)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/handlers/user_handler.cpp
    src/handlers/event_handler.cpp
    src/handlers/auth_handler.cpp
    src/cache/cache_manager.cpp
    src/cache/cache_component.cpp
    src/rate_limiter/rate_limiter.cpp
    src/rate_limiter/rate_limiter_component.cpp
    src/event_producer_component.cpp
    src/event_consumer_component.cpp
)

target_link_libraries(${PROJECT_NAME}
    userver::mongo
    userver::core
    userver::rabbitmq)
