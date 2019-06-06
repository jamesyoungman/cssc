# Makefile.am: Part of GNU CSSC.
#
# Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2019
# Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
## Process this file with automake to generate "Makefile.in"
##
## Since it doesn't start with "##", the following line will be copied
## into Makefile.in, and become the first line.
# Running ./configure will generate a Makefile from this file.

csscutildir = $(libexecdir)/cssc

ACLOCAL_AMFLAGS = -I m4
generic_CPPFLAGS = -I ../gl/lib -I $(srcdir)/../gl/lib
AM_CPPFLAGS = $(generic_CPPFLAGS)
AM_CFLAGS = "-DPREFIX=\"$(csscutildir)/\"" "-DLOCALEDIR=\"$(localedir)\"" $(generic_CPPFLAGS)

AM_LDFLAGS = -L../gl/lib
LDADD = libcssc.a -lgnulib

AM_CXXFLAGS = $(WARN_CXXFLAGS)
noinst_LIBRARIES = libcssc.a

bin_PROGRAMS = sccs
csscutil_PROGRAMS = get delta admin prs what unget sact cdc rmdel prt val
csscutil_SCRIPTS = sccsdiff
noinst_SCRIPTS = copyright.awk

# ../configure.ac specifies gnits rules, but we don't actually implement the
# --version and --help options.
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = \
	admin$(EXE)  \
	cdc$(EXE)    \
	delta$(EXE)  \
	get$(EXE)    \
	prs$(EXE)    \
	prt$(EXE)    \
	rmdel$(EXE)  \
	sact$(EXE)   \
	sccs$(EXE)   \
	sccsdiff     \
	unget$(EXE)  \
	val$(EXE)    \
	what$(EXE)

BUILT_SOURCES = version.cc copyright_data.inc
CLEANFILES = sccsdiff copyright_data.inc

libcssc_a_SOURCES = \
	base-reader.cc \
	base-reader.h \
	body-scanner.cc \
	body-scanner.h \
	bodyio.cc \
	bodyio.h \
	canonify.cc \
	cap.cc \
	cleanup.h \
	copyright.cc \
	cssc-assert.h \
	cssc.h \
	defaults.h \
	delta-iterator.cc \
	delta-iterator.h \
	delta-table.cc \
	delta-table.h \
	delta.h \
	diff-state.cc \
	diff-state.h \
	dtbl-prepend.cc \
	encoding.cc \
	environment.cc \
	except.h \
	failure.cc \
	failure.h \
	failure_macros.h \
	failure_or.h \
	fatalsig.cc \
	fdclosed.cc \
	file.cc \
	file.h \
	filediff.cc \
	filediff.h \
	fileiter.cc \
	fileiter.h \
	filelock.h \
	filepos.h \
	fnsplit.cc \
	ioerr.h \
	l-split.cc \
	l-split.h \
	linebuf.cc \
	linebuf.h \
	location.cc \
	location.h \
	mode.h \
	my-getopt.cc \
	my-getopt.h \
	optional.h \
	parser.cc \
	parser.h \
	pf-add.cc \
	pf-del.cc \
	pfile.cc \
	pfile.h \
	privs.cc \
	privs.h \
	prompt.cc \
	quit.cc \
	quit.h \
	rel_list.cc \
	rel_list.h \
	release.h \
	release.cc \
	relvbr.h \
	relvbr.cc \
	rl-merge.cc \
	run.cc \
	sccs-delta.cc \
	sccsdate.cc \
	sccsdate.h \
	sccsfile.cc \
	sccsfile.h \
	sccsname.cc \
	sccsname.h \
	seqstate.cc \
	seqstate.h \
	sf-add.cc \
	sf-admin.cc \
	sf-cdc.cc \
	sf-chkid.cc \
	sf-delta.cc \
	sf-get.cc \
	sf-get2.cc \
	sf-get3.cc \
	sf-kw.cc \
	sf-prs.cc \
	sf-prt.cc \
	sf-rmdel.cc \
	sf-val.cc \
	sf-write.cc \
	showconfig.cc \
	sid.cc \
	sid.h \
	sid_list.h \
	sl-merge.h \
	stringify.h \
	subst-parms.h \
	sysdep.h \
	valcodes.h \
	version.cc \
	version.h \
	writesubst.cc

nodist_libcssc_a_SOURCES = copyright_data.inc
EXTRA_DIST = copyright_data.txt copyright_data.inc.tmpl copyright.awk

# The source lists for the actual executables.  We list these explicitly because
# otherwise Automake will assume that prt$(EXE) is built from prt.c.
what_SOURCES = what.cc
sact_SOURCES = sact.cc
unget_SOURCES = unget.cc
prt_SOURCES = prt.cc
prs_SOURCES = prs.cc
get_SOURCES = get.cc
rmdel_SOURCES = rmdel.cc
cdc_SOURCES = cdc.cc
admin_SOURCES = admin.cc
delta_SOURCES = delta.cc
val_SOURCES = val.cc

# We explicitly list the dependency on copyright_data.inc, so that
# targets get rebuilt when we re-generate copyright_data.inc.
copyright.$(OBJEXT): copyright_data.inc

copyright_data.inc: $(srcdir)/copyright_data.txt $(srcdir)/copyright_data.inc.tmpl Makefile
	$(AWK) -vtext=$(srcdir)/copyright_data.txt -v outputfile="$@" -f $(srcdir)/copyright.awk  < $(srcdir)/copyright_data.inc.tmpl > $@

sccsdiff: sccsdiff.sh
	sed -e 's:__csscutildir__:$(csscutildir):g' < sccsdiff.sh > $@ && chmod a+x $@ || { rm -f $@; false; }

check-include-order:
	rv=0; \
	for f in $(srcdir)/*.cc; \
	do \
		msg="`sed -n -f $(top_srcdir)/auxfiles/firstinclude.sed < $$f`"; \
		if test -n "$${msg}"; \
		then \
			set -- $${msg} ; line=$${1} ; shift; errmsg="$${*}"; \
			echo "$${f}:$${line}:$${errmsg}" >&2; \
			rv=1; \
		fi \
	done; \
	exit $${rv}

testsplit$(EXEEXT): fnsplit.cc quit.$(OBJEXT)
	$(CXXCOMPILE) -DTEST_FNSPLIT -o testsplit$(EXEEXT) \
		$(srcdir)/fnsplit.cc quit.$(OBJEXT)
