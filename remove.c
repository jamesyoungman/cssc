#if HAVE_UNISTD_H
#include <unistdh>
#endif


#ifndef HAVE_REMOVE

int
remove(const char *name) {
	return unlink(name);
}

#endif /* HAVE_REMOVE */

