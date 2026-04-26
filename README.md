# ЛАБОРАТОРНАЯ РАБОТА №4 - ВАРИАНТ 22
# Event Manager API - MongoDB

REST API на C++ userver + MongoDB для управления событиями.

## Запуск

```bash
docker-compose up --build
```

API: `http://localhost:8080`
MongoDB: `localhost:27017`

## Инициализация данных

```bash
docker exec -i event-manager-mongo-mongo-1 mongosh eventdb < data.js
```

## Валидация схем

```bash
docker exec -i event-manager-mongo-mongo-1 mongosh < validation.js
```

## Тестовые запросы

```bash
docker exec -i event-manager-mongo-mongo-1 mongosh eventdb < queries.js
```

## API Endpoints

| Метод | URL | Описание | Auth |
|-------|-----|----------|------|
| POST | /users | Создать пользователя | нет |
| POST | /auth/login | Войти | нет |
| POST | /auth/logout | Выйти | да |
| GET | /users/{login} | Найти по логину | да |
| GET | /users/search?first_name=&last_name= | Поиск по имени | да |
| POST | /events | Создать событие | да |
| GET | /events | Список событий | да |
| GET | /events/search?date_from=&date_to= | Поиск по дате | да |
| POST | /events/{id}/register | Регистрация на событие | да |
| GET | /events/{id}/participants | Участники события | да |
| GET | /users/me/events | События пользователя | да |
| DELETE | /events/{id}/unregister | Отмена регистрации | да |

## Примеры

```bash
curl -X POST http://localhost:8080/users \
  -H "Content-Type: application/json" \
  -d '{"login":"test","password":"pass","first_name":"Test","last_name":"User","email":"test@example.com"}'

curl -X POST http://localhost:8080/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"test","password":"pass"}'

TOKEN="<token>"

curl -X POST http://localhost:8080/events \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"title":"My Event","date":"2026-12-25","location":"Moscow"}'

curl http://localhost:8080/events \
  -H "Authorization: Bearer $TOKEN"

curl "http://localhost:8080/events/search?date_from=2026-06-01&date_to=2026-12-31" \
  -H "Authorization: Bearer $TOKEN"

curl -X POST http://localhost:8080/events/<event_id>/register \
  -H "Authorization: Bearer $TOKEN"

curl http://localhost:8080/events/<event_id>/participants \
  -H "Authorization: Bearer $TOKEN"

curl http://localhost:8080/users/me/events \
  -H "Authorization: Bearer $TOKEN"

curl -X DELETE http://localhost:8080/events/<event_id>/unregister \
  -H "Authorization: Bearer $TOKEN"
```

## Документация

- `schema_design.md` - проектирование модели данных
- `data.js` - тестовые данные
- `queries.js` - примеры MongoDB запросов
- `validation.js` - валидация схем
