TESTDIRS =  cdc admin delta get prs prt unget large sccsdiff binary rmdel \
		bsd-sccs year-2000 initial what val
MKDIR = mkdir
VALGRIND = @VALGRIND@
export VALGRIND

links: $(TESTDIRS)
	../testutils/lndir $(srcdir) 

common: 
	ln -s $(srcdir)/common .

test-initial: links
	$(SHELL) -ec 'cd initial && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'


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

test-sact: links
	$(SHELL) -ec 'cd sact && for i in *.sh ; \
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

test-cdc: links
	$(SHELL) -ec 'cd cdc && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-binary: links
	$(SHELL) -ec 'cd binary && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-rmdel: links
	$(SHELL) -ec 'cd rmdel && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-what: links
	$(SHELL) -ec 'cd what && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-val: links
	$(SHELL) -ec 'cd val && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

test-bsd-sccs: links
	$(SHELL) -ec 'cd bsd-sccs && for i in *.sh ; \
	do \
		echo Running test $$i; \
		$(SHELL) $$i || exit 1; \
	done'

# Always do test-initial FIRST.
all-tests: 	test-initial \
		test-rmdel \
		test-admin test-delta test-get test-prs test-prt test-unget \
		test-cdc  test-sact test-val \
		test-large test-sccsdiff test-binary test-bsd-sccs test-what
	echo Tests passed.

check: all-tests

# When a distribution is being made and all the files that automake
# knows about have been copied into $(distdir), dist-hook is called.

# This old code copied the CVS directories too, which is awkward
# for anybody wanting to import the distribution into a local CVS 
# repository.
#
# dist-hook: 
# 	  ( cd $(srcdir) && find common $(TESTDIRS) -type l -print | \
#		xargs $(RM) )
# 	  ( cd $(srcdir)  && tar cf - common $(TESTDIRS) ) | \
# 	  ( cd $(distdir) && tar xf - )
# 
dist-hook: 
	( cd $(srcdir) && find common $(TESTDIRS) -type l -print | \
		xargs $(RM) )
	( cd $(srcdir) && tar cf - \
		$$( find . \( -type d -name CVS -prune \) -o \
		-type f -print ) ) | \
	( cd $(distdir) && tar xfv - ) ; true
