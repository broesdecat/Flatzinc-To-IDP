# Copyright 2011 Katholieke Universiteit Leuven
# Use of this software is governed by the GNU LGPLv3.0 license
# Written by Broes De Cat, K.U.Leuven, Departement
# Computerwetenschappen, Celestijnenlaan 200A, B-3001 Leuven, Belgium
AC_INIT([flatzinc-to-idp],[1.0.0],[krr@cs.kuleuven.be])
AC_CONFIG_HEADERS([config.h])

#this is not an ideal solution, but it overrides the default cxxflags, so those set later are not invalid
if test -z $CXXFLAGS; then
    CXXFLAGS=" "
fi
AC_PROG_CXX
AC_DECL_YYTEXT
AM_PROG_LEX
AC_PROG_LIBTOOL
AC_PROG_YACC
AM_INIT_AUTOMAKE([foreign subdir-objects silent-rules])
AC_LANG([C++])

AC_HEADER_STDC
AC_CHECK_HEADERS([\
	algorithm assert.h cassert cerrno cstdio cstdlib cstring ctime exception float.h \
	fstream inttypes.h iostream limits limits.h list map queue \
	set sstream stack stdexcept stdint.h stdio.h stdlib.h string vector])
AC_TYPE_SIZE_T

AC_SUBST([AC_CXXFLAGS])
AC_SUBST([AC_LDFLAGS])

AC_CXXFLAGS=" "
AC_LDFLAGS=" "

AC_CXXFLAGS+="-Wall -Wextra -pedantic "

AC_ARG_ENABLE([build],
[  --enable-build    Turn on debugging (debug), profiling (profile), release version(release), static release version (static) or code cover building (codecover)],
[case "${enableval}" in
  debug)   AC_CXXFLAGS+="-O0 -ggdb -D DEBUG " 		AC_LDFLAGS+="-ggdb " ;;
  profile) AC_CXXFLAGS+="-O3 -pg -ggdb -D NDEBUG " 	AC_LDFLAGS+="-ggdb " ;;
  release) AC_CXXFLAGS+="-O3 -D NDEBUG " 			AC_LDFLAGS+="" ;;
  static)  AC_CXXFLAGS+="-O3 -D NDEBUG " 			AC_LDFLAGS+="-static " ;;
  codecover) AC_CXXFLAGS+="-O0 -fprofile-arcs -ftest-coverage -ggdb -D DEBUG " AC_LDFLAGS+="-ggdb " ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-build]) ;;
esac],[		AC_CXXFLAGS+="-DNDEBUG -O3 " AC_LDFLAGS+=" "])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT