dnl Process this file with autoconf to produce a configure script.
dnl 
dnl configure.in: Part of GNU CSSC.
dnl 
dnl    Copyright (C) 1997,1998,1999,2000,2001,2002 Free Software Foundation, Inc. 
dnl 
dnl    This program is free software; you can redistribute it and/or modify
dnl    it under the terms of the GNU General Public License as published by
dnl    the Free Software Foundation; either version 2 of the License, or
dnl    (at your option) any later version.
dnl 
dnl    This program is distributed in the hope that it will be useful,
dnl    but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl    GNU General Public License for more details.
dnl 
dnl    You should have received a copy of the GNU General Public License
dnl    along with this program; if not, write to the Free Software
dnl    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
dnl 
dnl $Id: configure.ac,v 1.1 2002/07/26 17:21:00 james_youngman Exp $

dnl Initialise.   Look for sccsfile.h to double-check that we are looking
dnl at the correct directory.
AC_INIT
AC_CONFIG_SRCDIR([sccsfile.h])

# The package version should match the regexp
# ([0-9]+)\.([0-9]+)([a-z])?
# ...at least according to the "Gnits" rules outlined
# in the Automake manual.
#   usually use -CVSDEVEL to indicate that the person who compiled 
#   the code got it from CVS.
AM_INIT_AUTOMAKE(CSSC,0.14alpha.pl1-CVSDEVEL)

dnl AC_CONFIG_HEADER(config.h.)
AM_CONFIG_HEADER(config.h)

dnl Feature and package checks
binary_defaulted=no
AC_MSG_CHECKING([if binary file support should be disabled])
AC_ARG_ENABLE(binary,
[--enable-binary		  

    Enables support for creating "binary" SCCS files (required for
    interoperability with Solaris).  If binary support is disabled,
    encoded SCCS files can still be read, but not created.  You should
    disable this feature only if you need to interoperate with a
    version of SCCS which lacks binary file support.  To disable this
    feature, use --enable-binary=no or --disable-binary.  This setting
    is overridden by the environment variable CSSC_BINARY_SUPPORT.

--disable-binary

    This is the opposite of --enable-binary; see above.
],
dnl action if given
,
dnl action if not given
binary_defaulted=yes
enable_binary=yes
)
if test "$enable_binary" = yes; then
	AC_MSG_RESULT(no)
else
	AC_DEFINE(CONFIG_DISABLE_BINARY_SUPPORT)
	AC_MSG_RESULT(yes)
fi
if test "$binary_defaulted" = "yes"; then
	echo WARNING
	echo WARNING This was assumed because you did not specify either 
	echo WARNING --enable-binary or --disable-binary.	
	echo WARNING
	# Pause for a bit so that the user notices this message.
	( sleep 10 2>/dev/null )
fi
AC_SUBST(enable_binary)dnl

AC_MSG_CHECKING([for required SCCS file line length limits])
AC_ARG_ENABLE(max-line-length,
[--enable-max-line-length=N     

    Sets the maximum line length allowed in SCCS files to N.  By
    default, CSSC has no limit.  Use this option only if you need to
    make CSSC interoperate with other versions of SCCS.  Setting this 
    option to 0 will impose no limit (which is the default in any case).
    This setting is overridden by the environment variable 
    CSSC_MAX_LINE_LENGTH.

--disable-max-line-length

    This is equivalent to --enable-max-line-length=0 (which is the default).
],
if test "$enable_max_line_length" = "no"
then 
	enable_max_line_length=0
	AC_MSG_RESULT(no limit)
else
	AC_MSG_RESULT(limit is $enable_max_line_length)
fi
max_line_length_description=$enable_max_line_length

,
AC_MSG_RESULT(no limit)
enable_max_line_length=0
)
AC_DEFINE_UNQUOTED(CONFIG_MAX_BODY_LINE_LENGTH,$enable_max_line_length)


if test $enable_max_line_length -eq 0 
then
	max_line_length_description="no limit"
else
	max_line_length_description=$enable_max_line_length
fi

AC_SUBST(max_line_length_description)dnl



dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC

dnl Figure out how to get a POSIX compilation environment on ISC.
dnl We have to call this macro after the [AC_PROG_CC] macro and
dnl before any other macro that calls the compiler.   We also do
dnl something similar for MINIX (though CSSC has never been tested
dnl on Minix as far as I know).
AC_ISC_POSIX
AC_MINIX

AC_PROG_CXX
AC_SUBST(GXX)dnl
AC_PROG_CPP
AC_PROG_CXXCPP
dnl AC_PROG_INSTALL([])
AC_PROG_RANLIB


dnl The next test must be done in C++.
 
AC_LANG([C++])

dnl Determine if we need to work around the bogus printf macro in
dnl glibc 2.2.3, with GCC versions 2.97 and above. 
dnl 
dnl We cannot do this test with AC_EGREP_CPP because the configure 
dnl script does not pass the correct -O option to the compiler (well, 
dnl the preprocessor)
dnl 
AC_MSG_CHECKING([if you are using a broken version of glibc with a version of GCC (>=2.97) that doesn't like it])
AC_TRY_COMPILE([
#include <stdio.h>
int x = 0;
class foo { 
public:
foo() { }
void some_member_function(void) { };
int printf(const char *fmt, ...); 
};
],
[/* no function body*/],
AC_MSG_RESULT(no),
AC_MSG_RESULT(yes)

oldCFLAGS="$CFLAGS"
oldCXXFLAGS="$CXXFLAGS"
if echo "$CFLAGS" | grep -e "-O" >/dev/null
then
	CFLAGS="-g"
	CXXFLAGS="-g"
[
echo 
echo WARNING It seems that you are using a version of GNU lilbc 
echo WARNING that has a broken printf macro in /usr/include/bits/stdio.h.
echo WARNING I have changed CFLAGS and CXXFLAGS to turn optimisation off 
echo WARNING in order to work around the problem.   
echo WARNING
echo WARNING Other possible workarounds would be to use GCC 2.96 or
echo WARNING earlier, or a version of glibc which does not have this
echo WARNING bug.
echo WARNING
echo WARNING CFLAGS   has been changed from "$oldCFLAGS"   to "$CFLAGS"
echo WARNING CXXFLAGS has been changed from "$oldCXXFLAGS" to "$CXXFLAGS"
echo 
]
fi 
)

AC_LANG([C])
	 
AC_OBJEXT
AC_EXEEXT


AC_PATH_PROG(UNAME,uname,/bin/uname)
AC_PATH_PROG(PR,pr,/usr/bin/pr)
dnl It would be a good idea to default the path for diff
dnl to install_dir/diff-program so the installer can 
dnl add a symlink later.   TODO.
AC_PATH_PROG(CONFIG_DIFF_COMMAND,diff,/usr/bin/diff)
AC_DEFINE_UNQUOTED(CONFIG_DIFF_COMMAND,"$ac_cv_path_CONFIG_DIFF_COMMAND")


AC_MSG_CHECKING([for GNU diff])
if $ac_cv_path_CONFIG_DIFF_COMMAND --version 2>/dev/null | grep "GNU diff" >/dev/null
then
	AC_MSG_RESULT(yes)
	AC_DEFINE_UNQUOTED(HAVE_GNU_DIFF)	
else
	AC_MSG_RESULT(no)
fi


##
## Search for something we can use to decompress a .Z file.
## 
AC_CHECK_PROG(ZCAT_PROGRAM, uncompress, uncompress)
if test "$ZCAT_PROGRAM" = ""
then
  AC_CHECK_PROG(ZCAT_PROGRAM, compress,   compress -d)
fi
if test "$ZCAT_PROGRAM" = ""
then
  AC_CHECK_PROG(ZCAT_PROGRAM, gzip,       gzip -d)
fi
if test "$ZCAT_PROGRAM" = ""
then
  AC_CHECK_PROG(ZCAT_PROGRAM, zcat,       zcat)
fi
if test "$ZCAT_PROGRAM" = ""
then
  AC_CHECK_PROG(ZCAT_PROGRAM, gunzip,     gunzip)
fi

if test "$ZCAT_PROGRAM" = ""
then
echo 
echo "WARNING: Cannot find a program that will decompress a compressed file."
echo "WARNING: This means that you cannot run the test suite, since some"
echo "WARNING: if the test input files in the test suite are provided"
echo "WARNING: compressed.  The software should still compile."
echo "WARNING: "
echo "WARNING: If you have a command which will decompress a .Z file,"
echo "WARNING: please set the variable ZCAT_PROGRAM and re-run configure."
echo 
fi


				

dnl Checks for libraries.

dnl Checks for header files.

dnl Standard C header files:-
dnl We check these with language set to C only, otherwise autoconf-2.13 will 
dnl attempt to make the check with the C++ compiler.  It will then moan about
dnl an undeclared "exit" library function. 
AC_LANG([C])
AC_HEADER_STDC

AC_LANG([C++])
AC_CHECK_HEADERS(stdarg.h stdlib.h string.h stdio.h errno.h locale.h limits.h)

dnl Posix/Unix header files:-
AC_CHECK_HEADERS(fcntl.h prototypes.h io.h process.h pwd.h unistd.h)
AC_CHECK_HEADERS(sys/stat.h sys/param.h sys/types.h sys/file.h)
AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT
AC_HEADER_STAT

dnl C++ Header files...
AC_CHECK_HEADERS(string)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Do not use AC_C_CONST, because the tests for a fully working C "const"
dnl break when you use a standard-conforming C++ compiler.
dnl AC_C_CONST

AC_TYPE_SIZE_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_UID_T

dnl Can we use exceptions?
AC_MSG_CHECKING([for support for C++ exceptions])
AC_TRY_COMPILE( ,
	[throw int(1);],
	AC_DEFINE_UNQUOTED(HAVE_EXCEPTIONS)
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))

dnl Does the compiler support the "std" namespace?
dnl GCC version 3 not only supports this but insists on it, 
dnl so if this feature is supported, we always use it. 
AC_MSG_CHECKING([for support for C++ "std" namespace])
AC_TRY_COMPILE(
#ifdef HAVE_STRING
#include <string>
#endif
,
	[std::string s;],
	AC_DEFINE_UNQUOTED(HAVE_STD_NAMESPACE)
	AC_DEFINE(STD_NAMESPACE,std::)
	AC_MSG_RESULT(yes),
	AC_DEFINE(STD_NAMESPACE,/*nothing*/)
	AC_MSG_RESULT(no))


dnl AC_CHECK_DECL_IN_HEADER(IDENTIFIER, HEADERS)
AC_DEFUN(AC_CHECK_DECL_IN_HEADER,
AC_MSG_CHECKING(for $1 in $2)
ac_tr_ident=`echo $1 | tr '.abcdefghijklmnopqrstuvwxyz' '_ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
ac_tr_header=`echo $2 | tr '.abcdefghijklmnopqrstuvwxyz' '_ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
ac_tr_define=${ac_tr_header}_DECLARES_${ac_tr_ident}
AC_CACHE_VAL(ac_cv_decl_$ac_tr_define,
[AC_EGREP_HEADER([$1], [$2], 
eval "ac_cv_decl_$ac_tr_define=yes", 
eval "ac_cv_decl_$ac_tr_define=no")])dnl
if eval "test \"`echo '$ac_cv_decl_'$ac_tr_define`\" = yes"; then
	 AC_MSG_RESULT(yes)
	 # echo 'defining' $ac_tr_define
	 AC_DEFINE_UNQUOTED($ac_tr_define)
else
	 # echo 'NOT defining' $ac_tr_define 
	 AC_MSG_RESULT(no)
fi
)

dnl Deductions from proprocessor macros...
AC_MSG_CHECKING([for _POSIX_SAVED_IDS])
AC_CACHE_VAL(cssc_cv_sys_savedids_set,
AC_EGREP_CPP(yes,
[
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _POSIX_SAVED_IDS
       yes
#endif
], 
cssc_cv_sys_savedids_set=yes,
cssc_cv_sys_savedids_set=no
))
if test "$cssc_cv_sys_savedids_set" = "yes"; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(SAVED_IDS_OK)
else
	AC_MSG_RESULT(no)
fi


dnl Look for declarations in headers.

dnl bsd/sccs.c uses sys_siglist[], if available.
AC_DECL_SYS_SIGLIST

dnl fatalsig.cc uses signal handlers
AC_TYPE_SIGNAL

dnl Checks for library functions.
dnl AC_FUNC_MEMCMP
dnl The "select" that autoscan found was a method. AC_CHECK_FUNCS(select)

AC_CHECK_FUNCS(abort wait)

dnl Check for symlink() and readlink(), which lndir uses.
AC_CHECK_FUNCS(symlink readlink unsetenv fsetpos fileno fstat sysconf memchr) 
AC_CHECK_FUNCS(stat getpwuid getlogin setreuid pipe spawn geteuid getegid)
AC_REPLACE_FUNCS(strerror remove rename strstr)



dnl
dnl On AmigsOS, fork() is a stub (in ixemul.library).  This means that
dnl AC_CHECK_FUNC will find it and so unless we handle it specially, 
dnl configure will set up for using fork().  For AmigaOS, we will probably
dnl use system().  On AmigaOS system() is not going to be a security hole,
dnl since there is no concept of separate users and so no setuid execution,
dnl which is the potential problem with system, and the reason we don't
dnl use it by default.
dnl
AC_MSG_CHECKING([for AmigaOS target])
AC_EGREP_CPP(yes,
[#ifdef __amigaos__
       yes
#endif
], is_amigaos=yes, is_amigaos=no)

if test $is_amigaos = yes 
then
	AC_MSG_RESULT([avoiding fork() on AmigaOS]),
else
	AC_MSG_RESULT(no)
	AC_CHECK_FUNC(fork,AC_DEFINE_UNQUOTED(HAVE_FORK))
fi



dnl AC_SUBST(LIBOBJS)dnl



AC_DEFUN(AC_CHECK_GLOBAL,
[
# JY: AC_CHECK_GLOBAL begins
for ac_global in $1
do
   ac_tr_global=HAVE_`echo $ac_global | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
   AC_MSG_CHECKING([for global variable ${ac_global}])
   AC_CACHE_VAL(ac_cv_global_$ac_global,
   [
    AC_TRY_LINK(dnl
    [/* no includes */],
    [ extern long int $ac_global;  exit((int)$ac_global)],
    eval "ac_cv_global_${ac_global}=yes",
    eval "ac_cv_global_${ac_global}=no"
    )
   ]
   )
  if eval "test \"`echo '$ac_cv_global_'$ac_global`\" = yes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE_UNQUOTED($ac_tr_global)
  else
    AC_MSG_RESULT(no)
  fi
done
# JY: AC_CHECK_GLOBAL ends
])


dnl Search for the global variables sys_nerr, sys_errlist.

dnl Cope with not having a strerror() function
if test "$ac_cv_func_strerror" != yes; then
dnl AC_CHECK_GLOBAL(sys_nerr)
dnl AC_CHECK_GLOBAL(sys_errlist)
AC_CHECK_DECL_IN_HEADER(sys_errlist,errno.h)
AC_CHECK_DECL_IN_HEADER(sys_errlist,stdlib.h)
AC_CHECK_DECL_IN_HEADER(sys_errlist,stdio.h)

AC_CHECK_DECL_IN_HEADER(sys_nerr,errno.h)
AC_CHECK_DECL_IN_HEADER(sys_nerr,stdlib.h)
AC_CHECK_DECL_IN_HEADER(sys_nerr,stdio.h)
fi

dnl What about saved setuid?
AC_CONFIG_SUBDIRS(testutils)
dnl AC_CONFIG_FILES([shorter/Makefile])
AC_OUTPUT

dnl *** NOTE:
dnl There can be only ONE call to AC_OUTPUT in each configure.in,
dnl otherwise you get an error when you run configure.  AC_OUTPUT
dnl deletes confdefs.h and so the second invocation can't find it
dnl and so things go wrong.

AC_CONFIG_FILES([version.cc Makefile sccsdiff.sh bsd/Makefile tests/Makefile docs/Makefile testutils/decompress_stdin.sh testutils/Makefile auxfiles/Makefile auxfiles/CSSC.spec sccs-cgi/Makefile unused/Makefile docs/config-info.texi])
AC_OUTPUT