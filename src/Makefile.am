## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CDManager

CDManager_SOURCES=Record.h RecEdit.cpp RecEdit.h DB.cpp DB.h CDManager.cpp\
		  CDManager.h

INCLUDES = @INCLUDES@ -I../Common @DEPS_CFLAGS@

CDManager_LDFLAGS=@DEPS_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall
