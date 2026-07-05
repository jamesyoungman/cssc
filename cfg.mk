# The NEWS version check does not (yet) work for us because of
# differences in the format of the file, but
# d41d8cd98f00b204e9800998ecf8427e is the md5sum of an empty input.
# We use that initially to silence the error we would otherwise get.
old_NEWS_hash = 9bd58f124b49f7e69e6e4f3916f07ab1


# Exempt the Google C++ Testing Framework from the syntax-check which
# detects unexpanded at-foo-at keywords, in order not to have to
# maintain local diffs.
_makefile_at_at_check_exceptions = ' && !/PTHREAD_CFLAGS/ && !/PTHREAD_LIBS/'

sc_shellcheck:
	@$(VC_LIST) | $(GREP) '[.]sh$$' | xargs readlink -f | $(srcdir)/$(_build-aux)/shell_check.sh

release_archive_dir = ../..//tarfiles/public-releases/

# The CSSC package lives on GNU FTP sites in the directory "cssc", not
# "CSSC".
upload_dest_dir_ = cssc

# Indentation customisation is controlled by indent_args.  We depart
# from the (GNU) defaults in some small ways.
indent_args = --no-space-after-function-call-names --dont-break-procedure-type

# Our .h files are C++ rather than C.  GNU indent does not handle C++
# very well, so don't re-indent .h or .cc files. Also don't reindent
# sccs.c, instead retain the original indentation style.
exclude_file_name_regexp--sc_indent = [.]h$$|[.]cc$$|(src/sccs[.]c$$)


exclude_file_name_regexp--sc_codespell = ^po/.*.po|doc/.*.pdf$$|[.](binary|latin1.txt|input)$$|^tests/binary/umsp_s$$|^build-aux/git-log-fix$$

# Configuration for sc_codespell
codespell_ignore_words_list = ""
cssc_codespell_inhibit_color = --disable-colors
cssc_codespell_exclusion_file = $(srcdir)/$(_build-aux)/codespell.line-exclusions.txt
codespell_extra_args = --ignore-words $(srcdir)/$(_build-aux)/codespell.ignore-words.txt $(cssc_codespell_inhibit_color) --exclude-file=$(cssc_codespell_exclusion_file)
