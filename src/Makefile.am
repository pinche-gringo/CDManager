## Process this file with automake to produce Makefile.in

SRC_FILM=FilmList.cpp FilmList.h StorageFilm.cpp StorageFilm.h	\
IMDbProgress.cpp IMDbProgress.h ImportIMDb.cpp ImportIMDb.h		\
PFilms.cpp PFilms.h FilmData.cpp FilmData.h
SRC_GENFILM=Director.h Film.cpp Film.h
if WITH_FILMS
   USE_FILM=$(SRC_FILM)
   USE_GENFILM=$(SRC_GENFILM)
endif

SRC_RECORD=RecordList.cpp RecordList.h SongList.cpp SongList.h	\
StorageRecord.cpp StorageRecord.h PRecords.h PRecords.cpp
SRC_GENREC=Song.h Record.cpp Record.h Interpret.h
if WITH_RECORDS
   USE_RECORD=$(SRC_RECORD)
   USE_GENREC=$(SRC_GENREC)
endif

SRC_ACTOR=StorageActor.cpp StorageActor.h ActorList.cpp ActorList.h	\
RelateFilm.h RelateFilm.cpp PActors.cpp PActors.h
SRC_GENACTOR=Actor.h
if WITH_ACTORS
   USE_ACTOR=$(SRC_ACTOR)
   USE_GENACTOR=$(SRC_GENACTOR)
endif

SRC_MYSQL=DBMySQL.cpp DBMySQL.h
SRC_POSTGRES=DBPostgres.cpp DBPostgres.h
if WITH_MYSQL
   USE_DB=$(SRC_MYSQL)
endif
if WITH_POSTGRESQL
   USE_DB=$(SRC_POSTGRES)
endif

lib_LTLIBRARIES=libCDMgr.la

AM_CXXFLAGS = -std=c++26 -freflection -fcontracts -fcontract-evaluation-semantic=@CONTRACT_SEMANTIC@

bin_PROGRAMS=CDManager CDWriter

AM_CPPFLAGS = @INCLUDES@ -I../Common @DEPS_CFLAGS@ -Wall

libCDMgr_la_SOURCES=Options.h EntityAttributes.h Celebrity.cpp Celebrity.h CDType.cpp	\
CDType.h Words.cpp Words.h Genres.cpp Genres.h $(USE_GENFILM)		\
$(USE_GENREC) $(USE_GENACTOR)

libCDMgr_la_LIBADD=@DEPS_LIBS@

CDManager_SOURCES=NBPage.cpp Storage.cpp NBPage.h Storage.h		\
SaveCeleb.cpp SaveCeleb.h Settings.cpp Settings.h LangDlg.cpp		\
LangDlg.h OOList.cpp OOList.h LangImg.cpp LangImg.h WordDlg.cpp		\
WordDlg.h Language.cpp Language.h DB.cpp DB.h $(USE_DB) Statistics.cpp		\
Statistics.h $(USE_FILM) $(USE_RECORD) $(USE_ACTOR) CDManager.h	\
CDManager.cpp CDAppl.cpp CDAppl.h IconAuthor.h IconProgram.h

CDManager_CPPFLAGS=@INCLUDES@ -I../Common @DEPS_CFLAGS@ @XGP_CFLAGS@ -DUSE_LANGUAGEPIXMAPS -Wall
CDManager_LDADD=libCDMgr.la @DEPS_LIBS@ @XGP_LIBS@ @LTLIBINTL@ @DBLIBS@

CDWriter_SOURCES=CDWriter.cpp CDWriter.h Writer.cpp Writer.h \
		 Language.cpp Language.h
CDWriter_LDADD=libCDMgr.la @DEPS_LIBS@ @LTLIBINTL@

EXTRA_DIST=IconAuthor.gdkp IconProgram.gdkp $(SRC_FILM) \
$(SRC_GENFILM) $(SRC_RECORD) $(SRC_GENREC) $(SRC_ACTOR) $(SRC_GENACTOR) \
$(SRC_MYSQL) $(SRC_POSTGRES)
