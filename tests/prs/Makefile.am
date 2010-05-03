EXTRA_DIST = Makefile.am keywords.sh


check: 
	$(SHELL) -ec 'for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'
