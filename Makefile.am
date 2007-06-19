# Makefile.am: Part of GNU CSSC.
#
# Copyright (C) 1997,1998,1999,2000,2001,
#		2002,2003,2004 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
# USA.
#
## Process this file with automake to generate "Makefile.in"
##
## Since it doesn't start with "##", the following line will be copied 
## into Makefile.in, and become the first line.
# Running ./configure will generate a Makefile from this file.

csscutildir = $(libexecdir)/cssc

# We keep "tests" as the last named subdirectory in SUBDIRS,
# so that when we do "make check", the "Tests passed" message
# is emitted last (we wouldn't want "entering directory unused"
# messages to make that scroll off the screen).
SUBDIRS = bsd docs testutils auxfiles sccs-cgi tests 


csscutil_PROGRAMS = get delta admin prs what unget sact cdc rmdel prt val
csscutil_SCRIPTS = sccsdiff 

BUILT_SOURCES = version.cc
CLEANFILES = sccsdiff

noinst_HEADERS = cssc.h defaults.h file.h fileiter.h filelock.h \
	filepos.h my-getopt.h linebuf.h mylist.h mystring.h pfile.h \
	quit.h rel_list.h release.h run.h sccsdate.h sccsfile.h sccsname.h \
	seqstate.h sf-chkmr.h sid.h sid_list.h sl-merge.h stack.h sysdep.h \
	sysnerr.h version.h ioerr.h err_no.h delta-table.h delta.h \
	delta-iterator.h bodyio.h filediff.h except.h valcodes.h \
	relvbr.h myset.h

# Sources required by everybody.
common_sources = my-getopt.cc version.cc fdclosed.cc \
		showconfig.cc  environment.cc

# Sources required by programs working with lists of SCCS files.
sname_srcs = fileiter.cc file.cc linebuf.cc mystring.cc \
		sccsname.cc sid.cc sccsdate.cc split.cc fnsplit.cc \
		quit.cc fatalsig.cc

# Sources required by programs working with the contents of SCCS files.
sfile_srcs = sccsfile.cc rel_list.cc \
	sccs-delta.cc delta-table.cc delta-iterator.cc 


# Source files required by programs that modify SCCS files.
mod_srcs = cap.cc sf-write.cc

# Source files required by programs that modify delta entries.
moddelta_srcs = l-split.cc prompt.cc run.cc filediff.cc

# Source files required by programs that add new delta entries.
newdelta_srcs = sf-add.cc sf-kw.cc dtbl-prepend.cc sf-chkid.cc


# The source lists for the actual executables....


what_SOURCES = what.cc \
	$(common_sources)
sact_SOURCES = sact.cc pfile.cc \
	$(common_sources) $(sname_srcs)
unget_SOURCES = unget.cc pfile.cc pf-del.cc \
	$(common_sources) $(sname_srcs)

prt_SOURCES = prt.cc sf-prt.cc \
	$(common_sources) $(sname_srcs) $(sfile_srcs)
prs_SOURCES = prs.cc sf-prs.cc sf-chkid.cc sf-get.cc writesubst.cc \
	bodyio.cc encoding.cc canonify.cc \
	seqstate.cc sf-get3.cc \
	$(common_sources) $(sname_srcs) $(sfile_srcs)


get_SOURCES = get.cc pf-add.cc pfile.cc sf-chkid.cc canonify.cc sf-kw.cc \
	sf-get.cc sf-get2.cc writesubst.cc sf-get3.cc rl-merge.cc \
	bodyio.cc encoding.cc \
	seqstate.cc pf-del.cc \
	$(common_sources) $(sname_srcs) $(sfile_srcs)

rmdel_SOURCES = rmdel.cc sf-rmdel.cc \
	pfile.cc \
	$(common_sources) $(sname_srcs) $(sfile_srcs) $(mod_srcs)

cdc_SOURCES = cdc.cc sf-cdc.cc \
	$(moddelta_srcs) \
	$(common_sources) $(sname_srcs) $(sfile_srcs) $(mod_srcs)

admin_SOURCES = admin.cc sf-admin.cc \
	rl-merge.cc \
	bodyio.cc encoding.cc sf-val.cc \
	$(moddelta_srcs) $(newdelta_srcs)\
	$(common_sources) $(sname_srcs) $(sfile_srcs) $(mod_srcs)

delta_SOURCES = delta.cc sf-delta.cc \
	pf-del.cc pfile.cc sf-get.cc writesubst.cc sf-get3.cc \
	bodyio.cc encoding.cc \
	seqstate.cc canonify.cc \
	$(moddelta_srcs) $(newdelta_srcs)\
	$(common_sources) $(sname_srcs) $(sfile_srcs) $(mod_srcs)

val_SOURCES = val.cc sf-val.cc \
	$(common_sources) $(sname_srcs) $(sfile_srcs)

get_LDADD = @LIBOBJS@
delta_LDADD = @LIBOBJS@
admin_LDADD = @LIBOBJS@
prs_LDADD = @LIBOBJS@
what_LDADD = @LIBOBJS@
unget_LDADD = @LIBOBJS@
sact_LDADD = @LIBOBJS@
cdc_LDADD = @LIBOBJS@
rmdel_LDADD = @LIBOBJS@
prt_LDADD = @LIBOBJS@
val_LDADD = @LIBOBJS@

# $(testcode) is test code that isn't normally used.
testcode = list.cc 

EXTRA_DIST = ChangeLog ChangeLog.1 $(testcode) show-disp.sh


sccsdiff: sccsdiff.sh
	sed -e 's:__csscutildir__:$(csscutildir):g' < sccsdiff.sh > $@ && chmod a+x $@ || { rm -f $@; false; }

testsplit$(EXEEXT): fnsplit.cc mystring.$(OBJEXT) quit.$(OBJEXT)
	$(CXXCOMPILE) -DTEST_FNSPLIT -o testsplit$(EXEEXT) \
		$(srcdir)/fnsplit.cc mystring.$(OBJEXT) quit.$(OBJEXT)
