-- Таблица пользователей
CREATE TABLE IF NOT EXISTS Users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    weight REAL,
    height REAL
);

-- Таблица упражнений
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

-- Таблица тренировочных дней
CREATE TABLE IF NOT EXISTS TrainingDay (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT,                     -
    date DATE NOT NULL             
);

-- Таблица программ тренировок
CREATE TABLE IF NOT EXISTS Program (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT
);
    
-- Промежуточная таблица для связи программы и тренировочных дней
CREATE TABLE IF NOT EXISTS ProgramTrainingDay (
    program_id INTEGER NOT NULL,
    training_day_id INTEGER NOT NULL,
    PRIMARY KEY (program_id, training_day_id),
    FOREIGN KEY (program_id) REFERENCES Program(id),
    FOREIGN KEY (training_day_id) REFERENCES TrainingDay(id)
);

-- Таблица записей о выполнении упражнений
CREATE TABLE IF NOT EXISTS ExercisesEntry (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    training_day_id INTEGER NOT NULL,
    exercise_id INTEGER NOT NULL,
    sets INTEGER CHECK(sets > 0),
    reps INTEGER CHECK(reps > 0),
    weight REAL,
    rest_time_sec INTEGER CHECK(rest_time_sec >= 0), 
    note TEXT,
    FOREIGN KEY(training_day_id) REFERENCES TrainingDay(id),
    FOREIGN KEY(exercise_id) REFERENCES Exercise(id)
);

-- Таблица календаря для назначения тренировок
CREATE TABLE IF NOT EXISTS Calendar (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    training_day_id INTEGER NOT NULL,
    scheduled_date DATE NOT NULL,
    status TEXT CHECK(status IN ('planned','completed','missed')) DEFAULT 'planned',
    program_id INTEGER,
    FOREIGN KEY(user_id) REFERENCES Users(id),
    FOREIGN KEY(training_day_id) REFERENCES TrainingDay(id),
    FOREIGN KEY(program_id) REFERENCES Program(id)
);

-- Таблица результатов тренировок
CREATE TABLE IF NOT EXISTS Workout (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    training_day_id INTEGER NOT NULL,
    calendar_id INTEGER,
    date DATE NOT NULL,
    total_volume INTEGER,
    duration INTEGER,
    FOREIGN KEY(user_id) REFERENCES Users(id),
    FOREIGN KEY(training_day_id) REFERENCES TrainingDay(id),
    FOREIGN KEY(calendar_id) REFERENCES Calendar(id)
);

-- Таблица прогресса пользователя
CREATE TABLE IF NOT EXISTS Progress (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    date DATE NOT NULL,
    weight REAL,
    total_volume INTEGER,
    calendar_id INTEGER,
    FOREIGN KEY(user_id) REFERENCES Users(id),
    FOREIGN KEY(calendar_id) REFERENCES Calendar(id)
);