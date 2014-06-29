# We define the global AM_CPPFLAGS as everything we compile includes from these
# directories.
GTEST_DIR = $(srcdir)/googletest/
AM_CPPFLAGS =  -I$(GTEST_DIR)/include -pthread -I$(srcdir)/../src
# Modifies compiler and linker flags for pthreads compatibility.
AM_LIBS =
if HAVE_PTHREADS
  AM_CXXFLAGS = @PTHREAD_CFLAGS@ -DGTEST_HAS_PTHREAD=1
  AM_LIBS += @PTHREAD_LIBS@
else
  AM_CXXFLAGS = -DGTEST_HAS_PTHREAD=0
endif

LDADD = googletest/lib/libgtest.la googletest/lib/libgtest_main.la ../src/libcssc.a


# We do not include googletest in SUBDIRS since the Makefile.am file
# in thart directory forces "make install" to fail, because it doesn't
# want to install the built libraries.  We on the other hand just
# avoid installing them by using noinst_.  This means that the
# googletest targets are built by rules in this Makefile.am.

EXTRA_DIST = TODO.txt

# Build rules for libraries.
TESTS_ENVIRONMENT = GTEST_SOURCE_DIR="$(srcdir)/test" \
                    GTEST_BUILD_DIR="$(top_builddir)/test" \
		    sh testwrapper.sh
TESTS = $(unit_tests)
check_SCRIPTS = testwrapper.sh
unit_tests = test_mylist test_myset test_sid test_relvbr \
	test_release test_sid_list test_rel_list test_sccsdate \
	test_delta test_delta-table test_encoding \
	test_encoding2 test_linebuf
noinst_LTLIBRARIES = googletest/lib/libgtest.la googletest/lib/libgtest_main.la

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
test_delta_table_SOURCES = test_delta-table.cc
test_encoding_SOURCES = test_encoding.cc
test_encoding2_SOURCES = test_encoding2.cc
test_linebuf_SOURCES = test_linebuf.cc
test_bigfile_SOURCES = test_bigfile.cc
test_bigfile_LDADD = ../src/libcssc.a


# The death tests may leave a core file behind.  Some systems also use
# core.NNNN as the name of a core file (NNNN is usually the PID).
# Files with names like vgcore.* are produced by valgrind.
COREFILES = core core.* vgcore.*

DISTCLEANFILES = $(COREFILES)


##############################################################################
##############################################################################
##                       googletest rules
##############################################################################
##############################################################################

ACLOCAL_AMFLAGS = -I m4

# Nonstandard package files for distribution
EXTRA_DIST += \
  googletest/CHANGES \
  googletest/CONTRIBUTORS \
  googletest/LICENSE \
  googletest/include/gtest/gtest-param-test.h.pump \
  googletest/include/gtest/internal/gtest-param-util-generated.h.pump \
  googletest/include/gtest/internal/gtest-tuple.h.pump \
  googletest/include/gtest/internal/gtest-type-util.h.pump \
  googletest/make/Makefile \
  googletest/scripts/fuse_gtest_files.py \
  googletest/scripts/gen_gtest_pred_impl.py \
  googletest/scripts/pump.py \
  googletest/scripts/test/Makefile

# Sample files that we don't compile.
EXTRA_DIST += \
  googletest/samples/prime_tables.h \
  googletest/samples/sample1_unittest.cc \
  googletest/samples/sample2_unittest.cc \
  googletest/samples/sample3_unittest.cc \
  googletest/samples/sample4_unittest.cc \
  googletest/samples/sample5_unittest.cc \
  googletest/samples/sample6_unittest.cc \
  googletest/samples/sample7_unittest.cc \
  googletest/samples/sample8_unittest.cc \
  googletest/samples/sample9_unittest.cc \
  googletest/samples/sample10_unittest.cc \
  googletest/test/gtest_all_test.cc

# C++ test files that we don't compile directly.
EXTRA_DIST += \
  googletest/test/gtest-death-test_ex_test.cc \
  googletest/test/gtest-death-test_test.cc \
  googletest/test/gtest-filepath_test.cc \
  googletest/test/gtest-linked_ptr_test.cc \
  googletest/test/gtest-listener_test.cc \
  googletest/test/gtest-message_test.cc \
  googletest/test/gtest-options_test.cc \
  googletest/test/gtest-param-test2_test.cc \
  googletest/test/gtest-param-test2_test.cc \
  googletest/test/gtest-param-test_test.cc \
  googletest/test/gtest-param-test_test.cc \
  googletest/test/gtest-param-test_test.h \
  googletest/test/gtest-port_test.cc \
  googletest/test/gtest_premature_exit_test.cc \
  googletest/test/gtest-printers_test.cc \
  googletest/test/gtest-test-part_test.cc \
  googletest/test/gtest-tuple_test.cc \
  googletest/test/gtest-typed-test2_test.cc \
  googletest/test/gtest-typed-test_test.cc \
  googletest/test/gtest-typed-test_test.h \
  googletest/test/gtest-unittest-api_test.cc \
  googletest/test/gtest_break_on_failure_unittest_.cc \
  googletest/test/gtest_catch_exceptions_test_.cc \
  googletest/test/gtest_color_test_.cc \
  googletest/test/gtest_env_var_test_.cc \
  googletest/test/gtest_environment_test.cc \
  googletest/test/gtest_filter_unittest_.cc \
  googletest/test/gtest_help_test_.cc \
  googletest/test/gtest_list_tests_unittest_.cc \
  googletest/test/gtest_main_unittest.cc \
  googletest/test/gtest_no_test_unittest.cc \
  googletest/test/gtest_output_test_.cc \
  googletest/test/gtest_pred_impl_unittest.cc \
  googletest/test/gtest_prod_test.cc \
  googletest/test/gtest_repeat_test.cc \
  googletest/test/gtest_shuffle_test_.cc \
  googletest/test/gtest_sole_header_test.cc \
  googletest/test/gtest_stress_test.cc \
  googletest/test/gtest_throw_on_failure_ex_test.cc \
  googletest/test/gtest_throw_on_failure_test_.cc \
  googletest/test/gtest_uninitialized_test_.cc \
  googletest/test/gtest_unittest.cc \
  googletest/test/gtest_unittest.cc \
  googletest/test/gtest_xml_outfile1_test_.cc \
  googletest/test/gtest_xml_outfile2_test_.cc \
  googletest/test/gtest_xml_output_unittest_.cc \
  googletest/test/production.cc \
  googletest/test/production.h

# Python tests that we don't run.
EXTRA_DIST += \
  googletest/test/gtest_break_on_failure_unittest.py \
  googletest/test/gtest_catch_exceptions_test.py \
  googletest/test/gtest_color_test.py \
  googletest/test/gtest_env_var_test.py \
  googletest/test/gtest_filter_unittest.py \
  googletest/test/gtest_help_test.py \
  googletest/test/gtest_list_tests_unittest.py \
  googletest/test/gtest_output_test.py \
  googletest/test/gtest_output_test_golden_lin.txt \
  googletest/test/gtest_shuffle_test.py \
  googletest/test/gtest_test_utils.py \
  googletest/test/gtest_throw_on_failure_test.py \
  googletest/test/gtest_uninitialized_test.py \
  googletest/test/gtest_xml_outfiles_test.py \
  googletest/test/gtest_xml_output_unittest.py \
  googletest/test/gtest_xml_test_utils.py

# CMake script
EXTRA_DIST += \
  googletest/CMakeLists.txt \
  googletest/cmake/internal_utils.cmake

# MSVC project files
EXTRA_DIST += \
  googletest/msvc/gtest-md.sln \
  googletest/msvc/gtest-md.vcproj \
  googletest/msvc/gtest.sln \
  googletest/msvc/gtest.vcproj \
  googletest/msvc/gtest_main-md.vcproj \
  googletest/msvc/gtest_main.vcproj \
  googletest/msvc/gtest_prod_test-md.vcproj \
  googletest/msvc/gtest_prod_test.vcproj \
  googletest/msvc/gtest_unittest-md.vcproj \
  googletest/msvc/gtest_unittest.vcproj

# xcode project files
EXTRA_DIST += \
  googletest/xcode/Config/DebugProject.xcconfig \
  googletest/xcode/Config/FrameworkTarget.xcconfig \
  googletest/xcode/Config/General.xcconfig \
  googletest/xcode/Config/ReleaseProject.xcconfig \
  googletest/xcode/Config/StaticLibraryTarget.xcconfig \
  googletest/xcode/Config/TestTarget.xcconfig \
  googletest/xcode/Resources/Info.plist \
  googletest/xcode/Scripts/runtests.sh \
  googletest/xcode/Scripts/versiongenerate.py \
  googletest/xcode/gtest.xcodeproj/project.pbxproj

# xcode sample files
EXTRA_DIST += \
  googletest/xcode/Samples/FrameworkSample/Info.plist \
  googletest/xcode/Samples/FrameworkSample/WidgetFramework.xcodeproj/project.pbxproj \
  googletest/xcode/Samples/FrameworkSample/runtests.sh \
  googletest/xcode/Samples/FrameworkSample/widget.cc \
  googletest/xcode/Samples/FrameworkSample/widget.h \
  googletest/xcode/Samples/FrameworkSample/widget_test.cc

# C++Builder project files
EXTRA_DIST += \
  googletest/codegear/gtest.cbproj \
  googletest/codegear/gtest.groupproj \
  googletest/codegear/gtest_all.cc \
  googletest/codegear/gtest_link.cc \
  googletest/codegear/gtest_main.cbproj \
  googletest/codegear/gtest_unittest.cbproj

googletest_lib_libgtest_la_SOURCES = \
	googletest/src/gtest.cc \
	googletest/src/gtest-death-test.cc \
	googletest/src/gtest-filepath.cc \
	googletest/src/gtest-port.cc \
	googletest/src/gtest-printers.cc \
	googletest/src/gtest-test-part.cc \
	googletest/src/gtest-typed-test.cc \
	googletest/src/gtest-internal-inl.h

# We don't compile this, we build libgtest from the sources listed above.
EXTRA_DIST += \
	googletest/src/gtest-all.cc


googletest_lib_libgtest_la_CPPFLAGS = -I$(GTEST_DIR)/src -I$(GTEST_DIR)/include -I$(GTEST_DIR)
googletest_lib_libgtest_main_la_SOURCES = googletest/src/gtest_main.cc

googletest_lib_libgtest_main_la_LIBADD = googletest/lib/libgtest.la

noinst_HEADERS = \
  googletest/include/gtest/gtest-death-test.h \
  googletest/include/gtest/gtest-message.h \
  googletest/include/gtest/gtest-param-test.h \
  googletest/include/gtest/gtest-printers.h \
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
  googletest/include/gtest/internal/gtest-type-util.h

# Bulid rules for samples and tests. Automake's naming for some of
# these variables isn't terribly obvious, so this is a brief
# reference:
#
# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run

EXTRA_DIST += \
  googletest/samples/sample1.cc \
  googletest/samples/sample1.h \
  googletest/samples/sample2.cc \
  googletest/samples/sample2.h \
  googletest/samples/sample3-inl.h \
  googletest/samples/sample4.cc \
  googletest/samples/sample4.h


# Some more files which need to be distributed but which are otherwise
# not referenced (so that we have to tell Automake to include them in
# the release).
# configure.ac isn't seen by the Automake rules because we (deliberately) don't
# add googletest to SUBDIRS (since otherwise it will make "make install" fail).
#
# FIXME: we don't include the rules here to regenerate the fused test sources (though we ship
# the original fused test sources).
EXTRA_DIST += \
	googletest/configure.ac \
	googletest/m4/gtest.m4 \
	googletest/Makefile.am \
	googletest/Makefile.in \
	googletest/README \
	googletest/scripts/common.py \
	googletest/scripts/gtest-config.in \
	googletest/scripts/release_docs.py \
	googletest/scripts/upload_gtest.py \
	googletest/scripts/upload.py \
	googletest/xcode/Samples/FrameworkSample/WidgetFramework.xcodeproj/project.pbxproj \
	googletest/build-aux/.keep
