TESTDIRS = admin delta get prs prt unget large sccsdiff
MKDIR = mkdir

links: $(TESTDIRS)
	../testutils/lndir $(srcdir) 

common: 
	ln -s $(srcdir)/common .

test-admin: links
	$(SHELL) -ec 'cd admin && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'


test-delta: links
	$(SHELL) -ec 'cd delta && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-get: links
	$(SHELL) -ec 'cd get && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-prs: links
	$(SHELL) -ec 'cd prs && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-prt: links
	$(SHELL) -ec 'cd prt && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-unget: links
	$(SHELL) -ec 'cd unget && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-large: links
	$(SHELL) -ec 'cd large && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-sccsdiff: links
	$(SHELL) -ec 'cd sccsdiff && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

all-tests: 	test-admin test-delta test-get test-prs test-prt test-unget \
		test-large test-sccsdiff
	echo Tests passed.

check: all-tests

# When a distribution is being made and all the files that automake
# knows about have been copied into $(distdir), dist-hook is called.
dist-hook: 
	( cd $(srcdir) && find common $(TESTDIRS) -type l -print| xargs $(RM) )
	( cd $(srcdir)  && tar cf - common $(TESTDIRS) ) | \
	( cd $(distdir) && tar xf - )


