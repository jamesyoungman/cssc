#if HAVE_UNISTD_H
#include <unistdh>
#endif

#ifndef HAVE_RENAME

int
rename(const char *from, const char *to) {
	if (link(from, to) == -1 || unlink(from) == -1) {
		return -1;
	}
	return 0;
}

#endif /* HAVE_RENAME */
