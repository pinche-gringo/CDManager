dnl $Id: configure.ac,v 1.1 2004/10/11 01:53:02 markus Exp $
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)
AC_REVISION($Revision: 1.1 $)
AC_INIT(CDManager, 0.1.00, [g17m0@lycos.com], CDManager)

RELEASE=${PACKAGE_VERSION%.*}
AC_DEFINE_UNQUOTED(PRG_RELEASE, "$RELEASE", [Define to the release of the program])
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(RELEASE)

AM_INIT_AUTOMAKE(dist-bzip2)

INCLUDES="-I. -I\$(top_srcdir) -DLOCALEDIR=\\\"\$(datadir)/locale\\\" -DDOCUDIR=\\\"\$(datadir)/doc/\$(PACKAGE)/\\\""
AC_SUBST(INCLUDES)

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

PKG_CHECK_MODULES(DEPS, gtkmm-2.4 >= 2.4.0 gthread-2.0 > 2.2.0
                  YGP >= 0.6.40 XGP >= 0.6.47)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

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


AC_OUTPUT(Makefile intl/Makefile po/Makefile.in src/Makefile doc/Makefile)


dnl Settings for autoheader
AH_TOP([#ifndef CARDGAMES_CFG_H
#define CARDGAMES_CFG_H

//$Id: configure.ac,v 1.1 2004/10/11 01:53:02 markus Exp $

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
