FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    cmake g++ libpoco-dev libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel

EXPOSE 8080
CMD ["./build/event_manager"]
