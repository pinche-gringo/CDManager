## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CDManager

CDManager_SOURCES=SongList.cpp SongList.h RecordList.cpp RecordList.h	\
		  Record.h RecEdit.cpp RecEdit.h DB.cpp DB.h Song.h	\
		  CDManager.cpp CDManager.h Interpret.cpp Interpret.h

INCLUDES = @INCLUDES@ -I../Common @DEPS_CFLAGS@

CDManager_LDFLAGS=@DEPS_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall
