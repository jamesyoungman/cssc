# The code in this folder currently isn't used since we haven't yet
# replaced googletest.

# We define the global AM_CPPFLAGS as everything we compile includes from these
# directories.
AM_CPPFLAGS = -pthread -I$(srcdir)/../src
# Modifies compiler and linker flags for pthreads compatibility.
AM_LIBS =
if HAVE_PTHREADS
  AM_CXXFLAGS = @PTHREAD_CFLAGS@
  AM_LIBS += @PTHREAD_LIBS@
else
  AM_CXXFLAGS =
endif

AM_LDFLAGS = -L../gl/lib
LDADD = ../src/libcssc.a -lgnulib


EXTRA_DIST = TODO.txt

# Build rules for libraries.
TESTS_ENVIRONMENT = sh testwrapper.sh
TESTS = $(unit_tests)
check_SCRIPTS = testwrapper.sh
unit_tests = test_sid test_relvbr \
	test_release test_sid_list test_rel_list test_sccsdate \
	test_delta test_delta-table test_encoding \
	test_encoding2 test_linebuf test_split test_failure

check_PROGRAMS = $(unit_tests) test_bigfile

test_sid_SOURCES = test_sid.cc
test_relvbr_SOURCES = test_relvbr.cc
test_release_SOURCES = test_release.cc
test_sid_list_SOURCES = test_sid_list.cc
test_rel_list_SOURCES = test_rel_list.cc
test_sccsdate_SOURCES = test_sccsdate.cc
test_delta_SOURCES = test_delta.cc
test_delta_table_SOURCES = test_delta-table.cc
test_encoding_SOURCES = test_encoding.cc
test_encoding2_SOURCES = test_encoding2.cc
test_linebuf_SOURCES = test_linebuf.cc
test_split_SOURCES = test_split.cc
test_failure_SOURCES = test_failure.cc
test_bigfile_SOURCES = test_bigfile.cc



# The death tests may leave a core file behind.  Some systems also use
# core.NNNN as the name of a core file (NNNN is usually the PID).
# Files with names like vgcore.* are produced by valgrind.
COREFILES = core core.* vgcore.*

DISTCLEANFILES = $(COREFILES)
