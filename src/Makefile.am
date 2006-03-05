## Process this file with automake to produce Makefile.in

SRC_MOVIE=PMovies.cpp StorageMovie.cpp StorageMovie.h PMovies.h \
MovieList.cpp MovieList.h
SRC_GENMOVIE=Director.h Movie.cpp Movie.h
if WITH_MOVIES
   USE_MOVIE=$(SRC_MOVIE)
   USE_GENMOVIE=$(SRC_GENMOVIE)
endif

SRC_RECORD=PRecords.cpp StorageRecord.cpp StorageRecord.h PRecords.h \
RecordList.cpp RecordList.h SongList.cpp SongList.h
SRC_GENREC=Song.h Record.cpp Record.h Interpret.h
if WITH_RECORDS
   USE_RECORD=$(SRC_RECORD)
   USE_GENREC=$(SRC_GENREC)
endif

SRC_ACTOR=PActors.cpp StorageActor.cpp StorageActor.h PActors.h \
ActorList.cpp ActorList.h RelateMovie.h RelateMovie.cpp
SRC_GENACTOR=Actor.h
if WITH_ACTORS
   USE_ACTOR=$(SRC_ACTOR)
   USE_GENACTOR=$(SRC_GENACTOR)
endif

lib_LTLIBRARIES=libCDMgr.la

bin_PROGRAMS=CDManager CDWriter

AM_CPPFLAGS = @INCLUDES@ -I../Common @DEPS_CFLAGS@ -Wall

libCDMgr_la_SOURCES=Options.h Celebrity.cpp Celebrity.h CDType.cpp	\
CDType.h Words.cpp Words.h Genres.cpp Genres.h $(USE_GENMOVIE)		\
$(USE_GENREC) $(USE_GENACTOR)

libCDMgr_la_LIBADD=@DEPS_LIBS@

CDManager_SOURCES=NBPage.cpp Storage.cpp NBPage.h Storage.h		\
Settings.cpp Settings.h CDAppl.cpp CDAppl.h LangDlg.cpp LangDlg.h	\
OOList.cpp OOList.h LangImg.cpp LangImg.h WordDlg.cpp WordDlg.h		\
Language.cpp Language.h DB.cpp DB.h CDManager.h CDManager.cpp		\
$(USE_MOVIE) $(USE_RECORD) $(USE_ACTOR)

CDManager_CPPFLAGS=@INCLUDES@ -I../Common @DEPS_CFLAGS@ @XGP_CFLAGS@ -DUSE_LANGUAGEPIXMAPS -Wall
CDManager_LDFLAGS=-lCDMgr @DEPS_LIBS@ @XGP_LIBS@ @LTLIBINTL@ @MYSQLLIBS@

CDWriter_SOURCES=CDWriter.cpp CDWriter.h Writer.cpp Writer.h \
		 Language.cpp Language.h
CDWriter_LDFLAGS=-lCDMgr @DEPS_LIBS@ @LTLIBINTL@

AllMetaInfo: Options.meta Celebrity.meta Movie.meta Record.meta

Options.meta: Options.h
	mgeni.pl Options.h > Options.meta

Celebrity.meta: Celebrity.h
	mgeni.pl Celebrity.h > Celebrity.meta

Movie.meta: Movie.h
	mgeni.pl Movie.h > Movie.meta

Record.meta: Record.h
	mgeni.pl Record.h > Record.meta

EXTRA_DIST=Options.meta Celebrity.meta Movie.meta Record.meta $(SRC_MOVIE) \
$(SRC_GENMOVIE) $(SRC_RECORD) $(SRC_GENREC) $(SRC_ACTOR) $(SRC_GENACTOR)
