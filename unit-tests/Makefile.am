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

# Build rules for libraries.
LDADD = ../googletest/lib/libgtest.la \
	../googletest/lib/libgtest_main.la \
	../libcssc.a

TESTS = $(check_PROGRAMS)
check_PROGRAMS = test_mylist test_myset test_sid

test_mylist_SOURCES = test_mylist.cc
test_myset_SOURCES = test_myset.cc
test_sid_SOURCES = test_sid.cc
