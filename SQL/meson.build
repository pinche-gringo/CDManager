# Helper targets to set up the database; e.g.: meson compile -C build createDB
sql_dir = meson.current_source_dir()

if db_backend == 'postgresql'
    run_target('createDB',
               command: ['sh', '-c', 'echo Creating database ... && createdb -E UTF8 cdmedia && psql -d cdmedia -f "$0"',
                         sql_dir / 'createDB-pg.sql'])
    run_target('populateDB',
               command: ['sh', '-c', 'echo Populating database ... && sed "/^USE /d" "$0" | PGCLIENTENCODING=LATIN1 psql -U cdmgr -d cdmedia',
                         sql_dir / 'populateDB.sql'])
    run_target('syncSequences',
               command: ['sh', '-c', 'echo Adapting sequences ... && psql -U cdmgr -d cdmedia -f "$0"',
                         sql_dir / 'syncSequences-pg.sql'])
else
    run_target('createDB',
               command: ['sh', '-c', 'echo Creating database ... && mysql -u root -p -f < "$0"', sql_dir / 'createDB.sql'])
    run_target('populateDB',
               command: ['sh', '-c', 'echo Populating database ... && mysql -u cdmgr -p < "$0"', sql_dir / 'populateDB.sql'])
endif

# Upgrades of old (MySQL) databases
run_target('updateDB_Subtitles',
           command: ['sh', '-c', 'echo "ALTER TABLE Movies MODIFY subtitles VARCHAR(29) NOT NULL;" | mysql -u root -p CDMedia'])
run_target('addIMDbInfo',
           command: ['sh', '-c', 'echo "ALTER TABLE Movies ADD (summary VARCHAR(512) NOT NULL, image VARCHAR(8192) NOT NULL);" | mysql -u root -p CDMedia'])
run_target('renameToFilm',
           command: ['sh', '-c', 'echo "ALTER TABLE Movies RENAME Films, DROP KEY movieNames, ADD KEY filmNames (name);" '
                                 + '"ALTER TABLE MovieNames RENAME FilmNames, DROP KEY movieTranlations, ADD KEY filmTranslations (id), MODIFY id BIGINT UNSIGNED NOT NULL;" '
                                 + '"ALTER TABLE ActorsInMovies RENAME ActorsInFilms, CHANGE idMovie idFilm BIGINT UNSIGNED NOT NULL;" | mysql -u dbadmin -p CDMedia'])
run_target('upgrade10_11',
           command: ['sh', '-c', 'echo "ALTER TABLE Movies ADD (summary VARCHAR(512) NOT NULL, image VARCHAR(8192) NOT NULL);" | mysql -u root -p CDMedia && '
                                 + 'echo "ALTER TABLE Movies RENAME Films, DROP KEY movieNames, ADD KEY filmNames (name);" '
                                 + '"ALTER TABLE MovieNames RENAME FilmNames, DROP KEY movieTranlations, ADD KEY filmTranslations (id), MODIFY id BIGINT UNSIGNED NOT NULL;" '
                                 + '"ALTER TABLE ActorsInMovies RENAME ActorsInFilms, CHANGE idMovie idFilm BIGINT UNSIGNED NOT NULL;" | mysql -u dbadmin -p CDMedia'])
