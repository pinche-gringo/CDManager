dnl $Id: configure.ac,v 1.2 2004/10/30 14:46:20 markus Rel $
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)
AC_REVISION($Revision: 1.2 $)
AC_INIT(CDManager, 0.1.00, [g17m0@lycos.com], CDManager)

RELEASE=${PACKAGE_VERSION%.*}
AC_DEFINE_UNQUOTED(PRG_RELEASE, "$RELEASE", [Define to the release of the program])
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(RELEASE)

AM_INIT_AUTOMAKE(dist-bzip2)

INCLUDES="-I. -I\$(top_srcdir) -DLOCALEDIR=\\\"\$(datadir)/locale\\\" -DDOCUDIR=\\\"\$(datadir)/doc/\$(PACKAGE)/\\\""

AC_C_BIGENDIAN

ALL_LINGUAS="de en es"

dnl Checks for programs.
AC_LANG_CPLUSPLUS
AM_CONFIG_HEADER(cdmgr-cfg.h)

AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AM_GNU_GETTEXT

dnl Checks for libraries.
AC_CHECK_LIB(stdc++, main)
AC_CHECK_LIB(mysqlpp, main)

PKG_CHECK_MODULES(DEPS, gtkmm-2.4 >= 2.4.0 gthread-2.0 > 2.2.0
                  YGP >= 0.6.40 XGP >= 0.6.47)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)


#
# Location of MySQL installation
#
AC_ARG_WITH(mysql,
    AC_HELP_STRING([--with-mysql=PATH],
	           [root directory path of MySQL installation]),
    [MYSQL_lib_check=$withval/lib/mysql $with_mysql/lib
     MYSQL_inc_check=$withval/include/mysql],
    [MYSQL_lib_check="/usr/local/mysql/lib/mysql /usr/local/lib/mysql /opt/mysql/lib/mysql /usr/lib/mysql /usr/local/mysql/lib /usr/local/lib /opt/mysql/lib /usr/lib"
MYSQL_inc_check="/usr/local/mysql/include/mysql /usr/local/include/mysql /opt/mysql/include/mysql /usr/include/mysql"])

AC_ARG_WITH(mysql-lib,
    AC_HELP_STRING([--with-mysql-lib=PATH],
	           [directory path of MySQL library installation]),
    [MYSQL_lib_check=$withval/mysql])

AC_ARG_WITH(mysql-include,
    AC_HELP_STRING([--with-mysql-include=PATH],
                   [directory path of MySQL header installation]),
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
    AC_MSG_ERROR([Didn't find the mysql library dir in '$MYSQL_dir_check'])
fi

case "$MYSQL_libdir" in
  /* ) ;;
  * )  AC_MSG_ERROR([The MySQL library directory ($MYSQL_libdir) must be an absolute path.]) ;;
esac

AC_MSG_RESULT([$MYSQL_libdir])

case "$MYSQL_libdir" in
  /usr/lib) ;;
  *) LDFLAGS="$LDFLAGS -L${MYSQL_libdir}" ;;
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
    AC_MSG_ERROR([Didn't find the mysql include dir in '$MYSQL_inc_check'])
fi

case "$MYSQL_incdir" in
  /* ) ;;
  * )  AC_MSG_ERROR([The MySQL include directory ($MYSQL_incdir) must be an absolute path.]) ;;
esac
AC_MSG_RESULT([$MYSQL_incdir])

#
# Location of MySQL++ installation
#
AC_ARG_WITH(mysqlpp,
    AC_HELP_STRING([--with-mysql++=PATH],
	           [root directory path of MySQL++ installation]),
    [MYSQLPP_lib_check=$withval/mysql++ $withval/lib
     MYSQLPP_inc_check=$withval/mysql++],
    [MYSQLPP_lib_check="/usr/local/mysql++/lib/mysql++ /usr/local/lib/mysql++ /opt/mysql++/lib/mysql++ /usr/lib/mysql++ /usr/local/mysql++/lib /usr/local/lib /opt/mysql++/lib /usr/lib"
MYSQLPP_inc_check="/usr/local/mysql/include/mysql++ /usr/local/include/mysql++ /opt/mysql/include/mysql++ /usr/include/mysql++"])

AC_ARG_WITH(mysqlpp-lib,
    AC_HELP_STRING([--with-mysql++-lib=PATH],
	           [directory path of MySQL++ library installation]),
    [MYSQLPP_lib_check=$withval])

AC_ARG_WITH(mysqlpp-include,
    AC_HELP_STRING([--with-mysql++-include=PATH],
                   [directory path of MySQL++ header installation]),
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
    AC_MSG_ERROR([Didn't find the mysql++ library dir in '$MYSQLPP_dir_check'])
fi

case "$MYSQLPP_libdir" in
  /* ) ;;
  * )  AC_MSG_ERROR([The MySQL++ library directory ($MYSQLPP_libdir) must be an absolute path.]) ;;
esac

AC_MSG_RESULT([$MYSQLPP_libdir])

case "$MYSQLPP_libdir" in
  /usr/lib) ;;
  *) LDFLAGS="$LDFLAGS -L${MYSQLPP_libdir}" ;;
esac

AC_MSG_CHECKING([for MySQL++ include directory])
MYSQLPP_incdir=
for m in $MYSQLPP_inc_check; do
    if test -d "$m" && test -f "$m/mysql++.hh"
    then
        MYSQLPP_incdir=$m
        break
    fi
done

if test -z "$MYSQLPP_incdir"; then
    AC_MSG_ERROR([Didn't find the mysql++ include dir in '$MYSQLPP_inc_check'])
fi

case "$MYSQLPP_incdir" in
  /* ) ;;
  * )  AC_MSG_ERROR([The MySQL++ include directory ($MYSQLPP_incdir) must be an absolute path.]) ;;
esac
AC_MSG_RESULT([$MYSQLPP_incdir])

INCLUDES="$INCLUDES -I$MYSQLPP_incdir -I$MYSQL_incdir"
AC_SUBST(INCLUDES)


dnl Checks for header files.

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

dnl Enable special features
AC_ARG_ENABLE(debug,
              AC_HELP_STRING([[[[[--enable-debug[=LEVEL]]]]]],
                             [Enables additional traces]),
              if test "$enableval" != "no"; then
                  test "$enableval" == "yes" && enableval=0
                  CXXFLAGS="-g -O2 -DTRACELEVEL=$enableval";
              else
                  CXXFLAGS="-O2 -UTRACELEVEL"; fi)
AC_ARG_ENABLE(checks,
              AC_HELP_STRING([[[[[--enable-checks[=LEVEL]]]]]],
                             [Enables additional (internal) checks]),
              if test "$enableval" != "no"; then
                  test "$enableval" == "yes" && enableval=0
                  CXXFLAGS="$CXXFLAGS -DCHECK=$enableval";
              else
                  CXXFLAGS="$CXXFLAGS -DNDEBUG -UCHECK"; fi)


AC_OUTPUT(Makefile intl/Makefile po/Makefile.in src/Makefile doc/Makefile\
	  SQL/Makefile)


dnl Settings for autoheader
AH_TOP([#ifndef CARDGAMES_CFG_H
#define CARDGAMES_CFG_H

//$Id: configure.ac,v 1.2 2004/10/30 14:46:20 markus Rel $

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
#  define _(String)                           Glib::locale_to_utf8 (gettext (String))
#  define N_(String)                          gettext_noop (String)
#  define gettext_noop(String)                (String)
#else
#  define _(String)                           (String)
#  define N_(String)                          (String)
#  define ngettext(Str1, Str2, flag)          (((flag) == 1) ? (Str1) : (Str2))
#  define  binddomain(Domain)
#  define bindtextdomain(Package, Directory)
#endif

#endif])
