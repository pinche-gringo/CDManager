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
	echo "ALTER TABLE Movies MODIFY subtitles VARCHAR(29) NOT NULL;"|mysql -u root -p
