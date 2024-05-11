TESTDIRS =  cdc admin delta get prs prt unget large sccsdiff binary rmdel \
                bsd-sccs year-2000 initial what val
TESTFILE_SUFFIXES = .sh
MKDIR = mkdir
RUN_CSSC_TEST = $(PYTHON) $(srcdir)/run_tests.py
CLEANFILES = */command.log

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
	@$(RUN_CSSC_TEST) initial

test-admin: prepare
	@$(RUN_CSSC_TEST) admin

test-delta: prepare
	@$(RUN_CSSC_TEST) delta

test-get: prepare
	@$(RUN_CSSC_TEST) get

test-prs: prepare
	@$(RUN_CSSC_TEST) prs

test-prt: prepare
	@$(RUN_CSSC_TEST) prt

test-unget: prepare
	@$(RUN_CSSC_TEST) unget

test-sact: prepare
	@$(RUN_CSSC_TEST) sact

test-large: prepare
	@$(RUN_CSSC_TEST) large

test-sccsdiff: prepare
	@$(RUN_CSSC_TEST) sccsdiff

test-cdc: prepare
	@$(RUN_CSSC_TEST) cdc

test-binary: prepare
	@$(RUN_CSSC_TEST) binary

test-rmdel: prepare
	@$(RUN_CSSC_TEST) rmdel

test-what: prepare
	@$(RUN_CSSC_TEST) what

test-val: prepare
	@$(RUN_CSSC_TEST) val

test-year-2000: prepare
	@$(RUN_CSSC_TEST) year-2000

test-bsd-sccs: links
	@$(RUN_CSSC_TEST) bsd-sccs

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
