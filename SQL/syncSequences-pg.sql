-- Adapts the sequences of the identity columns in a PostgreSQL database to
-- the stored entries, so the next generated id follows the highest one.
--
-- Needed after loading entries with explicit ids (e.g. migrated from MySQL),
-- as this doesn't advance the sequences (and so inserts fail with unique key
-- violations). Run it connected to the database, e.g.:
--   psql -d cdmedia -f syncSequences-pg.sql

SELECT setval(pg_get_serial_sequence('celebrities', 'id'), COALESCE(MAX(id), 0) + 1, false) FROM Celebrities;
SELECT setval(pg_get_serial_sequence('films', 'id'), COALESCE(MAX(id), 0) + 1, false) FROM Films;
SELECT setval(pg_get_serial_sequence('records', 'id'), COALESCE(MAX(id), 0) + 1, false) FROM Records;
SELECT setval(pg_get_serial_sequence('songs', 'id'), COALESCE(MAX(id), 0) + 1, false) FROM Songs;
