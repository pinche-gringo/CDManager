## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CDManager

CDManager_SOURCES=Writer.cpp Writer.h OOList.cpp OOList.h MovieList.cpp \
		  MovieList.h RecordList.cpp RecordList.h RendererList.cpp \
		  RendererList.h SongList.cpp SongList.h DB.cpp DB.h Song.h \
		  Celebrity.cpp Celebrity.h Interpret.h Record.h CDManager.cpp\
		  CDManager.h

INCLUDES = @INCLUDES@ -I../Common @DEPS_CFLAGS@

CDManager_LDFLAGS=@DEPS_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall
