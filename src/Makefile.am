# Makefile.am: Part of GNU CSSC.
#
# Copyright (C) 1997,1998,1999,2000,2001,
#		2002,2003,2004 Free Software Foundation, Inc.
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
AM_CFLAGS = "-DPREFIX=\"$(csscutildir)/\"" $(generic_CPPFLAGS)

AM_LDFLAGS = -L../gl/lib
LDADD = -lgnulib

AM_CXXFLAGS = $(WARN_CFLAGS)
noinst_LIBRARIES = libcssc.a

bin_PROGRAMS = sccs
csscutil_PROGRAMS = get delta admin prs what unget sact cdc rmdel prt val
csscutil_SCRIPTS = sccsdiff
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

BUILT_SOURCES = version.cc
CLEANFILES = sccsdiff

noinst_HEADERS = cssc.h defaults.h file.h fileiter.h filelock.h \
	filepos.h my-getopt.h linebuf.h mylist.h pfile.h \
	quit.h rel_list.h release.h run.h sccsdate.h sccsfile.h sccsname.h \
	seqstate.h sf-chkmr.h sid.h sid_list.h sl-merge.h sysdep.h \
	version.h ioerr.h delta-table.h delta.h \
	delta-iterator.h bodyio.h filediff.h except.h valcodes.h \
	relvbr.h cssc-assert.h cleanup.h

libcssc_a_SOURCES = \
	rl-merge.cc \
	bodyio.cc sf-get.cc sf-get3.cc \
	seqstate.cc canonify.cc writesubst.cc \
	pfile.cc encoding.cc \
	sf-add.cc sf-kw.cc dtbl-prepend.cc sf-chkid.cc \
	l-split.cc prompt.cc run.cc filediff.cc \
	cap.cc sf-write.cc \
	sccsfile.cc rel_list.cc \
	sccs-delta.cc delta-table.cc delta-iterator.cc \
	fileiter.cc file.cc linebuf.cc \
	sccsname.cc sid.cc sccsdate.cc fnsplit.cc \
	quit.cc fatalsig.cc \
	my-getopt.cc version.cc fdclosed.cc \
	showconfig.cc  environment.cc

# The source lists for the actual executables....
what_SOURCES = what.cc
sact_SOURCES = sact.cc
unget_SOURCES = unget.cc pf-del.cc
prt_SOURCES = prt.cc sf-prt.cc
prs_SOURCES = prs.cc sf-prs.cc sf-chkid.cc
get_SOURCES = get.cc pf-add.cc sf-chkid.cc sf-kw.cc sf-get2.cc pf-del.cc
rmdel_SOURCES = rmdel.cc sf-rmdel.cc
cdc_SOURCES = cdc.cc sf-cdc.cc
admin_SOURCES = admin.cc sf-admin.cc sf-val.cc
delta_SOURCES = delta.cc sf-delta.cc pf-del.cc
val_SOURCES = val.cc sf-val.cc

get_LDADD = libcssc.a $(LIBOBJS)
delta_LDADD = libcssc.a $(LIBOBJS)
admin_LDADD = libcssc.a $(LIBOBJS)
prs_LDADD = libcssc.a $(LIBOBJS)
what_LDADD = libcssc.a $(LIBOBJS)
unget_LDADD = libcssc.a $(LIBOBJS)
sact_LDADD = libcssc.a $(LIBOBJS)
cdc_LDADD = libcssc.a $(LIBOBJS)
rmdel_LDADD = libcssc.a $(LIBOBJS)
prt_LDADD = libcssc.a $(LIBOBJS)
val_LDADD = libcssc.a $(LIBOBJS)
sccs_LDADD = libcssc.a $(LIBOBJS) -lgnulib

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
