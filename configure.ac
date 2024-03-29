dnl $Id: configure.ac,v 1.32 2009/08/08 13:26:16 markus Exp $
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)
AC_REVISION($Revision: 1.32 $)
AC_INIT(CDManager, 1.2.00, [g17m0@users.sourceforge.net], CDManager)

RELEASE=${PACKAGE_VERSION%.*}
AC_DEFINE_UNQUOTED(PRG_RELEASE, "$RELEASE", [Define to the release of the program])
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(RELEASE)

AM_INIT_AUTOMAKE(dist-bzip2)

INCLUDES="-I. -I\$(top_srcdir) -DLOCALEDIR=\\\"\$(datadir)/locale\\\" -DDOCUDIR=\\\"\$(datadir)/doc/\$(PACKAGE)/\\\" -DDATADIR=\\\"\$(pkgdatadir)/\\\""

AC_C_BIGENDIAN

dnl Checks for programs.
AC_LANG_CPLUSPLUS
AM_CONFIG_HEADER(cdmgr-cfg.h)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CXX
AC_LANG_CPLUSPLUS
AM_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_GNU_SOURCE
AM_GNU_GETTEXT

ALL_LINGUAS="de en es"
AC_DEFINE_UNQUOTED(CATALOGS, "$ALL_LINGUAS", [Translations])

dnl Checks for header files.

AC_CHECK_HEADERS(boost/shared_ptr.hpp boost/tokenizer.hpp boost/bind.hpp boost/asio.hpp,,
	         AC_MSG_ERROR([Required headers not found]))

dnl Checks for libraries.
AC_CHECK_LIB(stdc++, main)
AC_CHECK_LIB(boost_system, main,, AC_MSG_ERROR([Required library boost_filesystem not found!
Either it can't be found in your default link-path (check the LIBS-environment
variable) or install it using your package-manager or from its website at
http://www.boost.org]))


PKG_CHECK_MODULES(DEPS, glibmm-2.4 >= 2.4.0 YGP >= 0.9.42)
PKG_CHECK_MODULES(XGP, gtkmm-3.0 >= 3.0.0 gthread-2.0 > 2.2.0 XGP >= 0.9.50)

#
# Location of MySQL installation
#
AC_ARG_WITH(mysql,
    AC_HELP_STRING([--with-mysql=DIR],
	           [search for MySQL in DIR/include and DIR/lib]),
    [MYSQL_lib_check=$withval/lib/mysql $with_mysql/lib
     MYSQL_inc_check=$withval/include/mysql],
    [MYSQL_lib_check="/usr/local/mysql/lib/mysql /usr/local/lib/mysql /opt/mysql/lib/mysql /usr/lib/mysql /usr/local/mysql/lib /usr/local/lib /opt/mysql/lib /usr/lib /usr/lib64 /usr/lib64/mysql"
     MYSQL_inc_check="/usr/local/mysql/include/mysql /usr/local/include/mysql /opt/mysql/include/mysql /usr/include/mysql"])

AC_ARG_WITH(mysql-lib,
    AC_HELP_STRING([--with-mysql-lib=DIR],
	           [directory of MySQL library installation]),
    [MYSQL_lib_check=$withval/mysql])

AC_ARG_WITH(mysql-include,
    AC_HELP_STRING([--with-mysql-include=DIR],
                   [directory of MySQL header installation]),
    [MYSQL_inc_check=$withval/mysql])

AC_MSG_CHECKING([for MySQL library directory])
MYSQL_libdir=
for m in $MYSQL_lib_check; do
    if test -d "$m" && \
       (test -f "$m/libmysqlclient.so" || test -f "$m/libmysqlclient.a")
    then
        MYSQL_libdir=$m
        break
    fi
done

if test -z "$MYSQL_libdir"; then
    AC_MSG_ERROR([Didn't find the MySQL library dir in '$MYSQL_dir_check'])
fi

case "$MYSQL_libdir" in
  /* ) ;;
  * )  AC_MSG_ERROR([The MySQL library directory ($MYSQL_libdir) must be an absolute path.]) ;;
esac

AC_MSG_RESULT([$MYSQL_libdir])

case "$MYSQL_libdir" in
  /usr/lib) ;;
  *) MYSQLLIBS="-L${MYSQL_libdir}" ;;
esac

AC_MSG_CHECKING([for MySQL include directory])
MYSQL_incdir=
for m in $MYSQL_inc_check; do
    if test -d "$m" && test -f "$m/mysql.h"
    then
        MYSQL_incdir=$m
        break
    fi
done

if test -z "$MYSQL_incdir"; then
    AC_MSG_ERROR([Didn't find the MySQL include dir in '$MYSQL_inc_check'])
fi

case "$MYSQL_incdir" in
  /* ) INCLUDES="$INCLUDES -I$MYSQL_incdir" ;;
  * )  AC_MSG_ERROR([The MySQL include directory ($MYSQL_incdir) must be an absolute path.]) ;;
esac
AC_MSG_RESULT([$MYSQL_incdir])
AC_DEFINE_UNQUOTED(HAVE_LIBMYSQL, "1", [Define to 1, if libmysql is installed])
MYSQLLIBS="$MYSQLLIBS -lmysqlclient"

#
# Location of MySQL++ installation
#
AC_ARG_WITH(mysqlpp,
    AC_HELP_STRING([--with-mysqlpp=DIR],
	           [search for MySQL++ in DIR/include and DIR/lib]),
    [MYSQLPP_lib_check=$withval/mysql++ $withval/lib
     MYSQLPP_inc_check=$withval/mysql++],
    [MYSQLPP_lib_check="/usr/local/mysql++/lib/mysql++ /usr/local/lib/mysql++ /opt/mysql++/lib/mysql++ /usr/lib/mysql++ /usr/local/mysql++/lib /usr/local/lib /opt/mysql++/lib /usr/lib /usr/lib64 /usr/lib64"
     MYSQLPP_inc_check="/usr/local/mysql/include/mysql++ /usr/local/include/mysql++ /opt/mysql/include/mysql++ /usr/include/mysql++"])

AC_ARG_WITH(mysqlpp-lib,
    AC_HELP_STRING([--with-mysqlpp-lib=DIR],
	           [directory of MySQL++ library installation]),
    [MYSQLPP_lib_check=$withval])

AC_ARG_WITH(mysqlpp-include,
    AC_HELP_STRING([--with-mysqlpp-include=DIR],
                   [directory of MySQL++ header installation]),
    [MYSQLPP_inc_check=$withval])

AC_MSG_CHECKING([for MySQL++ library directory])
MYSQLPP_libdir=
for m in $MYSQLPP_lib_check; do
    if test -d "$m" && test -f "$m/libmysqlpp.so"
    then
        MYSQLPP_libdir=$m
        break
    fi
done

if test -z "$MYSQLPP_libdir"; then
    if test -n "$MYSQLPP_dir_check"; then
	AC_MSG_WARN([Didn't find the MySQL++ library dir in '$MYSQLPP_dir_check'])
    fi
    MYSQLPP_libdir=no
fi

case "$MYSQLPP_libdir" in
  /usr/lib) ;;
  no ) ;;
  /* ) MYSQLLIBS="$MYSQLLIBS -L${MYSQLPP_libdir}  -lmysqlpp" ;;
  * )  AC_MSG_ERROR([The MySQL++ library directory ($MYSQLPP_libdir) must be an absolute path.]) ;;
esac

AC_MSG_RESULT([$MYSQLPP_libdir])

AC_MSG_CHECKING([for MySQL++ include directory])
MYSQLPP_incdir=
for m in $MYSQLPP_inc_check; do
    if test -d "$m" && test -f "$m/mysql++.h"
    then
        MYSQLPP_incdir=$m
        break
    fi
done

if test -z "$MYSQLPP_incdir"; then
    if test -n "$MYSQLPP_inc_check"; then
        AC_MSG_WARN([Didn't find the MySQL++ include dir in '$MYSQLPP_inc_check'])
    fi
    MYSQLPP_incdir=no
fi

case "$MYSQLPP_incdir" in
  /* ) INCLUDES="$INCLUDES -I$MYSQLPP_incdir" ;;
  no) ;;
  * ) AC_MSG_ERROR([The MySQL++ include directory ($MYSQLPP_incdir) must be an absolute path.]) ;;
esac
AC_MSG_RESULT([$MYSQLPP_incdir])

if test -d "$MYSQLPP_incdir" -a -d "$MYSQLPP_libdir"; then
   MYSQLLIBS="$MYSQLLIBS -lmysqlpp"
   AC_DEFINE_UNQUOTED(HAVE_LIBMYSQLPP, "1", [Define to 1, if libmysqlpp is installed])
fi

AC_SUBST(INCLUDES)
AC_SUBST(MYSQLLIBS)


dnl Checks for header files.
AC_CHECK_HEADERS([sys/shm.h],, AC_MSG_ERROR([needed header sys/shm.h not found!]))

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

dnl Enable special features
ACTORS=0
FILMS=0
RECORDS=0
AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug=LEVEL],
                             [Enables additional traces]),
              if test "$enableval" != "no"; then
                  test "$enableval" = "yes" && enableval=0
                  CXXFLAGS="-g -O2 -DTRACELEVEL=$enableval";
              else
                  CXXFLAGS="-O2 -UTRACELEVEL"; fi)
AC_ARG_ENABLE(checks,
              AC_HELP_STRING([--enable-checks=LEVEL],
                             [Enables additional (internal) checks]),
              if test "$enableval" != "no"; then
                  test "$enableval" = "yes" && enableval=0
                  CXXFLAGS="$CXXFLAGS -DCHECK=$enableval";
              else
                  CXXFLAGS="$CXXFLAGS -DNDEBUG -UCHECK"; fi)
AC_ARG_ENABLE(actors,
              AC_HELP_STRING([--disable-actors],
                             [Disables handling of actors]))
if test "$enable_actors" != "no"; then
   if test "$enable_films" = "no"; then
      AC_MSG_WARN([--enable-actors needs films enabled! Enabling both ...])
      enable_films=yes
   fi
   ACTORS=1
fi
AC_ARG_ENABLE(films,
              AC_HELP_STRING([--disable-films],
                             [Disables handling of films]))
if test "$enable_films" != "no"; then
   FILMS=1
fi
AC_ARG_ENABLE(records,
              AC_HELP_STRING([--disable-records],
                             [Disables handling of records]))
if test "$enable_records" != "no"; then
   RECORDS=1
fi

PAGES=$(($ACTORS+$FILMS+$RECORDS))
AC_MSG_CHECKING([the number of enabled pages])
AC_MSG_RESULT([$PAGES])
if test $PAGES -eq 0; then
  AC_MSG_ERROR([Disabling all features makes the program quite useless, innit?])
fi


AC_DEFINE_UNQUOTED(WITH_ACTORS, $ACTORS, [Define if actors should be handled])
AC_DEFINE_UNQUOTED(WITH_FILMS, $FILMS, [Define if films should be handled])
AC_DEFINE_UNQUOTED(WITH_RECORDS, $RECORDS, [Define if records should be handled])

AM_CONDITIONAL(WITH_ACTORS, test $ACTORS -eq 1)
AM_CONDITIONAL(WITH_FILMS, test $FILMS -eq 1)
AM_CONDITIONAL(WITH_RECORDS, test $RECORDS -eq 1)


AC_OUTPUT(Makefile po/Makefile.in src/Makefile doc/Makefile intl/Makefile\
	  SQL/Makefile data/Makefile)


dnl Settings for autoheader
AH_TOP([#ifndef CDMANAGER_CFG_H
#define CDMANAGER_CFG_H

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
])
AH_BOTTOM([#if defined (HAVE_GETTEXT) && defined (ENABLE_NLS)
#  include <libintl.h>
#  ifdef DONT_CONVERT
#     define _(String)                        gettext (String)
#  else
#     include <glibmm/convert.h>

#     define _(String)                        Glib::locale_to_utf8 (gettext (String))
#  endif
#  define N_(String)                          gettext_noop (String)
#  define gettext_noop(String)                (String)

#  define LANGUAGES			      CATALOGS
#else
#  define _(String)                           (String)
#  define N_(String)                          (String)
#  define ngettext(Str1, Str2, flag)          (((flag) == 1) ? (Str1) : (Str2))
#  define  binddomain(Domain)
#  define bindtextdomain(Package, Directory)

#  define LANGUAGES			      "en"
#endif

#endif])
