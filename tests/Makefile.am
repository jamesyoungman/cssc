TESTDIRS = admin delta get prs prt unget
SUBDIRS = $(TESTDIRS)

MKDIR = mkdir

links: $(TESTDIRS)
	../testutils/lndir $(srcdir) 

common: 
	ln -s $(srcdir)/common .

test-admin: links
	$(SHELL) -ec 'cd admin && for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'


test-delta: links
	$(SHELL) -ec 'cd delta && for i in *.sh ; \
		do $(SHELL) $$i || exit 1; \
	done'

test-get: links
	$(SHELL) -ec 'cd get && for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'

test-prs: links
	$(SHELL) -ec 'cd prs && for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'

test-prt: links
	$(SHELL) -ec 'cd prt && for i in *.sh ; \
		do $(SHELL) $$i || exit 1; \
	done'

test-unget: links
	$(SHELL) -ec 'cd unget && for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'

all-tests: test-admin test-delta test-get test-prs test-prt test-unget

check: all-tests

# Some magic which generates the Makefile.am files for the
# subdirectories.  Basically, we just add all the contents of 
# the subdirectories, except symbolic links and directories.
#
sub-automake:
	cd $(srcdir)
	for  i in $(TESTDIRS) ; \
	do \
		( cd $$i ; \
		rm $$i/Makefile.am \ \
		echo "EXTRA_DIST ="$$(\
		for i in * ; \
		do \
			[ ! -d $$i -a ! -L $$i ] && echo $$i; \
		done );\
		) > $$i/Makefile.am ; \
	done

