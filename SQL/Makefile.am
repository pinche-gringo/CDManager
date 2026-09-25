## Process this file with automake to produce Makefile.in

EXTRA_DIST=createDB.sql createDB-pg.sql populateDB.sql

if WITH_POSTGRESQL
createDB:
	@echo Creating database ...
	createdb -E UTF8 CDMedia
	psql -d CDMedia -f $(srcdir)/createDB-pg.sql

# populateDB.sql is Latin-1 encoded and starts with MySQL's USE-statement
populateDB:
	@echo Populating database ...
	sed '/^USE /d' $(srcdir)/populateDB.sql | PGCLIENTENCODING=LATIN1 psql -U cdmgr -d CDMedia
else
createDB:
	@echo Creating database ...
	mysql -u root -p -f < createDB.sql

populateDB:
	@echo Populating database ...
	mysql -u cdmgr -p < populateDB.sql
endif


updateDB_Subtitles:
	@echo Changing length of subtitle-column in Movie-table
	echo "ALTER TABLE Movies MODIFY subtitles VARCHAR(29) NOT NULL;"|mysql -u root -p CDMedia

addIMDbInfo:
	@echo Adding description columns to Movie-table
	echo "ALTER TABLE Movies ADD (summary VARCHAR(512) NOT NULL, image VARCHAR(8192) NOT NULL);"|mysql -u root -p CDMedia

renameToFilm:
	@echo Renaming Tables to *Film*
	echo "ALTER TABLE Movies RENAME Films, DROP KEY movieNames, ADD KEY filmNames (name);" \
	     "ALTER TABLE MovieNames RENAME FilmNames, DROP KEY movieTranlations, ADD KEY filmTranslations (id), MODIFY id BIGINT UNSIGNED NOT NULL;" \
	     "ALTER TABLE ActorsInMovies RENAME ActorsInFilms, CHANGE idMovie idFilm BIGINT UNSIGNED NOT NULL;"|mysql -u dbadmin -p CDMedia

upgrade10_11: addIMDbInfo renameToFilm