EXTRA_DIST = Makefile.am output.sh


check: 
	$(SHELL) -ec 'for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'
