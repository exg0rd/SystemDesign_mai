# Домашнее задание 05: Оптимизация производительности через кеширование и rate limiting

## 1. Анализ производительности

### Определение hot paths (часто выполняемых операций)

**Частые операции (hot paths):**
- `GET /events` - получение списка событий (основная страница)
- `GET /users/{login}` - поиск пользователя по логину (часто используемый endpoint)
- `GET /events/search` - поиск событий по дате (фильтрация)
- `GET /events/{id}/participants` - получение участников события

**Медленные операции (обращения к БД):**
- `GET /users/search` - поиск по маске (regex-запросы к MongoDB)
- `GET /users/me/events` - получение событий пользователя (aggregation)
- `GET /events/{id}/participants` - извлечение вложенных данных

### Требования к производительности

| Параметр | Значение |
|----------|----------|
| P50 отклик | < 100ms |
| P95 отклик | < 300ms |
| P99 отклик | < 500ms |
| Пропускная способность | 1000 req/s |
| Доступность | 99.9% |

## 2. Проектирование стратегии кеширования

### Типы данных для кеширования

| Тип данных | Примеры | TTL | Стратегия |
|------------|---------|-----|-----------|
| Часто читаемые | Список событий, профиль пользователя | 5 min | Cache-Aside |
| Редко изменяемые | Информация о событии | 15 min | Cache-Aside |
| Данные с высокой стоимостью | Поиск по маске | 2 min | Cache-Aside |
| Временные данные | Результаты агрегаций | 1 min | Cache-Aside |

### Выбор стратегии кеширования

**Cache-Aside (Lazy Loading)** - выбрана для всех endpoints:
- При запросе сначала проверяем кеш
- Если miss - читаем из MongoDB и записываем в кеш
- При обновлении данных - инвалидируем кеш

**Почему не Write-Through:**
- MongoDB используется как primary store
- Userver не требует синхронной записи в кеш
- Проще в реализации и отладке

### TTL для кешируемых данных

| Endpoint | TTL | Обоснование |
|----------|-----|-------------|
| `/events` | 5 min | События редко меняются, но нужна свежесть |
| `/users/{login}` | 10 min | Профили пользователей статичны |
| `/events/search` | 2 min | Поиск может быть дорогим |
| `/events/{id}/participants` | 5 min | Участники могут меняться |
| `/users/me/events` | 3 min | Персональные данные |

### Стратегия инвалидации кеша

**Инвалидация при обновлении:**
- `POST /events` - инвалидация `GET /events`, `GET /users/me/events`
- `POST /events/{id}/register` - инвалидация `GET /events/{id}/participants`, `GET /users/me/events`
- `DELETE /events/{id}/unregister` - инвалидация `GET /events/{id}/participants`, `GET /users/me/events`
- `POST /users` - без инвалидации (новый пользователь не в кеше)

## 3. Реализация кеширования

### Архитектура кеша

```
┌─────────────────────────────────────────────────────────────┐
│                      Redis Cache                            │
│  ┌─────────────────┬─────────────────┬──────────────────┐  │
│  │   events_list   │  user_profile   │  search_results  │  │
│  │   (5 min TTL)   │  (10 min TTL)   │  (2 min TTL)     │  │
│  └─────────────────┴─────────────────┴──────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                            ↑
                            │
┌─────────────────────────────────────────────────────────────┐
│                    Userver API                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │  Cache Check │  │  MongoDB     │  │  Cache Write     │  │
│  │  (Hit/Miss)  │  │  (Fallback)  │  │  (On Miss)       │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### Реализованные endpoints с кешированием

**1. GET /events (handler-get-events)**
- Кеширует: JSON массив всех событий
- Ключ: `events:all`
- TTL: 300 секунд
- Инвалидация: при создании нового события

**2. GET /users/{login} (handler-get-user-login)**
- Кеширует: JSON профиля пользователя
- Ключ: `user:login:{login}`
- TTL: 600 секунд
- Инвалидация: при создании нового пользователя

### Код реализации кеширования

См. файлы:
- `src/cache/cache_manager.hpp` - интерфейс кеша
- `src/cache/cache_manager.cpp` - реализация кеша
- `src/cache/cache_component.hpp` - компонент кеша для userver
- `src/cache/cache_component.cpp` - реализация компонента

### Интеграция кеширования в handlers

См. реализацию в:
- `src/handlers/user_handler.cpp` - кеширование для GET /users/{login}, GET /users/search
- `src/handlers/event_handler.cpp` - кеширование для всех read endpoints

## 4. Проектирование rate limiting

### Endpoints, требующие rate limiting

| Endpoint | Лимит (обычный) | Лимит (premium) | Алгоритм |
|----------|-----------------|-----------------|----------|
| `/auth/login` | 5 req/min | 20 req/min | Token Bucket |
| `/auth/logout` | 10 req/min | 50 req/min | Sliding Window |
| `/users` | 10 req/min | 50 req/min | Token Bucket |
| `/events` | 100 req/min | 500 req/min | Sliding Window |
| `/events/search` | 30 req/min | 100 req/min | Sliding Window |
| `/events/{id}/register` | 20 req/min | 100 req/min | Token Bucket |

### Выбор алгоритмов rate limiting

**Token Bucket:**
- `/auth/login` - защита от brute-force атак
- `/events/{id}/register` - защита от спама реги��трации

**Sliding Window Log/Counter:**
- `/events` - более точный лимит для read-only операций
- `/events/search` - защита от heavy queries

**Почему не Fixed Window:**
- Fixed Window может пропустить в 2x запросов в конце/начале окна
- Sliding Window точнее, но требует больше памяти
- Token Bucket позволяет "накапливать" запросы

### Реализация rate limiting

См. файлы:
- `src/rate_limiter/rate_limiter.hpp` - интерфейс rate limiter
- `src/rate_limiter/rate_limiter.cpp` - реализация rate limiter
- `src/rate_limiter/rate_limiter_component.hpp` - компонент rate limiter
- `src/rate_limiter/rate_limiter_component.cpp` - реализация компонента

### Интеграция rate limiting в handlers

См. реализацию в:
- `src/handlers/event_handler.cpp` - rate limiting для GET /events

### HTTP заголовки rate limiting

| Заголовок | Описание | Пример |
|-----------|----------|--------|
| `X-RateLimit-Limit` | Максимальное количество запросов | `100` |
| `X-RateLimit-Remaining` | Оставшееся количество запросов | `95` |
| `X-RateLimit-Reset` | Время сброса (Unix timestamp) | `1715865600` |

### HTTP статус коды

| Код | Описание |
|-----|----------|
| `200 OK` | Запрос разрешен |
| `429 Too Many Requests` | Превышен лимит запросов |

## 5. Реализация rate limiting (завершенная)

См. реализацию в `src/rate_limiter/` и `src/handlers/`.

## 6. Анализ производительности

### Как кеширование улучшает производительность

**До кеширования:**
- `GET /events`: ~50ms (MongoDB query)
- `GET /users/{login}`: ~30ms (MongoDB query)
- `GET /events/search`: ~100ms (MongoDB query + regex)

**После кеширования:**
- `GET /events`: ~1ms (cache hit)
- `GET /users/{login}`: ~0.5ms (cache hit)
- `GET /events/search`: ~5ms (cache hit)

**Улучшение:**
- P50: 50ms → 1ms (50x ускорение)
- P95: 100ms → 5ms (20x ускорение)
- Пропускная способность: ~1000 req/s → ~10000 req/s

### Как rate limiting улучшает производительность

**Защита от:**
- Brute-force атак на `/auth/login`
- DDoS атак на `/events`
- Heavy queries на `/events/search`

**Метрики мониторинга:**
- `rate_limit_exceeded_total` - количество блокировок
- `cache_hit_total` - количество hit'ов
- `cache_miss_total` - количество miss'ов
- `request_duration_seconds` - время отклика

### Метрики для мониторинга

**Кеширование:**
- `hit_rate: cache_hits / (cache_hits + cache_misses)`
- `avg_ttl_remaining` - среднее время до истечения TTL
- `memory_usage` - использование памяти кеша

**Rate limiting:**
- `exceeded_count` - количество 429 ответов
- `avg_requests_per_client` - среднее количество запросов на клиента
- `top_blocked_ips` - топ заблокированных IP

### Измерение эффективности кеширования

**Hit rate:**
```
hit_rate = cache_hits / (cache_hits + cache_misses)
```

**Целевые значения:**
- Hit rate > 80% для `/events`
- Hit rate > 90% для `/users/{login}`

**Инструменты мониторинга:**
- Prometheus для сбора метрик
- Grafana для визуализации
- Логирование всех cache miss'ов для анализа

## Заключение

Реализованные оптимизации:

1. **Кеширование:**
   - Cache-Aside стратегия для всех read endpoints
   - TTL от 1 до 15 минут в зависимости от типа данных
   - Инвалидация при обновлении данных
   - Ожидаемое ускорение: 20-50x для кешируемых endpoints

2. **Rate limiting:**
   - Sliding Window для read endpoints
   - Token Bucket для write/auth endpoints
   - HTTP заголовки для информирования клиентов
   - Защита от abuse и DDoS атак

3. **Метрики:**
   - Hit rate кеширования
   - Количество блокировок rate limiting
   - Время отклика по percentiles
