EXTRA_DIST = comment.sh flags.sh i-option.sh init-mr.sh t-option.sh

check: 
	$(SHELL) -ec 'for i in *.sh ; \
	do \
		$(SHELL) $$i || exit 1; \
	done'

