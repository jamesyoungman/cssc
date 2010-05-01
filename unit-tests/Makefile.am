# We define the global AM_CPPFLAGS as everything we compile includes from these
# directories.
AM_CPPFLAGS = -I$(srcdir)/../googletest -I$(srcdir)/../googletest/include -I$(srcdir)/..

# Modifies compiler and linker flags for pthreads compatibility.
if HAVE_PTHREADS
  AM_CXXFLAGS = @PTHREAD_CFLAGS@ -DGTEST_HAS_PTHREAD=1
  AM_LIBS = @PTHREAD_LIBS@
else
  AM_CXXFLAGS = -DGTEST_HAS_PTHREAD=0
endif

EXTRA_DIST = TODO.txt

# Build rules for libraries.
LDADD = ../googletest/lib/libgtest.la \
	../googletest/lib/libgtest_main.la \
	../libcssc.a

TESTS_ENVIRONMENT = sh testwrapper.sh
TESTS = $(unit_tests)
check_SCRIPTS = testwrapper.sh
unit_tests = test_mylist test_myset test_sid test_relvbr \
	test_release test_sid_list test_rel_list test_sccsdate \
	test_delta test_stack test_delta-table test_encoding \
	test_encoding2

check_PROGRAMS = $(unit_tests) test_bigfile

test_mylist_SOURCES = test_mylist.cc
test_myset_SOURCES = test_myset.cc
test_sid_SOURCES = test_sid.cc
test_relvbr_SOURCES = test_relvbr.cc
test_release_SOURCES = test_release.cc
test_sid_list_SOURCES = test_sid_list.cc
test_rel_list_SOURCES = test_rel_list.cc
test_sccsdate_SOURCES = test_sccsdate.cc
test_delta_SOURCES = test_delta.cc
test_stack_SOURCES = test_stack.cc
test_delta_table_SOURCES = test_delta-table.cc
test_encoding_SOURCES = test_encoding.cc
test_encoding2_SOURCES = test_encoding2.cc
test_bigfile_SOURCES = test_bigfile.cc
test_bigfile_LDADD = ../libcssc.a

# The death tests may leave a core file behind.  Some systems also use
# core.NNNN as the name of a core file (NNNN is usually the PID).
DISTCLEANFILES = core core.*
