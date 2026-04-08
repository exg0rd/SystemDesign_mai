-- schema.sql: Event Manager DB schema

CREATE TABLE IF NOT EXISTS users (
    id            SERIAL PRIMARY KEY,
    login         VARCHAR(64)  UNIQUE NOT NULL,
    password_hash VARCHAR(128) NOT NULL,
    first_name    VARCHAR(64)  NOT NULL,
    last_name     VARCHAR(64)  NOT NULL,
    email         VARCHAR(128) UNIQUE NOT NULL,
    created_at    TIMESTAMPTZ  NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS events (
    id           SERIAL PRIMARY KEY,
    title        VARCHAR(256) NOT NULL,
    description  TEXT,
    event_date   DATE         NOT NULL,
    location     VARCHAR(256),
    organizer_id INTEGER      NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at   TIMESTAMPTZ  NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS participants (
    id         SERIAL PRIMARY KEY,
    event_id   INTEGER NOT NULL REFERENCES events(id) ON DELETE CASCADE,
    user_id    INTEGER NOT NULL REFERENCES users(id)  ON DELETE CASCADE,
    registered_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_event_user UNIQUE (event_id, user_id)
);

CREATE TABLE IF NOT EXISTS sessions (
    token      VARCHAR(64) PRIMARY KEY,
    user_id    INTEGER     NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Indexes for foreign keys and frequent WHERE/JOIN columns
CREATE INDEX IF NOT EXISTS idx_events_organizer    ON events(organizer_id);
CREATE INDEX IF NOT EXISTS idx_events_date         ON events(event_date);
CREATE INDEX IF NOT EXISTS idx_participants_event  ON participants(event_id);
CREATE INDEX IF NOT EXISTS idx_participants_user   ON participants(user_id);
CREATE INDEX IF NOT EXISTS idx_sessions_user       ON sessions(user_id);
CREATE INDEX IF NOT EXISTS idx_users_login         ON users(login);
CREATE INDEX IF NOT EXISTS idx_users_name          ON users(first_name, last_name);
