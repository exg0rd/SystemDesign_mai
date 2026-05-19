# Event-Driven Architecture Design

## Анализ событий в системе

### Основные события (Events)

1. **UserCreated** - событие создания нового пользователя
2. **UserDeleted** - событие удаления пользователя
3. **EventCreated** - событие создания нового события
4. **EventUpdated** - событие обновления события
5. **EventDeleted** - событие удаления события
6. **ParticipantRegistered** - событие регистрации пользователя на событие
7. **ParticipantUnregistered** - событие отмены регистрации пользователя
8. **EventCompleted** - событие завершения события (по дате)

### Основные команды (Commands)

1. **CreateUser** - команда создания пользователя
2. **DeleteUser** - команда удаления пользователя
3. **CreateEvent** - команда создания события
4. **UpdateEvent** - команда обновления события
5. **DeleteEvent** - команда удаления события
6. **RegisterParticipant** - команда регистрации на событие
7. **UnregisterParticipant** - команда отмены регистрации
8. **CompleteEvent** - команда завершения события

### Связь команд и событий

| Команда | Событие | Описание |
|---------|---------|----------|
| CreateUser | UserCreated | Пользователь успешно создан |
| DeleteUser | UserDeleted | Пользователь удален |
| CreateEvent | EventCreated | Событие успешно создано |
| UpdateEvent | EventUpdated | Событие обновлено |
| DeleteEvent | EventDeleted | Событие удалено |
| RegisterParticipant | ParticipantRegistered | Пользователь зарегистрирован |
| UnregisterParticipant | ParticipantUnregistered | Регистрация отменена |
| CompleteEvent | EventCompleted | Событие завершено |

### Сервисы-потребители событий

| Событие | Потребители | Назначение |
|---------|-------------|------------|
| UserCreated | NotificationService, AnalyticsService, SearchService | Уведомление, аналитика, индексация |
| UserDeleted | NotificationService, AnalyticsService, SearchService | Уведомление, аналитика, индексация |
| EventCreated | NotificationService, CacheService, AnalyticsService | Уведомление об организаторе, кеширование, аналитика |
| EventUpdated | NotificationService, CacheService, AnalyticsService | Уведомление об организаторе, кеширование, аналитика |
| EventDeleted | CacheService, AnalyticsService | Инвалидация кеша, аналитика |
| ParticipantRegistered | NotificationService, CacheService, AnalyticsService | Уведомление участника, кеширование, аналитика |
| ParticipantUnregistered | NotificationService, CacheService, AnalyticsService | Уведомление участника, кеширование, аналитика |
| EventCompleted | NotificationService, AnalyticsService | Уведомление участников, аналитика |

## Проектирование Event-Driven архитектуры

### Компоненты системы

#### Event Producers (Производители событий)

1. **UserService** - производит события UserCreated, UserDeleted
2. **EventService** - производит события EventCreated, EventUpdated, EventDeleted
3. **RegistrationService** - производит события ParticipantRegistered, ParticipantUnregistered
4. **EventScheduler** - производит событие EventCompleted

#### Event Consumers (Потребители событий)

1. **NotificationService** - отправляет уведомления
2. **CacheService** - управляет кешированием
3. **AnalyticsService** - собирает аналитику
4. **SearchService** - индексирует данные для поиска
5. **ReportingService** - генерирует отчеты

### Потоки событий

#### Сценарий 1: Создание пользователя

```
[HTTP Request] → UserService → MongoDB
                              ↓
                         UserCreated
                              ↓
              ┌───────────────┼───────────────┐
              ↓               ↓               ↓
    NotificationService  AnalyticsService  SearchService
```

#### Сценарий 2: Создание события

```
[HTTP Request] → EventService → MongoDB
                              ↓
                          EventCreated
                              ↓
              ┌───────────────┼───────────────┐
              ↓               ↓               ↓
    NotificationService  CacheService   AnalyticsService
```

#### Сценарий 3: Регистрация на событие

```
[HTTP Request] → RegistrationService → MongoDB
                                    ↓
                              ParticipantRegistered
                                    ↓
              ┌─────────────────────┼─────────────────────┐
              ↓                     ↓                     ↓
    NotificationService      CacheService          AnalyticsService
```

## Брокер сообщений: RabbitMQ

### Выбор RabbitMQ

**Причины выбора:**
- Простота настройки и развертывания
- Хорошая документация и сообщество
- Поддержка всех необходимых паттернов
- Подходит для средних нагрузок (до 50K msg/sec)
- Встроенные механизмы надежности

### Exchange и Queue конфигурация

#### Exchanges

| Exchange | Type | Назначение |
|----------|------|------------|
| events.direct | direct | Прямая маршрутизация событий |
| events.fanout | fanout | Широковещательная рассылка |
| events.topic | topic | Маршрутизация по шаблону |

#### Queues

| Queue | Binding Key | Потребитель |
|-------|-------------|-------------|
| notifications | user.created | NotificationService |
| notifications | event.created | NotificationService |
| notifications | participant.registered | NotificationService |
| notifications | event.completed | NotificationService |
| analytics | # | AnalyticsService |
| cache-invalidation | event.* | CacheService |
| cache-invalidation | user.* | CacheService |
| search-index | user.created | SearchService |
| search-index | event.created | SearchService |
| reporting | # | ReportingService |

### Формат сообщений

```json
{
  "event_id": "uuid",
  "event_type": "user.created",
  "timestamp": "2026-05-20T12:00:00Z",
  "payload": {
    "user_id": "uuid",
    "login": "user_login",
    "first_name": "John",
    "last_name": "Doe",
    "email": "john@example.com"
  },
  "metadata": {
    "source_service": "user-service",
    "correlation_id": "uuid",
    "trace_id": "uuid"
  }
}
```

### Гарантии доставки

**At-Least-Once Delivery:**
- Публикация с mandatory=true и immediate=false
- Подтверждение доставки (publisher confirms)
- Persistent messages на диск
- Consumer acknowledgment (ack/nack)
- Dead Letter Exchange для недоставленных сообщений

**Повторная доставка:**
- Max requeue count: 3
- TTL для сообщений: 24 часа
- DLQ для недоставленных сообщений

## Паттерн CQRS

### Применимость CQRS

**Да, CQRS применим в этой системе** по следующим причинам:

1. **Разные нагрузки на read и write операции**
   - Write: низкая частота (создание событий, регистрация)
   - Read: высокая частота (просмотр событий, поиск)

2. **Разные требования к данным**
   - Write: строгая согласованность
   - Read: событийная консистентность приемлема

3. **Разные модели данных**
   - Command model: операции над агрегатами
   - Query model: оптимизированные представления для чтения

### Разделение операций

#### Command Model (Write)

**Операции:**
- CreateUser
- DeleteUser
- CreateEvent
- UpdateEvent
- DeleteEvent
- RegisterParticipant
- UnregisterParticipant

**Хранение:**
- MongoDB (основная БД)
- Event Store (опционально для CQRS с event sourcing)

#### Query Model (Read)

**Операции:**
- GetUserById
- GetUserByLogin
- SearchUsers
- GetEventById
- GetEvents
- SearchEventsByDate
- GetEventParticipants
- GetUserEvents

**Хранение:**
- MongoDB (таблицы для read models)
- Redis (кеш для частых запросов)

### Синхронизация Read и Write моделей

```
Command Model (Write)          Event Store
       │                              │
       │ 1. Command                   │
       ├─────────────────────────────┤
       │                              │
       │ 2. Event Generated           │
       ├─────────────────────────────┤
       │                              │
       │ 3. Event Published           │
       ├─────────────────────────────┼───────────────►
       │                              │
       │                              │ 4. Event Consumed
       │                              ├───────────────►
       │                              │    Query Model Update
       │                              │
       │                              │ 5. Read Model Updated
       │                              ├───────────────►
       │                              │    Redis Cache Updated
       │                              ├───────────────►
       │                              │    Search Index Updated
```

### Event Sourcing (опционально)

Для полного CQRS с event sourcing:

**Event Store:**
- Хранит все события в неизменяемом виде
- Каждое состояние агрегата восстанавливается из событий
- Позволяет смотреть во времени и читать логи

**Проекции:**
- Read models строятся из событий
- Могут быть перестроены в любой момент
- Поддерживают событийную консистентность

## Архитектура в целом

```
┌─────────────────────────────────────────────────────────────────────┐
│                         API Gateway / HTTP Layer                     │
│  [Auth] [Users] [Events] [Registration] [Search]                   │
└─────────────────────────────────────────────────────────────────────┘
                                    │
        ┌───────────────────────────┼───────────────────────────┐
        │                           │                           │
        ▼                           ▼                           ▼
┌─────────────────┐       ┌─────────────────┐       ┌─────────────────┐
│  Command        │       │  Query          │       │  Event Bus      │
│  Service        │       │  Service        │       │  (RabbitMQ)     │
│  (Write)        │       │  (Read)         │       │                 │
│  - Validate     │       │  - Cache        │       │  Exchanges:     │
│  - Authorize    │       │  - Query DB     │       │  - events.direct│
│  - Publish      │       │  - Query Cache  │       │  - events.fanout│
│    Event        │       │                 │       │  - events.topic │
└────────┬────────┘       └─────────────────┘       └────────┬────────┘
         │                                                   │
         ▼                                                   ▼
┌─────────────────┐                                 ┌─────────────────┐
│  MongoDB        │                                 │  Consumers:     │
│  (Main Store)   │                                 │  - Notification │
│  - Users        │                                 │  - Analytics    │
│  - Events       │                                 │  - Cache        │
│  - Participants │                                 │  - Search       │
└─────────────────┘                                 │  - Reporting    │
                                                    └─────────────────┘
```

## Преимущества Event-Driven архитектуры

1. **Слабая связанность** - сервисы не зависят друг от друга
2. **Масштабируемость** - каждый потребитель масштабируется независимо
3. **Надежность** - очередь брокера гарантирует доставку
4. **Гибкость** - легко добавлять новые потребители событий
5. **Асинхронность** - улучшенная производительность и отклик

## Недостатки и компромиссы

1. **Сложность** - распределенная система сложнее монолита
2. **Eventual Consistency** - данные могут быть несогласованы временно
3. **Отладка** - сложнее отлаживать распределенные потоки
4. **Повторная доставка** - нужно обрабатывать дубликаты
