EXTRA_DIST = MRs.sh Makefile.am nulldelta.sh


check: 
	$(SHELL) -ec 'for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'
