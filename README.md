# ЛАБОРАТОРНАЯ РАБОТА №3 - ВАРИАНТ 22

REST API система управления событиями на C++ PostgreSQL и юсервер C++

## Схема БД

```
users          — пользователи (id, login, password_hash, first_name, last_name, email)
events         — события (id, title, description, event_date, location, organizer_id → users)
participants   — участники (id, event_id → events, user_id → users) [UNIQUE event_id+user_id]
sessions       — сессии/токены (token PK, user_id → users)
```

## API

| Метод | Путь | Описание |
|-------|------|----------|
| POST | /auth/login | Логин, возвращает Bearer token |
| POST | /auth/logout | Логаут |
| POST | /users | Создание пользователя |
| GET | /users/{login} | Поиск по логину |
| GET | /users/search?first_name=&last_name= | Поиск по маске имени |
| POST | /events | Создание события |
| GET | /events | Список всех событий |
| GET | /events/search?date=YYYY-MM-DD | Поиск событий по дате |
| POST | /events/{event_id}/participants | Регистрация на событие |
| GET | /events/{event_id}/participants | Участники события |
| GET | /users/{user_id}/events | События пользователя |
| DELETE | /events/{event_id}/participants | Отмена регистрации |

## Запуск по умолчанию делаем

```bash
docker-compose up --build
```

## Скрипты

```bash
chmod +x tests/test.sh
./tests/test.sh
```

PostgreSQL 5432, API 8080
Схема и тестовые данные применяются при первом запуске

## Файлы

- `schema.sql` — DDL: таблицы + индексы
- `data.sql` — тестовые данные (12 пользователей, 12 событий, 40+ регистраций)
- `queries.sql` — SQL для всех операций API
- `optimization.md` — анализ индексов и планов EXPLAIN
- `static_config.yaml` — конфигурация userver

## Переменые окружения

| Переменая | Значение по умолчанию |
|------------|----------------------|
| POSTGRES_DSN | postgresql://eventuser:eventpass@postgres:5432/eventdb |

## Примеры запросов

```bash
# Создание юзера
curl -X POST http://localhost:8080/users \
  -H 'Content-Type: application/json' \
  -d '{"login":"testuser","password":"secret","first_name":"Test","last_name":"User","email":"test@example.com"}'

# Логин
curl -X POST http://localhost:8080/auth/login \
  -H 'Content-Type: application/json' \
  -d '{"login":"testuser","password":"secret"}'
# → {"token":"..."}

# Новое событие
curl -X POST http://localhost:8080/events \
  -H 'Authorization: Bearer <token>' \
  -H 'Content-Type: application/json' \
  -d '{"title":"My Event","event_date":"2026-09-01","location":"Moscow"}'

# Регистрация
curl -X POST http://localhost:8080/events/1/participants \
  -H 'Authorization: Bearer <token>'
```
