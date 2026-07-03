generic_CPPFLAGS = -I ../gl/lib -I $(srcdir)/../gl/lib -I $(srcdir)/../src
AM_CPPFLAGS = $(generic_CPPFLAGS)
AM_CFLAGS = "-DPREFIX=\"$(csscutildir)/\"" "-DLOCALEDIR=\"$(localedir)\"" $(generic_CPPFLAGS)
AM_LDFLAGS = -L../src -L../gl/lib
AM_CXXFLAGS = $(WARN_CXXFLAGS)
LDADD = libtesting.a -lcssc  -lgnulib

noinst_PROGRAMS = lndir realpwd user yes ekko seeker yammer runtests
noinst_LIBRARIES = libtesting.a

libtesting_a_SOURCES = execute.cc execute.h filebody.cc filebody.h echo_unescape.c echo_unescape.h

ekko_SOURCES = ekko.c
realpwd_SOURCES = realpwd.cc
runtests_SOURCES = runtests.cc

EXTRA_DIST = last-time.c compare_gets.sh gcov-util.sh lndir.man mogrify.awk decompress_stdin.sh.in
DISTCLEANFILES = decompress_stdin.sh
