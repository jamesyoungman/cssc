EXTRA_DIST = Makefile.am 

EXP_FILES = sf513800.sh

check: 
	$(SHELL) -ec 'for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'

dist-hook:
	mkdir $(distdir)/expected
	for i in $(EXP_FILES) ; do cp expected/$$i $(distdir)/expected ;done
