dnl Process this file with autoconf to produce a configure script.

dnl Initialise.   Look for lndir.c to double-check that we are looking
dnl at the correct directory.
AC_INIT
AC_CONFIG_SRCDIR([lndir.c])
dnl AC_CONFIG_HEADER(aconf.h)

dnl We have to do this next bit in order to appeast automake. 
dnl We don't really want a version number for the test suite 
dnl separate from that of CSSC itself (at least at the moment).
AM_INIT_AUTOMAKE(CSSC-testsuite,same-version-as-CSSC-itself)



dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

dnl Checks for header files.
AC_CHECK_HEADERS(stdarg.h stdlib.h string.h stdio.h errno.h)
AC_CHECK_HEADERS(unistd.h sys/stat.h sys/param.h)
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_STAT

AC_CHECK_FUNCS(symlink readlink)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT



