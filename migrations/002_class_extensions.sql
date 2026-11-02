-- CR1: additive migration from schema version 1 to version 2.
CREATE TABLE class_extensions (
 assignment_id INTEGER PRIMARY KEY REFERENCES assignments(id),
 deadline_utc INTEGER NOT NULL,
 granted_by INTEGER NOT NULL REFERENCES users(id),
 granted_at INTEGER NOT NULL,
 reason TEXT NOT NULL CHECK(length(trim(reason)) BETWEEN 1 AND 1000)
);
