# Домашнее задание 02
# Вариант 22: Система управления событиями 

REST API сервис для управления событиями на C++ (Poco + SQLite).

## Запуск через Docker

```bash
docker-compose up --build
```

Сервер запускается на порту `8080`.

## Endpoints

| Метод | URL | Описание | Auth |
|-------|-----|----------|------|
| POST | /users | Создать пользователя | нет |
| POST | /auth/login | Войти, получить токен | нет |
| POST | /auth/logout | Выйти | да |
| GET | /users/{login} | Найти пользователя по логину | да |
| GET | /users/search?first_name=&last_name= | Поиск по маске имени/фамилии | да |
| POST | /events | Создать событие | да |
| GET | /events | Список событий | да |


## Тесты

```bash
bash tests/test.sh
```

## Документация API

Файл `openapi.yaml` 
