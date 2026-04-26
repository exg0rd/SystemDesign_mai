FROM ghcr.io/userver-framework/ubuntu-22.04-userver-base:latest

WORKDIR /app
COPY . .

RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DUSERVER_FEATURE_POSTGRESQL=OFF \
        -DUSERVER_FEATURE_MONGODB=ON \
        -DUSERVER_FEATURE_REDIS=OFF \
        -DUSERVER_FEATURE_GRPC=OFF \
        -DUSERVER_FEATURE_CLICKHOUSE=OFF \
        -DUSERVER_FEATURE_KAFKA=OFF \
        -DUSERVER_FEATURE_RABBITMQ=OFF \
    && cmake --build build --parallel $(nproc)

EXPOSE 8080
CMD ["./build/event_manager", "--config", "configs/static_config.yaml", "--config_vars", "configs/config_vars.yaml"]
