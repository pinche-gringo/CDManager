## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CDManager CDWriter

CDManager_SOURCES=Settings.cpp Settings.h CDAppl.cpp CDAppl.h		\
		  LangDlg.cpp LangDlg.h CDType.cpp CDType.h		\
		  Language.cpp Language.h Words.cpp Words.h OOList.cpp	\
		  OOList.h MovieList.cpp MovieList.h RecordList.cpp	\
		  RecordList.h RendererList.cpp RendererList.h		\
		  SongList.cpp SongList.h DB.cpp DB.h Song.h		\
		  Celebrity.cpp Celebrity.h Interpret.h Record.cpp	\
		  Record.h Movie.cpp Movie.h CDManager.cpp CDManager.h	\
		  Options.h

INCLUDES = @INCLUDES@ -I../Common @DEPS_CFLAGS@

CDManager_LDFLAGS=@DEPS_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall

CDWriter_SOURCES=CDWriter.cpp CDWriter.h Options.h Writer.cpp Writer.h	 \
		 Movie.cpp Movie.h Celebrity.cpp Celebrity.h Interpret.h \
		 CDType.cpp CDType.h Language.cpp Language.h Words.cpp	 \
		 Words.h Record.cpp Record.h DB.cpp DB.h

CDWriter_LDFLAGS=@DEPS_LIBS@ @LTLIBINTL@

Options.meta: Options.h
	mgeni.pl Options.h > Options.meta

Celebrity.meta: Celebrity.h
	mgeni.pl Celebrity.h > Celebrity.meta

Movie.meta: Movie.h
	mgeni.pl Movie.h > Movie.meta

Record.meta: Record.h
	mgeni.pl Record.h > Record.meta

EXTRA_DIST=Options.meta Celebrity.meta Movie.meta Record.meta