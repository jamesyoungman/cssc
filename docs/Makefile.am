## CSSC's docs/Makefile.am.

info_TEXINFOS = cssc.texi 
cssc_TEXINFOS = gpl.texi config-info.texi

EXTRA_DIST = BUGS FIXED TODO CREDITS TESTING \
	missing.txt patches.txt \
	Platforms mailing-list.txt style.txt \
	texinfo.tex \
	.ispell_english

# XXX: this is slightly brain-damaged; a file in $(srcdir) is modified
#      based on the results of running "configure".  That's usually 
#      undesirable.  However, I can't get "makeinfo" and "texi2dvi"
#      to include files except those in the $(srcdir).
## $(srcdir)/config-info.texi: config-info.texi
## 	   cp $(srcdir)/config-info.texi x
## 	   cp x config-info.texi
## 	   $(RM) x
##*c	


# The .ispell_english file is included in the distribution because it
# is useful for the list of correct spellings in this document to
# accompany the document itself.

MOSTLYCLEANFILES = cssc.info
