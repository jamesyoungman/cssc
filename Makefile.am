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
	filepos.h my-getopt.h linebuf.h mylist.h mystring.h pfile.h pipe.h \
	quit.h rel_list.h release.h run.h sccsdate.h sccsfile.h sccsname.h \
	seqstate.h sf-chkmr.h sid.h sid_list.h sl-merge.h stack.h sysdep.h \
	sysnerr.h version.h xalloc.h ioerr.h err_no.h delta-table.h delta.h \
	delta-iterator.h bodyio.h filediff.h

# Sources required by everybody.
common_SOURCES = my-getopt.cc version.cc

# Sources required by programs working with lists of SCCS files.
sname_SOURCES = fileiter.cc file.cc linebuf.cc mystring.cc xalloc.cc \
		sccsname.cc sid.cc sccsdate.cc split.cc fnsplit.cc \
		quit.cc 

# Sources required by programs working with the contents of SCCS files.
sfile_SOURCES = sccsfile.cc rel_list.cc \
	sccs-delta.cc delta-table.cc delta-iterator.cc 


# Source files required by programs that modify SCCS files.
mod_SOURCES = cap.cc sf-write.cc

# Source files required by programs that modify delta entries.
moddelta_SOURCES = l-split.cc prompt.cc run.cc filediff.cc

# Source files required by programs that add new delta entries.
newdelta_SOURCES = sf-add.cc sf-kw.cc dtbl-prepend.cc sf-chkid.cc


# The source lists for the actual executables....


what_SOURCES = what.cc \
	$(common_SOURCES)
sact_SOURCES = sact.cc pfile.cc \
	$(common_SOURCES) $(sname_SOURCES)
unget_SOURCES = unget.cc pfile.cc pf-del.cc \
	$(common_SOURCES) $(sname_SOURCES)

prt_SOURCES = prt.cc sf-prt.cc \
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES)
prs_SOURCES = prs.cc sf-prs.cc sf-chkid.cc sf-get.cc \
	bodyio.cc encoding.cc canonify.cc \
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES)


get_SOURCES = get.cc pf-add.cc pfile.cc sf-chkid.cc canonify.cc sf-kw.cc \
	sf-get.cc sf-get2.cc sf-get3.cc rl-merge.cc \
	bodyio.cc encoding.cc \
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES)

rmdel_SOURCES = rmdel.cc sf-rmdel.cc \
	pfile.cc \
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES) $(mod_SOURCES)

cdc_SOURCES = cdc.cc sf-cdc.cc \
	$(moddelta_SOURCES) \
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES) $(mod_SOURCES)

admin_SOURCES = admin.cc sf-admin.cc \
	rl-merge.cc \
	bodyio.cc encoding.cc \
	$(moddelta_SOURCES) $(newdelta_SOURCES)\
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES) $(mod_SOURCES)

delta_SOURCES = delta.cc sf-delta.cc \
	pf-del.cc pfile.cc sf-get.cc sf-get3.cc \
	bodyio.cc encoding.cc \
	$(moddelta_SOURCES) $(newdelta_SOURCES)\
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES) $(mod_SOURCES)


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
rainyday_src = dummy.cc fsync.cc _chmod.cc sid_list.cc pipe.h pipe.cc \
	move-if-change 

EXTRA_DIST = ChangeLog $(testcode) $(rainyday_src) show-disp.sh


sccsdiff: sccsdiff.sh
	cp sccsdiff.sh $@
	chmod a+x $@

testsplit: fnsplit.cc mystring.o quit.o
	$(CXXCOMPILE) -DTEST_FNSPLIT -o testsplit \
		$(srcdir)/fnsplit.cc mystring.o quit.o
