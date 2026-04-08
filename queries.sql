-- queries.sql: All API operations for Event Manager (Variant 22)

-- 1. Create user (done via INSERT in API, example):
INSERT INTO users (login, password_hash, first_name, last_name, email)
VALUES ('newuser', encode(sha256('mypassword'::bytea), 'hex'), 'New', 'User', 'new@example.com');

-- 2. Find user by login
SELECT id, login, first_name, last_name, email
FROM users
WHERE login = 'alice';

-- 3. Find users by first_name and last_name mask
SELECT id, login, first_name, last_name, email
FROM users
WHERE first_name ILIKE '%ali%'
  AND last_name  ILIKE '%smi%';

-- 4. Create event (done via INSERT in API, example):
INSERT INTO events (title, description, event_date, location, organizer_id)
VALUES ('New Event', 'Description', '2026-08-01', 'Moscow', 1);

-- 5. Get list of all events
SELECT id, title, description, event_date, location, organizer_id
FROM events
ORDER BY event_date;

-- 6. Search events by date
SELECT id, title, description, event_date, location, organizer_id
FROM events
WHERE event_date = '2026-06-01';

-- 7. Register user for event
INSERT INTO participants (event_id, user_id)
VALUES (1, 10);

-- 8. Get participants of an event (with user details)
SELECT u.id, u.login, u.first_name, u.last_name, u.email, p.registered_at
FROM participants p
JOIN users u ON u.id = p.user_id
WHERE p.event_id = 1;

-- 9. Get events of a user (events where user is registered or organizer)
SELECT DISTINCT e.id, e.title, e.description, e.event_date, e.location, e.organizer_id
FROM events e
LEFT JOIN participants p ON p.event_id = e.id
WHERE p.user_id = 1 OR e.organizer_id = 1
ORDER BY e.event_date;

-- 10. Cancel registration (unregister user from event)
DELETE FROM participants
WHERE event_id = 1 AND user_id = 10;

-- 11. Login: find user by login and password hash
SELECT id, login FROM users
WHERE login = 'alice'
  AND password_hash = encode(sha256('password123'::bytea), 'hex');

-- 12. Create session token
INSERT INTO sessions (token, user_id)
VALUES ('some-random-token-value', 1);

-- 13. Validate session token
SELECT user_id FROM sessions WHERE token = 'some-random-token-value';

-- 14. Delete session (logout)
DELETE FROM sessions WHERE token = 'some-random-token-value';
