# Лабораторная работа 6: разработка ED архитектуры

Проектирование и реализация событийно-ориентированной архитектуры с использованием RabbitMQ и паттерна CQRS.

## Структура проекта

```
.
├── event_driven_design.md      # Описание Event-Driven архитектуры
├── event_catalog.md            # Каталог событий
├── docker-compose.yml          # Docker Compose для RabbitMQ
├── Dockerfile.event-producer   # Dockerfile для producer
├── Dockerfile.event-consumer   # Dockerfile для consumer
├── src/
│   ├── event_producer.hpp      # Интерфейс producer
│   ├── event_producer.cpp      # Реализация producer
│   ├── event_consumer.hpp      # Интерфейс consumer
│   └── event_consumer.cpp      # Реализация consumer
└── README.md                   # Этот файл
```

## Архитектура

### Event-Driven компоненты

1. **Event Producers** - производители событий
   - UserService - события UserCreated, UserDeleted
   - EventService - события EventCreated, EventUpdated, EventDeleted
   - RegistrationService - события ParticipantRegistered, ParticipantUnregistered
   - EventScheduler - событие EventCompleted

2. **Event Consumers** - потребители событий
   - NotificationService - уведомления
   - AnalyticsService - аналитика
   - CacheService - инвалидация кеша
   - SearchService - индексация
   - ReportingService - отчеты

3. **Message Broker** - RabbitMQ
   - Exchanges: direct, fanout, topic
   - Queues: notifications, analytics, cache-invalidation, search-index, reporting

### CQRS

- **Command Model** - операции записи (CreateUser, CreateEvent, RegisterParticipant)
- **Query Model** - операции чтения (GetUser, GetEvents, SearchUsers)
- События синхронизируют read и write модели

## Запуск

### С помощью Docker Compose

```bash
docker-compose up -d
```

### Проверка RabbitMQ

```bash
# Проверка статуса
docker-compose ps

# Логи RabbitMQ
docker-compose logs rabbitmq

# Доступ к management console
# http://localhost:15672
# user: guest
# pass: guest
```

## События

### 1. UserCreated
- Routing Key: `user.created`
- Exchange: `events.direct`
- Потребители: NotificationService, AnalyticsService, SearchService

### 2. UserDeleted
- Routing Key: `user.deleted`
- Exchange: `events.direct`
- Потребители: NotificationService, AnalyticsService, SearchService

### 3. EventCreated
- Routing Key: `event.created`
- Exchange: `events.direct`
- Потребители: NotificationService, CacheService, AnalyticsService

### 4. EventUpdated
- Routing Key: `event.updated`
- Exchange: `events.direct`
- Потребители: NotificationService, CacheService, AnalyticsService

### 5. EventDeleted
- Routing Key: `event.deleted`
- Exchange: `events.direct`
- Потребители: CacheService, AnalyticsService

### 6. ParticipantRegistered
- Routing Key: `participant.registered`
- Exchange: `events.direct`
- Потребители: NotificationService, CacheService, AnalyticsService

### 7. ParticipantUnregistered
- Routing Key: `participant.unregistered`
- Exchange: `events.direct`
- Потребители: NotificationService, CacheService, AnalyticsService

### 8. EventCompleted
- Routing Key: `event.completed`
- Exchange: `events.direct`
- Потребители: NotificationService, AnalyticsService

## Гарантии доставки

- **At-Least-Once** доставка
- Persistent messages на диск
- Publisher confirms
- Consumer acknowledgment (ack/nack)
- Max requeue: 3
- Dead Letter Queue для недоставленных сообщений

## Использование

### Компиляция

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### Запуск producer

```bash
./build/event_manager
```

### Запуск consumer

```bash
./build/event_manager
```

## Документация

- `event_driven_design.md` - полное описание Event-Driven архитектуры
- `event_catalog.md` - каталог всех событий с описанием

## Требования

- Docker и Docker Compose
- C++20 компилятор
- CMake 3.16+
- userver framework
- RabbitMQ C client library (librabbitmq)

## Критерии оценки

- Корректность определения событий и команд
- Качество проектирования Event-Driven архитектуры
- Правильность выбора типов exchange и routing
- Применение паттерна CQRS
- Качество каталога событий
- Работоспособность реализации
