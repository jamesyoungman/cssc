csscutildir = $(libexecdir)/cssc
bin_PROGRAMS = sccs
# "itbm" stands for "include the blasted manual pages".
itbm = sccs.1
EXTRA_DIST = sccs.me COPYING.bsd .ispell_english $(itbm)
man_MANS = sccs.1

AM_CPPFLAGS = "-DPREFIX=\"$(csscutildir)/\""

# The .ispell_english file is included in the distribution because it
# is useful for the list of correct spellings in this document to
# accompany the document itself.
