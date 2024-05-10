# Autoconf configuration for kgrep
#
AC_INIT([kgrep],[1.0.14],[jchern@naver.com],[kgrep])
PACKAGE=kgrep

AC_PREREQ(2.50)
AC_COPYRIGHT([Copyright 2009- Kim Eung Gyun])

AC_CONFIG_AUX_DIR([build])

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([include/auto_config.h])

AC_PREFIX_DEFAULT(`pwd`/install)
AC_MSG_RESULT("prefix is $prefix.")

dnl ---------------------------------------------------------------------------
AH_TOP([#ifndef AUTO_CONFIG_H
#define AUTO_CONFIG_H 1
])

AH_BOTTOM([
#endif /* AUTO_CONFIG_H */
])

AH_VERBATIM([1_NEAR_TOP__],[
/* example */
#undef TIME_HASH_HOME
])

AC_DEFINE(TIME_HASH_HOME,`pwd`)

dnl ---------------------------------------------------------------------------
dnl Checks for programs
AC_PROG_CC
AC_PROG_INSTALL

#AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

dnl should fail configure when followings headers does not exist.
AC_CHECK_HEADERS( \
  string.h stdlib.h stdio.h sys/types.h sys/stat.h \
  getopt.h \
)

AC_CHECK_FUNCS( \
  getopt \
  getopt_long \
)

TH_RELEASE_DATE=`LANG=C date "+%a %b %d %T %Z %Y"`

AC_SUBST(PACKAGE)
AC_SUBST(TH_RELEASE_DATE)

AC_SUBST(LIBTOOL)

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(AM_LIBTOOLFLAGS)

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/kgrep.c
  include/Makefile
  wrapper/Makefile
  wrapper/kgrep.py
  wrapper/test_kgrep.py
])

AC_OUTPUT

