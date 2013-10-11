# Copyright (c) 2011 Sergey Kolotsey.
# This file if part of expat-dom library.
# See the file COPYING for copying permission.


AC_PREREQ(2.60)
LT_PREREQ(2.2.0)

dnl Init autoconf
dnl Increment LIBREVISION if source code has changed at all
dnl LIBVERSION=CURRENT.REVISION.AGE
dnl If the API has changed, increment LIBCURRENT and set LIBREVISION to 0
dnl If the API changes compatibly (i.e. simply adding a new function
dnl without changing or removing earlier interfaces), then increment LIBAGE.
dnl If the API changes incompatibly set LIBAGE back to 0

dnl AC_INIT defines AC_PACKAGE_VERSION and PACKAGE_VERSION
dnl AC_INIT defines AC_PACKAGE_NAME and PACKAGE_NAME
m4_include(acversion.m4)
AC_INIT(ac_PACKAGE_NAME, ac_VERSION, ac_EMAIL, ac_PACKAGE_NAME, ac_URL)
AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_VERSION)
AC_CONFIG_SRCDIR(expat-dom.c)
AC_CONFIG_AUX_DIR(tools)
AC_CONFIG_HEADERS(expat-config.h)

dnl $ac_add_cflags is used to set add -Wall to default CFLAGS value set by AC_PROG_CC
dnl the following condition tests if CFLAGS was set by user or not
if test -z "${CFLAGS}"; then
	ac_add_cflags="-Wall"
else
	ac_add_cflags=""
fi

dnl Init libtool to build libraries
LT_INIT
AC_PROG_LIBTOOL

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AM_PROG_INSTALL_STRIP
AC_PROG_MKDIR_P

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdint.h stdlib.h string.h unistd.h])

dnl Checks for library functions.
AC_CHECK_FUNCS([strcasecmp strdup strerror], [], [AC_MSG_ERROR([
Could not found standard C functions:
    strcasecmp, strdup, strerror
These functions are required
])])

dnl Checks for libraries.
EQ_CHECK_PACKAGE( EXPAT, expat, [expat:XML_ParserCreate], [expat.h])

CFLAGS="$CFLAGS $ac_add_cflags"
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
