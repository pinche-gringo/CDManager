## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CDManager

CDManager_SOURCES=Words.cpp Words.h Writer.cpp Writer.h OOList.cpp OOList.h \
		  MovieList.cpp MovieList.h RecordList.cpp RecordList.h \
		  RendererList.cpp RendererList.h SongList.cpp SongList.h \
		  DB.cpp DB.h Song.h Celebrity.cpp Celebrity.h Interpret.h \
		  Record.cpp Record.h Movie.cpp Movie.h CDManager.cpp \
		  CDManager.h Options.h

INCLUDES = @INCLUDES@ -I../Common @DEPS_CFLAGS@

CDManager_LDFLAGS=@DEPS_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall
