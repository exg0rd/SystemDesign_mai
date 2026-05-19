FROM ghcr.io/userver-framework/ubuntu-22.04-userver-base:latest

RUN apt-get update && apt-get install -y \
    libzstd-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN chmod +x entrypoint.sh

RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DUSERVER_FEATURE_POSTGRESQL=OFF \
        -DUSERVER_FEATURE_MONGODB=ON \
        -DUSERVER_FEATURE_REDIS=OFF \
        -DUSERVER_FEATURE_GRPC=OFF \
        -DUSERVER_FEATURE_CLICKHOUSE=OFF \
        -DUSERVER_FEATURE_KAFKA=OFF \
        -DUSERVER_FEATURE_RABBITMQ=ON \
    && cmake --build build --parallel $(nproc)

EXPOSE 8080

CMD ["/app/entrypoint.sh"]