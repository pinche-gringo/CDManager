## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CDManager

CDManager_SOURCES=MovieList.cpp MovieList.h RendererList.cpp RendererList.h \
		  SongList.cpp SongList.h RecordList.cpp RecordList.h Record.h\
		  DB.cpp DB.h Song.h Celibrity.cpp Celibrity.h Interpret.h \
		  CDManager.cpp CDManager.h

INCLUDES = @INCLUDES@ -I../Common @DEPS_CFLAGS@

CDManager_LDFLAGS=@DEPS_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall
