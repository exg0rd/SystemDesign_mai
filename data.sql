-- data.sql: Test data for Event Manager
-- Спасибо ии за генерацию тестовых данных 
-- password_hash is sha256('password123') for all test users
INSERT INTO users (login, password_hash, first_name, last_name, email) VALUES
('alice',   encode(sha256('password123'::bytea), 'hex'), 'Alice',   'Smith',    'alice@example.com'),
('bob',     encode(sha256('password123'::bytea), 'hex'), 'Bob',     'Johnson',  'bob@example.com'),
('carol',   encode(sha256('password123'::bytea), 'hex'), 'Carol',   'Williams', 'carol@example.com'),
('dave',    encode(sha256('password123'::bytea), 'hex'), 'Dave',    'Brown',    'dave@example.com'),
('eve',     encode(sha256('password123'::bytea), 'hex'), 'Eve',     'Jones',    'eve@example.com'),
('frank',   encode(sha256('password123'::bytea), 'hex'), 'Frank',   'Garcia',   'frank@example.com'),
('grace',   encode(sha256('password123'::bytea), 'hex'), 'Grace',   'Martinez', 'grace@example.com'),
('henry',   encode(sha256('password123'::bytea), 'hex'), 'Henry',   'Davis',    'henry@example.com'),
('ivan',    encode(sha256('password123'::bytea), 'hex'), 'Ivan',    'Wilson',   'ivan@example.com'),
('julia',   encode(sha256('password123'::bytea), 'hex'), 'Julia',   'Anderson', 'julia@example.com'),
('kevin',   encode(sha256('password123'::bytea), 'hex'), 'Kevin',   'Taylor',   'kevin@example.com'),
('laura',   encode(sha256('password123'::bytea), 'hex'), 'Laura',   'Thomas',   'laura@example.com');

INSERT INTO events (title, description, event_date, location, organizer_id) VALUES
('Tech Conference 2026',    'Annual tech conference',          '2026-05-10', 'Moscow, Skolkovo',    1),
('Python Meetup',           'Python community meetup',         '2026-05-15', 'Moscow, Technopark',  2),
('C++ Workshop',            'Advanced C++ techniques',         '2026-05-20', 'Online',              1),
('AI Summit',               'Artificial Intelligence summit',  '2026-06-01', 'Saint Petersburg',    3),
('DevOps Days',             'DevOps practices and tools',      '2026-06-05', 'Moscow, Expo',        4),
('Startup Pitch Night',     'Startup presentations',           '2026-06-10', 'Moscow, Hub',         5),
('Data Science Forum',      'Data science and ML',             '2026-06-15', 'Online',              2),
('Cloud Native Day',        'Kubernetes and cloud',            '2026-06-20', 'Moscow, Digital',     6),
('Security Conference',     'Cybersecurity topics',            '2026-07-01', 'Moscow, Kremlin',     7),
('Open Source Fest',        'Open source projects showcase',   '2026-07-10', 'Novosibirsk',         8),
('Mobile Dev Summit',       'iOS and Android development',     '2026-07-15', 'Online',              9),
('Game Dev Expo',           'Game development showcase',       '2026-07-20', 'Moscow, VDNKh',       10);

INSERT INTO participants (event_id, user_id) VALUES
(1, 2), (1, 3), (1, 4), (1, 5),
(2, 1), (2, 3), (2, 6),
(3, 2), (3, 7), (3, 8),
(4, 1), (4, 2), (4, 9), (4, 10),
(5, 3), (5, 4), (5, 11),
(6, 1), (6, 5), (6, 12),
(7, 1), (7, 3), (7, 6), (7, 9),
(8, 2), (8, 4), (8, 7),
(9, 5), (9, 8), (9, 10),
(10, 1), (10, 6), (10, 11), (10, 12),
(11, 2), (11, 3), (11, 7),
(12, 4), (12, 8), (12, 9), (12, 10);
