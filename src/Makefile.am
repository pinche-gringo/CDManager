## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES=libCDMgr.la

bin_PROGRAMS=CDManager CDWriter

libCDMgr_la_SOURCES=Options.h Movie.cpp Movie.h Celebrity.cpp		\
		    Celebrity.h CDType.cpp CDType.h Language.cpp	\
		    Language.h Words.cpp Words.h Record.cpp Record.h	\
		    DB.cpp DB.h Interpret.h Genres.cpp Genres.h

CDManager_SOURCES=Settings.cpp Settings.h CDAppl.cpp CDAppl.h		\
		  LangDlg.cpp LangDlg.h OOList.cpp OOList.h		\
		  MovieList.cpp MovieList.h RecordList.cpp		\
		  RecordList.h RendererList.cpp RendererList.h		\
		  SongList.cpp SongList.h Song.h CDManagerDB.cpp	\
		  CDManager.h CDManagerDel.cpp CDManager.cpp

INCLUDES = @INCLUDES@ -I../Common @DEPS_CFLAGS@

CDManager_LDFLAGS=-lCDMgr @DEPS_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall

CDWriter_SOURCES=CDWriter.cpp CDWriter.h Writer.cpp Writer.h

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