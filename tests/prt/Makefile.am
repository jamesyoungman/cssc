EXTRA_DIST = Makefile.am TODO all-512.sh all-variations.txt all.expected.Z default.sh nodel.sh s.testfile2.uue testfile.uue


EXP_FILES = default.1 nodel.-f nodel.-t-b nodel.-u-f \
	nodel.-b nodel.-t nodel.-u nodel.-u-f-t-b 
dist-hook:
	mkdir $(distdir)/expected
	for i in $(EXP_FILES) ; do cp expected/$$i $(distdir)/expected ;done
