/*
 * xalloc.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Declarations of safe versions of the standard memory allocation routines.
 *
 * @(#) CSSC xalloc.h 1.1 93/11/09 17:30:45
 *
 */

#ifndef __XALLOC_H__
#define __XALLOC_H__

void *xmalloc(size_t size);
void *xrealloc(void *p, size_t size);
void *xcalloc(size_t n, size_t size);

void *operator new(size_t size);
void operator delete(void *p);

#endif /* __XALLOC_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
