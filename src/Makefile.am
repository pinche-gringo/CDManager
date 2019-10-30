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

lib_LTLIBRARIES=libCDMgr.la

bin_PROGRAMS=CDManager CDWriter

AM_CPPFLAGS = @INCLUDES@ -I../Common @DEPS_CFLAGS@ -Wall

libCDMgr_la_SOURCES=Options.h Celebrity.cpp Celebrity.h CDType.cpp	\
CDType.h Words.cpp Words.h Genres.cpp Genres.h $(USE_GENFILM)		\
$(USE_GENREC) $(USE_GENACTOR)

libCDMgr_la_LIBADD=@DEPS_LIBS@

CDManager_SOURCES=NBPage.cpp Storage.cpp NBPage.h Storage.h		\
SaveCeleb.cpp SaveCeleb.h Settings.cpp Settings.h LangDlg.cpp		\
LangDlg.h OOList.cpp OOList.h LangImg.cpp LangImg.h WordDlg.cpp		\
WordDlg.h Language.cpp Language.h DB.cpp DB.h Statistics.cpp		\
Statistics.h $(USE_FILM) $(USE_RECORD) $(USE_ACTOR) CDManager.h	\
CDManager.cpp CDAppl.cpp CDAppl.h IconAuthor.h IconProgram.h

CDManager_CPPFLAGS=@INCLUDES@ -I../Common @DEPS_CFLAGS@ @XGP_CFLAGS@ -DUSE_LANGUAGEPIXMAPS -Wall
CDManager_LDFLAGS=-lCDMgr @DEPS_LIBS@ @XGP_LIBS@ @LTLIBINTL@ @MYSQLLIBS@

CDWriter_SOURCES=CDWriter.cpp CDWriter.h Writer.cpp Writer.h \
		 Language.cpp Language.h
CDWriter_LDFLAGS=-lCDMgr @DEPS_LIBS@ @LTLIBINTL@

AllMetaInfo: Options.meta Celebrity.meta Film.meta Record.meta

Options.meta: Options.h
	mgeni.pl Options.h > Options.meta

Celebrity.meta: Celebrity.h
	mgeni.pl Celebrity.h > Celebrity.meta

Film.meta: Film.h
	mgeni.pl Film.h > Film.meta

Record.meta: Record.h
	mgeni.pl Record.h > Record.meta

EXTRA_DIST=Options.meta Celebrity.meta Film.meta Record.meta $(SRC_FILM) \
$(SRC_GENFILM) $(SRC_RECORD) $(SRC_GENREC) $(SRC_ACTOR) $(SRC_GENACTOR)
