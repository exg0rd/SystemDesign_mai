# Лабораторная работа 6: каталог событий Event Manager


---

## События

### 1. UserCreated

**Описание:** Событие публикуется после успешного создания нового пользователя в системе.

**Структура payload:**
```json
{
  "user_id": "550e8400-e29b-41d4-a716-446655440000",
  "login": "user_login",
  "first_name": "John",
  "last_name": "Doe",
  "email": "john@example.com",
  "created_at": "2026-05-20T12:00:00Z"
}
```

**Производитель:**
- Сервис: UserService
- Команда: CreateUser
- Место публикации: src/handlers/user_handler.cpp (после InsertOne)

**Потребители:**
| Потребитель | Назначение | Гарантия |
|-------------|------------|----------|
| NotificationService | Отправка приветственного письма | At-Least-Once |
| AnalyticsService | Сбор статистики по пользователям | At-Least-Once |
| SearchService | Индексация пользователя в поисковый индекс | At-Least-Once |

**Routing Key:** `user.created`

**Exchange:** `events.direct`

**Гарантии доставки:**
- Persistent message (сохраняется на диск)
- Publisher confirms (подтверждение от брокера)
- Consumer acknowledgment (ack/nack)
- Max requeue: 3
- Dead Letter Queue для недоставленных сообщений

---

### 2. UserDeleted

**Описание:** Событие публикуется после успешного удаления пользователя из системы.

**Структура payload:**
```json
{
  "user_id": "550e8400-e29b-41d4-a716-446655440000",
  "login": "user_login",
  "deleted_at": "2026-05-20T12:00:00Z"
}
```

**Производитель:**
- Сервис: UserService
- Команда: DeleteUser
- Место публикации: src/handlers/user_handler.cpp (после DeleteOne)

**Потребители:**
| Потребитель | Назначение | Гарантия |
|-------------|------------|----------|
| NotificationService | Уведомление об удалении | At-Least-Once |
| AnalyticsService | Обновление статистики | At-Least-Once |
| SearchService | Удаление из поискового индекса | At-Least-Once |

**Routing Key:** `user.deleted`

**Exchange:** `events.direct`

**Гарантии доставки:**
- Persistent message
- Publisher confirms
- Consumer acknowledgment
- Max requeue: 3
- Dead Letter Queue

---

### 3. EventCreated

**Описание:** Событие публикуется после успешного создания нового события.

**Структура payload:**
```json
{
  "event_id": "550e8400-e29b-41d4-a716-446655440001",
  "title": "My Event",
  "description": "Event description",
  "date": "2026-12-25T00:00:00Z",
  "location": "Moscow",
  "organizer_id": "550e8400-e29b-41d4-a716-446655440000",
  "created_at": "2026-05-20T12:00:00Z"
}
```

**Производитель:**
- Сервис: EventService
- Команда: CreateEvent
- Место публикации: src/handlers/event_handler.cpp (после InsertOne)

**Потребители:**
| Потребитель | Назначение | Гарантия |
|-------------|------------|----------|
| NotificationService | Уведомление организатора | At-Least-Once |
| CacheService | Инвалидация кеша событий | At-Least-Once |
| AnalyticsService | Сбор статистики по событиям | At-Least-Once |

**Routing Key:** `event.created`

**Exchange:** `events.direct`

**Гарантии доставки:**
- Persistent message
- Publisher confirms
- Consumer acknowledgment
- Max requeue: 3
- Dead Letter Queue

---

### 4. EventUpdated

**Описание:** Событие публикуется после успешного обновления события.

**Структура payload:**
```json
{
  "event_id": "550e8400-e29b-41d4-a716-446655440001",
  "title": "Updated Event",
  "description": "Updated description",
  "date": "2026-12-26T00:00:00Z",
  "location": "St. Petersburg",
  "updated_at": "2026-05-20T12:00:00Z",
  "changes": {
    "title": {"old": "My Event", "new": "Updated Event"},
    "date": {"old": "2026-12-25", "new": "2026-12-26"}
  }
}
```

**Производитель:**
- Сервис: EventService
- Команда: UpdateEvent
- Место публикации: src/handlers/event_handler.cpp (после UpdateOne)

**Потребители:**
| Потребитель | Назначение | Гарантия |
|-------------|------------|----------|
| NotificationService | Уведомление об изменениях | At-Least-Once |
| CacheService | Инвалидация кеша | At-Least-Once |
| AnalyticsService | Отслеживание изменений | At-Least-Once |

**Routing Key:** `event.updated`

**Exchange:** `events.direct`

**Гарантии доставки:**
- Persistent message
- Publisher confirms
- Consumer acknowledgment
- Max requeue: 3
- Dead Letter Queue

---

### 5. EventDeleted

**Описание:** Событие публикуется после успешного удаления события.

**Структура payload:**
```json
{
  "event_id": "550e8400-e29b-41d4-a716-446655440001",
  "title": "Deleted Event",
  "deleted_at": "2026-05-20T12:00:00Z"
}
```

**Производитель:**
- Сервис: EventService
- Команда: DeleteEvent
- Место публикации: src/handlers/event_handler.cpp (после DeleteOne)

**Потребители:**
| Потребитель | Назначение | Гарантия |
|-------------|------------|----------|
| CacheService | Инвалидация кеша | At-Least-Once |
| AnalyticsService | Обновление статистики | At-Least-Once |

**Routing Key:** `event.deleted`

**Exchange:** `events.direct`

**Гарантии доставки:**
- Persistent message
- Publisher confirms
- Consumer acknowledgment
- Max requeue: 3
- Dead Letter Queue

---

### 6. ParticipantRegistered

**Описание:** Событие публикуется после успешной регистрации пользователя на событие.

**Структура payload:**
```json
{
  "event_id": "550e8400-e29b-41d4-a716-446655440001",
  "user_id": "550e8400-e29b-41d4-a716-446655440000",
  "login": "user_login",
  "first_name": "John",
  "last_name": "Doe",
  "email": "john@example.com",
  "registered_at": "2026-05-20T12:00:00Z"
}
```

**Производитель:**
- Сервис: RegistrationService
- Команда: RegisterParticipant
- Место публикации: src/handlers/event_handler.cpp (после UpdateOne с $push)

**Потребители:**
| Потребитель | Назначение | Гарантия |
|-------------|------------|----------|
| NotificationService | Уведомление участника | At-Least-Once |
| CacheService | Инвалидация кеша участников | At-Least-Once |
| AnalyticsService | Сбор статистики по регистрациям | At-Least-Once |

**Routing Key:** `participant.registered`

**Exchange:** `events.direct`

**Гарантии доставки:**
- Persistent message
- Publisher confirms
- Consumer acknowledgment
- Max requeue: 3
- Dead Letter Queue

---

### 7. ParticipantUnregistered

**Описание:** Событие публикуется после успешной отмены регистрации пользователя.

**Структура payload:**
```json
{
  "event_id": "550e8400-e29b-41d4-a716-446655440001",
  "user_id": "550e8400-e29b-41d4-a716-446655440000",
  "login": "user_login",
  "unregistered_at": "2026-05-20T12:00:00Z"
}
```

**Производитель:**
- Сервис: RegistrationService
- Команда: UnregisterParticipant
- Место публикации: src/handlers/event_handler.cpp (после UpdateOne с $pull)

**Потребители:**
| Потребитель | Назначение | Гарантия |
|-------------|------------|----------|
| NotificationService | Уведомление об отмене | At-Least-Once |
| CacheService | Инвалидация кеша участников | At-Least-Once |
| AnalyticsService | Обновление статистики | At-Least-Once |

**Routing Key:** `participant.unregistered`

**Exchange:** `events.direct`

**Гарантии доставки:**
- Persistent message
- Publisher confirms
- Consumer acknowledgment
- Max requeue: 3
- Dead Letter Queue

---

### 8. EventCompleted

**Описание:** Событие публикуется при завершении события (по дате).

**Структура payload:**
```json
{
  "event_id": "550e8400-e29b-41d4-a716-446655440001",
  "title": "Completed Event",
  "date": "2026-05-20T00:00:00Z",
  "completed_at": "2026-05-20T12:00:00Z"
}
```

**Производитель:**
- Сервис: EventScheduler
- Команда: CompleteEvent
- Место публикации: отдельный сервис-планировщик

**Потребители:**
| Потребитель | Назначение | Гарантия |
|-------------|------------|----------|
| NotificationService | Уведомление участников о завершении | At-Least-Once |
| AnalyticsService | Завершение сбора статистики | At-Least-Once |

**Routing Key:** `event.completed`

**Exchange:** `events.direct`

**Гарантии доставки:**
- Persistent message
- Publisher confirms
- Consumer acknowledgment
- Max requeue: 3
- Dead Letter Queue

---

## Сводная таблица событий

| # | Событие | Routing Key | Exchange | Производитель | Потребители |
|---|---------|-------------|----------|---------------|-------------|
| 1 | UserCreated | user.created | events.direct | UserService | Notification, Analytics, Search |
| 2 | UserDeleted | user.deleted | events.direct | UserService | Notification, Analytics, Search |
| 3 | EventCreated | event.created | events.direct | EventService | Notification, Cache, Analytics |
| 4 | EventUpdated | event.updated | events.direct | EventService | Notification, Cache, Analytics |
| 5 | EventDeleted | event.deleted | events.direct | EventService | Cache, Analytics |
| 6 | ParticipantRegistered | participant.registered | events.direct | RegistrationService | Notification, Cache, Analytics |
| 7 | ParticipantUnregistered | participant.unregistered | events.direct | RegistrationService | Notification, Cache, Analytics |
| 8 | EventCompleted | event.completed | events.direct | EventScheduler | Notification, Analytics |

---

## Формат метаданных событий

Все события содержат следующие метаданные:

```json
{
  "event_id": "uuid-v4",
  "event_type": "user.created",
  "timestamp": "ISO-8601",
  "payload": { ... },
  "metadata": {
    "source_service": "user-service",
    "correlation_id": "uuid-v4",
    "trace_id": "uuid-v4",
    "version": "1.0"
  }
}
```

**Поля метаданных:**
- `event_id` - уникальный идентификатор события
- `event_type` - тип события (routing key)
- `timestamp` - время создания события (ISO-8601)
- `source_service` - имя сервиса-производителя
- `correlation_id` - идентификатор цепочки запросов
- `trace_id` - идентификатор трассировки
- `version` - версия схемы события

---

## Гарантии доставки

### Для всех событий:

1. **Publisher Side:**
   - Persistent messages (delivery_mode=2)
   - Publisher confirms (RabbitMQ)
   - Mandatory flag для обнаружения недоставимых сообщений

2. **Broker Side:**
   - Durable queues (persistent=true)
   - HA configuration (mirrored queues)
   - Dead Letter Exchange для недоставленных сообщений

3. **Consumer Side:**
   - Manual acknowledgment (ack/nack)
   - Requeue with limit (max 3 attempts)
   - DLQ для окончательно недоставленных сообщений

4. **Idempotency:**
   - Все потребители идемпотентны
   - Проверка по event_id для дубликатов
   - Логирование обработанных событий

---

## Мониторинг событий

### Метрики:

| Метрика | Описание | Алерт |
|---------|----------|-------|
| events_published_total | Всего опубликованных событий | - |
| events_delivered_total | Всего доставленных событий | - |
| events_failed_total | Всего недоставленных событий | > 100/ч |
| events_in_dlq | Событий в DLQ | > 10 |
| consumer_lag | Задержка потребителей | > 1000 |
| message_size_bytes | Размер сообщений | > 1MB |

### Логирование:

- Все события логируются с уровнем INFO
- Ошибки доставки - уровень ERROR
- DLQ - уровень WARN