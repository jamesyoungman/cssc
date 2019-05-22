generic_CPPFLAGS = -I ../gl/lib -I $(srcdir)/../gl/lib
AM_CPPFLAGS = $(generic_CPPFLAGS)
AM_CFLAGS = "-DPREFIX=\"$(csscutildir)/\"" "-DLOCALEDIR=\"$(localedir)\"" $(generic_CPPFLAGS)
AM_LDFLAGS = -L../gl/lib
LDADD = -lgnulib

noinst_PROGRAMS = lndir realpwd user yes ekko seeker yammer uu_decode
realpwd_SOURCES = realpwd.cc
EXTRA_DIST = last-time.c compare_gets.sh gcov-util.sh lndir.man mogrify.awk decompress_stdin.sh.in
DISTCLEANFILES = decompress_stdin.sh
