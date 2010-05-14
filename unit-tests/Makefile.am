# We define the global AM_CPPFLAGS as everything we compile includes from these
# directories.
AM_CPPFLAGS = -I$(srcdir)/googletest -I$(srcdir)/googletest/include -I$(srcdir)/../src -DGTEST_HAS_PTHREAD=0
AM_LIBS = 
LDADD = libgtest.a libgtest_main.a ../src/libcssc.a

EXTRA_DIST = TODO.txt \
	googletest/COPYING \
	googletest/CONTRIBUTORS \
	googletest/README \
	googletest/CHANGES

# Build rules for libraries.

noinst_LIBRARIES = libgtest.a libgtest_main.a

gtest_headers = \
	googletest/include/gtest/gtest-death-test.h \
	googletest/include/gtest/gtest-message.h \
	googletest/include/gtest/gtest-param-test.h \
	googletest/include/gtest/gtest-spi.h \
	googletest/include/gtest/gtest-test-part.h \
	googletest/include/gtest/gtest-typed-test.h \
	googletest/include/gtest/gtest.h \
	googletest/include/gtest/gtest_pred_impl.h \
	googletest/include/gtest/gtest_prod.h \
	googletest/include/gtest/internal/gtest-death-test-internal.h \
	googletest/include/gtest/internal/gtest-filepath.h \
	googletest/include/gtest/internal/gtest-internal.h \
	googletest/include/gtest/internal/gtest-linked_ptr.h \
	googletest/include/gtest/internal/gtest-param-util-generated.h \
	googletest/include/gtest/internal/gtest-param-util.h \
	googletest/include/gtest/internal/gtest-port.h \
	googletest/include/gtest/internal/gtest-string.h \
	googletest/include/gtest/internal/gtest-tuple.h \
	googletest/include/gtest/internal/gtest-type-util.h \
	googletest/src/gtest-internal-inl.h

libgtest_a_SOURCES = $(gtest_headers) \
	googletest/src/gtest.cc \
	googletest/src/gtest-death-test.cc \
	googletest/src/gtest-filepath.cc \
	googletest/src/gtest-port.cc \
	googletest/src/gtest-test-part.cc \
	googletest/src/gtest-typed-test.cc

libgtest_a_CXXFLAGS = -Igoogletest/include -Igoogletest

libgtest_main_a_SOURCES = $(gtest_headers) \
	googletest/src/gtest_main.cc 


libgtest_main_a_CXXFLAGS = -I$(srcdir)/googletest/include -I$(srcdir)/googletest


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
test_bigfile_LDADD = ../src/libcssc.a


# The death tests may leave a core file behind.  Some systems also use
# core.NNNN as the name of a core file (NNNN is usually the PID).
# Files with names like vgcore.* are produced by valgrind.
COREFILES = core core.* vgcore.*

DISTCLEANFILES = $(COREFILES)
