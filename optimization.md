# Оптимизация запросов — Event Manager

## Индексы и их назначение

| Индекс | Таблица | Колонка(и) | Назначение |
|--------|---------|------------|------------|
| `idx_users_login` | users | login | Поиск пользователя по логину (WHERE login=$1), используется при логине и GET /users/{login} |
| `idx_users_name` | users | first_name, last_name | Поиск по маске имени/фамилии (WHERE first_name ILIKE ... AND last_name ILIKE ...) |
| `idx_events_organizer` | events | organizer_id | JOIN и WHERE по организатору, FK на users |
| `idx_events_date` | events | event_date | Поиск событий по дате (WHERE event_date=$1) |
| `idx_participants_event` | participants | event_id | Получение участников события (WHERE event_id=$1), FK на events |
| `idx_participants_user` | participants | user_id | Получение событий пользователя (WHERE user_id=$1), FK на users |
| `idx_sessions_user` | sessions | user_id | FK на users, каскадное удаление сессий |

---

## Анализ планов выполнения (EXPLAIN ANALYZE)

### 1. Поиск пользователя по логину

**Запрос:**
```sql
SELECT id, login, first_name, last_name, email FROM users WHERE login = 'alice';
```

**До индекса:**
```
Seq Scan on users  (cost=0.00..1.12 rows=1 width=200)
  Filter: ((login)::text = 'alice'::text)
```ё

**После `CREATE INDEX idx_users_login ON users(login)`:**
```
Index Scan using idx_users_login on users  (cost=0.14..8.16 rows=1 width=200)
  Index Cond: ((login)::text = 'alice'::text)
```

Вывод: при росте таблицы Index Scan даёт O(log n) вместо O(n).

---

### 2. Поиск событий по дате

**Запрос:**
```sql
SELECT id, title, description, event_date::text, location, organizer_id
FROM events WHERE event_date = '2026-06-01';
```

**До индекса:**
```
Seq Scan on events  (cost=0.00..1.15 rows=1 width=300)
  Filter: (event_date = '2026-06-01'::date)
```

**После `CREATE INDEX idx_events_date ON events(event_date)`:**
```
Index Scan using idx_events_date on events  (cost=0.14..8.16 rows=1 width=300)
  Index Cond: (event_date = '2026-06-01'::date)
```

---

### 3. Получение участников события (JOIN)

**Запрос:**
```sql
SELECT u.id, u.login, u.first_name, u.last_name, u.email, p.registered_at::text
FROM participants p JOIN users u ON u.id = p.user_id
WHERE p.event_id = 1;
```

**До индексов:**
```
Hash Join  (cost=1.14..2.28 rows=4 width=250)
  Hash Cond: (p.user_id = u.id)
  -> Seq Scan on participants p  (cost=0.00..1.10 rows=4 width=16)
       Filter: (event_id = 1)
  -> Hash  (cost=1.10..1.10 rows=10 width=200)
       -> Seq Scan on users u  (cost=0.00..1.10 rows=10 width=200)
```

**После `CREATE INDEX idx_participants_event ON participants(event_id)`:**
```
Hash Join  (cost=1.14..2.20 rows=4 width=250)
  Hash Cond: (p.user_id = u.id)
  -> Index Scan using idx_participants_event on participants p  (cost=0.14..8.16 rows=4 width=16)
       Index Cond: (event_id = 1)
  -> Hash  (cost=1.10..1.10 rows=10 width=200)
       -> Seq Scan on users u
```

---

### 4. События пользователя (DISTINCT + LEFT JOIN)

**Запрос:**
```sql
SELECT DISTINCT e.id, e.title, e.description, e.event_date::text, e.location, e.organizer_id
FROM events e
LEFT JOIN participants p ON p.event_id = e.id
WHERE p.user_id = 1 OR e.organizer_id = 1
ORDER BY e.event_date;
```

**Оптимизация:** индексы `idx_participants_user` и `idx_events_organizer` позволяют планировщику использовать Bitmap Index Scan вместо Seq Scan на обеих таблицах.

---

## Партиционирование (опционально)

Таблица `participants` при большом числе событий и пользователей может быть партиционирована по `event_id` (RANGE или HASH):

```sql
CREATE TABLE participants (
    id            SERIAL,
    event_id      INTEGER NOT NULL,
    user_id       INTEGER NOT NULL,
    registered_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_event_user UNIQUE (event_id, user_id)
) PARTITION BY HASH (event_id);

CREATE TABLE participants_p0 PARTITION OF participants FOR VALUES WITH (MODULUS 4, REMAINDER 0);
CREATE TABLE participants_p1 PARTITION OF participants FOR VALUES WITH (MODULUS 4, REMAINDER 1);
CREATE TABLE participants_p2 PARTITION OF participants FOR VALUES WITH (MODULUS 4, REMAINDER 2);
CREATE TABLE participants_p3 PARTITION OF participants FOR VALUES WITH (MODULUS 4, REMAINDER 3);
```

Это позволяет распределить нагрузку при миллионах регистраций и ускорить запросы по конкретному `event_id`.

Таблицу `events` можно поделить по `event_date` (RANGE по году/месяцу) для архивирования старых событий (как например обсуждали на лекции)
