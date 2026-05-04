# ЛАБОРАТОРНАЯ РАБОТА №5 - ВАРИАНТ 22
# Event Manager API - MongoDB с кешированием и rate limiting

REST API на C++ userver + MongoDB для управления событиями с оптимизацией производительности.

## Оптимизации

### Кеширование
- Cache-Aside стратегия для всех read endpoints
- TTL от 1 до 15 минут в зависимости от типа данных
- Инвалидация при обновлении данных
- Ожидаемое ускорение: 20-50x для кешируемых endpoints

### Rate limiting
- Sliding Window для read endpoints
- Token Bucket для write/auth endpoints
- HTTP заголовки для информирования клиентов
- Защита от abuse и DDoS атак

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

| Метод | URL | Описание | Auth | Rate Limit |
|-------|-----|----------|------|------------|
| POST | /users | Создать пользователя | нет | 10 req/min |
| POST | /auth/login | Войти | нет | 5 req/min |
| POST | /auth/logout | Выйти | да | 10 req/min |
| GET | /users/{login} | Найти по логину | да | 100 req/min |
| GET | /users/search?first_name=&last_name= | Поиск по имени | да | 100 req/min |
| POST | /events | Создать событие | да | 20 req/min |
| GET | /events | Список событий | да | 100 req/min |
| GET | /events/search?date_from=&date_to= | Поиск по дате | да | 30 req/min |
| POST | /events/{id}/register | Регистрация на событие | да | 20 req/min |
| GET | /events/{id}/participants | Участники события | да | 100 req/min |
| GET | /users/me/events | События пользователя | да | 100 req/min |
| DELETE | /events/{id}/unregister | Отмена регистрации | да | 20 req/min |

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

- `performance_design.md` - описание стратегии кеширования и rate limiting
- `schema_design.md` - проектирование модели данных
- `data.js` - тестовые данные
- `queries.js` - примеры MongoDB запросов
- `validation.js` - валидация схем
