## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CDManager

CDManager_SOURCES=CDManager.cpp CDManager.h DB.cpp DB.h

INCLUDES = @INCLUDES@ -I../Common @DEPS_CFLAGS@

CDManager_LDFLAGS=@DEPS_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall
