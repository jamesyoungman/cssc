## Process this file with automake to generate "Makefile.in"
##
## Since it doesn't start with "##", the following line will be copied 
## into Makefile.in, and become the first line.
# Running ./configure will generate a Makefile from this file.

csscutildir = $(libexecdir)/cssc

SUBDIRS = docs testutils tests


csscutil_PROGRAMS = get delta admin prs what unget sact cdc rmdel prt
csscutil_SCRIPTS = sccsdiff 

BUILT_SOURCES = version.cc

noinst_HEADERS = cssc.h defaults.h file.h fileiter.h filelock.h \
	filepos.h getopt.h linebuf.h list.h mystring.h pfile.h pipe.h \
	quit.h rel_list.h release.h run.h sccsdate.h sccsfile.h sccsname.h \
	seqstate.h sf-chkmr.h sid.h sid_list.h sl-merge.h stack.h sysdep.h \
	sysnerr.h version.h xalloc.h 

common_SOURCES = quit.cc getopt.cc version.cc

sfile_SOURCES = fileiter.cc file.cc linebuf.cc mystring.cc xalloc.cc \
		sccsname.cc sid.cc sccsdate.cc split.cc

what_SOURCES = what.cc $(common_SOURCES)
get_SOURCES = get.cc pf-add.cc pfile.cc sccsfile.cc rel_list.cc \
	sf-chkid.cc sf-get.cc sf-get2.cc sf-get3.cc rl-merge.cc \
	canonify.cc sf-kw.cc $(common_SOURCES) $(sfile_SOURCES)

delta_SOURCES = delta.cc \
	l-split.cc pf-del.cc pfile.cc pipe.cc prompt.cc run.cc sccsfile.cc \
	 rel_list.cc sf-add.cc sf-chkid.cc sf-delta.cc sf-get.cc sf-get3.cc \
	sf-write.cc sf-kw.cc  $(common_SOURCES) $(sfile_SOURCES)

admin_SOURCES = admin.cc l-split.cc prompt.cc run.cc sccsfile.cc sf-add.cc \
	sf-admin.cc sf-chkid.cc sf-write.cc rel_list.cc rl-merge.cc \
	sf-kw.cc $(common_SOURCES) $(sfile_SOURCES)

prs_SOURCES = prs.cc sccsfile.cc rel_list.cc sf-chkid.cc sf-get.cc sf-prs.cc \
	$(common_SOURCES) $(sfile_SOURCES)

prt_SOURCES = prt.cc sf-prt.cc sccsfile.cc rel_list.cc \
	$(common_SOURCES) $(sfile_SOURCES)

unget_SOURCES = unget.cc pf-del.cc pfile.cc  $(common_SOURCES) $(sfile_SOURCES)

sact_SOURCES = sact.cc pfile.cc $(common_SOURCES) $(sfile_SOURCES)

cdc_SOURCES = cdc.cc sccsfile.cc rel_list.cc sf-cdc.cc \
	sf-write.cc run.cc l-split.cc prompt.cc \
	$(common_SOURCES) $(sfile_SOURCES)

rmdel_SOURCES = rmdel.cc sccsfile.cc rel_list.cc sf-rmdel.cc \
	sf-write.cc pfile.cc $(common_SOURCES) $(sfile_SOURCES)



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

# $(testcode) is test code that isn't normally used.
testcode = list.cc 

# rainyday_src is code that we don't currently need, and would be
# in the "unused" directory but might be needed for a port (e.g. 
# to DOS or something).
rainyday_src = dummy.cc fsync.cc _chmod.cc sid_list.cc \
	move-if-change 

EXTRA_DIST = ChangeLog $(testcode) $(rainyday_src)


sccsdiff: sccsdiff.sh
	cp $< $@
	chmod a+x $@
