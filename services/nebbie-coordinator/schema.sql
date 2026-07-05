CREATE TABLE IF NOT EXISTS reservations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    builder TEXT NOT NULL,
    zone_num INTEGER NOT NULL,
    kind TEXT NOT NULL DEFAULT 'room',
    start_vnum INTEGER NOT NULL,
    end_vnum INTEGER NOT NULL,
    note TEXT NOT NULL DEFAULT '',
    expires_at TEXT NOT NULL DEFAULT '',
    status TEXT NOT NULL DEFAULT 'active',
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_reservations_zone ON reservations(zone_num);
CREATE INDEX IF NOT EXISTS idx_reservations_status ON reservations(status);
