## Process this file with automake to generate "Makefile.in"
##
## Since it doesn't start with "##", the following line will be copied 
## into Makefile.in, and become the first line.
# Running ./configure will generate a Makefile from this file.

csscutildir = $(libexecdir)/cssc

# We keep "tests" as the last named subdirectory in SUBDIRS,
# so that when we do "make check", the "Tests passed" message
# is emitted last (we wouldn't want "entering directory unused"
# messages to make that scroll offf the screen).
SUBDIRS = bsd docs testutils auxfiles unused tests


csscutil_PROGRAMS = get delta admin prs what unget sact cdc rmdel prt val
csscutil_SCRIPTS = sccsdiff 

BUILT_SOURCES = version.cc

noinst_HEADERS = cssc.h defaults.h file.h fileiter.h filelock.h \
	filepos.h my-getopt.h linebuf.h mylist.h mystring.h pfile.h \
	quit.h rel_list.h release.h run.h sccsdate.h sccsfile.h sccsname.h \
	seqstate.h sf-chkmr.h sid.h sid_list.h sl-merge.h stack.h sysdep.h \
	sysnerr.h version.h ioerr.h err_no.h delta-table.h delta.h \
	delta-iterator.h bodyio.h filediff.h except.h valcodes.h \
	relvbr.h

# Sources required by everybody.
common_SOURCES = my-getopt.cc version.cc fdclosed.cc showconfig.cc

# Sources required by programs working with lists of SCCS files.
sname_SOURCES = fileiter.cc file.cc linebuf.cc mystring.cc \
		sccsname.cc sid.cc sccsdate.cc split.cc fnsplit.cc \
		quit.cc fatalsig.cc

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
	seqstate.cc \
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES)


get_SOURCES = get.cc pf-add.cc pfile.cc sf-chkid.cc canonify.cc sf-kw.cc \
	sf-get.cc sf-get2.cc sf-get3.cc rl-merge.cc \
	bodyio.cc encoding.cc \
	seqstate.cc \
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
	seqstate.cc \
	$(moddelta_SOURCES) $(newdelta_SOURCES)\
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES) $(mod_SOURCES)

val_SOURCES = val.cc sf-val.cc \
	$(common_SOURCES) $(sname_SOURCES) $(sfile_SOURCES)

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
	cp sccsdiff.sh $@
	chmod a+x $@

testsplit: fnsplit.cc mystring.o quit.o
	$(CXXCOMPILE) -DTEST_FNSPLIT -o testsplit \
		$(srcdir)/fnsplit.cc mystring.o quit.o
