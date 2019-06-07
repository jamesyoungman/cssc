TESTDIRS =  cdc admin delta get prs prt unget large sccsdiff binary rmdel \
                bsd-sccs year-2000 initial what val
TESTFILE_SUFFIXES = .sh
MKDIR = mkdir


remove_trash:
	trash="`find . -name 'z.*' -print`"; echo "$${trash}" >&2 ; test -z "$${trash}" || exit 1

intro:
	@echo "Valgrind is: $(VALGRIND)"

links: $(TESTDIRS) remove_trash intro
	../testutils/lndir $(srcdir) 

common: 
	ln -s $(srcdir)/common .

prepare: links intro

test-initial: prepare
	@$(PYTHON) run_tests.py initial

test-admin: prepare
	@$(PYTHON) run_tests.py admin

test-delta: prepare
	@$(PYTHON) run_tests.py delta

test-get: prepare
	@$(PYTHON) run_tests.py get

test-prs: prepare
	@$(PYTHON) run_tests.py prs

test-prt: prepare
	@$(PYTHON) run_tests.py prt

test-unget: prepare
	@$(PYTHON) run_tests.py unget

test-sact: prepare
	@$(PYTHON) run_tests.py sact

test-large: prepare
	@$(PYTHON) run_tests.py large

test-sccsdiff: prepare
	@$(PYTHON) run_tests.py sccsdiff

test-cdc: prepare
	@$(PYTHON) run_tests.py cdc

test-binary: prepare
	@$(PYTHON) run_tests.py binary

test-rmdel: prepare
	@$(PYTHON) run_tests.py rmdel

test-what: prepare
	@$(PYTHON) run_tests.py what

test-val: prepare
	@$(PYTHON) run_tests.py val

test-year-2000: prepare
	@$(PYTHON) run_tests.py year-2000

test-bsd-sccs: links
	@$(PYTHON) run_tests.py bsd-sccs

# Always do test-initial FIRST.
all-tests:      test-initial \
                test-rmdel \
                test-admin test-delta test-get test-prs test-prt test-unget \
                test-cdc  test-sact test-val \
                test-large test-sccsdiff test-binary test-bsd-sccs test-what \
                test-year-2000
	echo Tests passed.

check: all-tests

# When a distribution is being made and all the files that automake
# knows about have been copied into $(distdir), dist-hook is called.

# This old code copied the CVS directories too, which is awkward
# for anybody wanting to import the distribution into a local CVS 
# repository.
#
# dist-hook: 
#         ( cd $(srcdir) && find common $(TESTDIRS) -type l -print | \
#               xargs $(RM) )
#         ( cd $(srcdir)  && tar cf - common $(TESTDIRS) ) | \
#         ( cd $(distdir) && tar xf - )
# 
dist-hook: 
	( cd $(srcdir) && find common $(TESTDIRS) -type l -print | \
		xargs $(RM) )
	( cd $(srcdir) && tar cf - \
		$$( find . \( -type d -name CVS -prune \) -o \
		-type f -print ) ) | \
	( cd $(distdir) && tar xfv - ) ; true
	$(srcdir)/../auxfiles/check-testfiles.sh \
		"$$(cd $(distdir) && pwd)" \
		"$$(cd $(srcdir) && pwd)" \
		$(TESTFILE_SUFFIXES)
