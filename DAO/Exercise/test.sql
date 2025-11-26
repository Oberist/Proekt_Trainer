CREATE TABLE IF NOT EXISTS Exercise (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    difficulty INTEGER CHECK(difficulty BETWEEN 1 AND 3),
    muscle_group_primary TEXT,
    muscle_group_secondary TEXT,
    type TEXT,
    equipment TEXT,
    image_path TEXT
);