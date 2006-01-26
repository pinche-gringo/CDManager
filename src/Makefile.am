## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES=libCDMgr.la

bin_PROGRAMS=CDManager CDWriter

AM_CPPFLAGS = @INCLUDES@ -I../Common @DEPS_CFLAGS@ -Wall

libCDMgr_la_SOURCES=Options.h Movie.cpp Movie.h Celebrity.cpp		\
Celebrity.h CDType.cpp CDType.h Words.cpp Words.h Record.cpp Record.h	\
Interpret.h Genres.cpp Genres.h PRecords.h PActors.h PMovies.h		\
NBPage.h Storage.h StorageActor.h StorageMovie.h StorageRecord.

libCDMgr_la_LIBADD=@DEPS_LIBS@

CDManager_SOURCES=NBPage.cpp PRecords.cpp PMovies.cpp PActors.cpp	\
Storage.cpp StorageRecord.cpp StorageMovie.cpp StorageActor.cpp		\
Settings.cpp Settings.h CDAppl.cpp CDAppl.h LangDlg.cpp LangDlg.h	\
OOList.cpp OOList.h MovieList.cpp MovieList.h RecordList.cpp		\
RecordList.h SongList.cpp SongList.h Song.h LangImg.cpp LangImg.h	\
WordDlg.cpp WordDlg.h Language.cpp Language.h DB.cpp DB.h Actor.h	\
ActorList.cpp ActorList.h RelateMovie.h RelateMovie.cpp CDManager.h	\
Director.h CDManager.cpp

CDManager_CPPFLAGS=@INCLUDES@ -I../Common @DEPS_CFLAGS@ @XGP_CFLAGS@ -DUSE_LANGUAGEPIXMAPS -Wall
CDManager_LDFLAGS=-lCDMgr @DEPS_LIBS@ @XGP_LIBS@ @LTLIBINTL@ @MYSQLLIBS@

CDWriter_SOURCES=CDWriter.cpp CDWriter.h Writer.cpp Writer.h \
		 Language.cpp Language.h
CDWriter_LDFLAGS=-lCDMgr @DEPS_LIBS@ @LTLIBINTL@

Options.meta: Options.h
	mgeni.pl Options.h > Options.meta

Celebrity.meta: Celebrity.h
	mgeni.pl Celebrity.h > Celebrity.meta

Movie.meta: Movie.h
	mgeni.pl Movie.h > Movie.meta

Record.meta: Record.h
	mgeni.pl Record.h > Record.meta

EXTRA_DIST=Options.meta Celebrity.meta Movie.meta Record.meta