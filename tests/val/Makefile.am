EXTRA_DIST = Makefile.am valbasic.sh y2k.sh


check: 
	$(SHELL) -ec 'for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'
