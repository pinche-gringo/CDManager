## Process this file with automake to produce Makefile.in

EXTRA_DIST=createDB.sql populateDB.sql

createDB:
	@echo Creating database ...
	mysql -u root -p -f < createDB.sql

populateDB:
	@echo Populating database ...
	mysql -u cdmgr -p < populateDB.sql


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