/*
 * xalloc.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Interface to the standard memory allocation routines that quit if
 * there isn't enough memory available. 
 *
 */

#include "cssc.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: xalloc.cc,v 1.3 1997/05/10 14:50:00 james Exp $";
#endif

#ifdef CONFIG_DECLARE_MALLOC
extern "C" {
	void * CDECL malloc(size_t);
	void * CDECL realloc(void *, size_t);
	void * CDECL calloc(size_t, size_t);
};
#endif

void *
xmalloc(size_t size) {
	void *p = malloc(size);
	if (p == NULL) {
		nomem();
	}
	return p;
}

void *
xrealloc(void *p, size_t size) {
	p = realloc(p, size);
	if (p == NULL) {
		nomem();
	}
	return p;
}

void *
xcalloc(size_t n, size_t size) {
	void *p = calloc(n, size);
	if (p == NULL) {
		nomem();
	}
	return p;
}


/* Insure that xmalloc and operator new are compatible. */

void *
operator new(size_t size) {
	return xmalloc(size);
}       

void
operator delete(void *p) {
	free(p);
}

/* Local variables: */
/* mode: c++ */
/* End: */
