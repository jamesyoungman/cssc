# The NEWS version check does not (yet) work for us because of
# differences in the format of the file, but
# d41d8cd98f00b204e9800998ecf8427e is the md5sum of an empty input.
# We use that initially to silence the error we would otherwise get.
old_NEWS_hash = d41d8cd98f00b204e9800998ecf8427e


# Exempt the Google C++ Testing Framework from the @FOO@ syntax-check
# on order not to have to maintain local diffs.
_makefile_at_at_check_exceptions = ' && !/PTHREAD_CFLAGS/ && !/PTHREAD_LIBS/'
