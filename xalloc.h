/*
 * xalloc.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Declarations of safe versions of the standard memory allocation routines.
 *
 * @(#) CSSC xalloc.h 1.1 93/11/09 17:30:45
 *
 */

#ifndef CSSC__XALLOC_H__
#define CSSC__XALLOC_H__

void *xmalloc(size_t size);
void *xrealloc(void *p, size_t size);
void *xcalloc(size_t n, size_t size);
char *xstrdup(const char *);


#ifdef __GNUG__
// GNU C++ compiler.
#if __GNUG__ >= 2
// GNU C++ compiler, version 2 or above.
#if __GNUC_MINOR__ > 7
// GNU C++ newer than 2.7.
#define EXCEPTIONS_SUPPORTED
#endif
#endif
#else
// Assume exceptions supported.
#define EXCEPTIONS_SUPPORTED 
#endif


#ifdef EXCEPTIONS_SUPPORTED
class bad_alloc;
#define OPERATOR_NEW_THROWSPEC /* throw(bad_alloc) */
#define OPERATOR_DELETE_THROWSPEC throw() /* Throws nothing. */
#else
#define OPERATOR_NEW_THROWSPEC /* throw(bad_alloc) */
#define OPERATOR_DELETE_THROWSPEC /* Throws nothing anyway. */
#endif



void *operator new(size_t size) OPERATOR_NEW_THROWSPEC; 
void operator delete(void *p)   OPERATOR_DELETE_THROWSPEC;

#endif /* __XALLOC_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
