/*
 * ffsync.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998 Free Software Foundation, Inc. 
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
 * System dependent routines for accessing files (used to be in file.cc)
 *
 */
#include "cssc.h"
#include "sysdep.h"

#include <stdio.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: ffsync.cc,v 1.1 1998/06/14 15:24:16 james Exp $";
#endif

#if defined(CONFIG_SYNC_BEFORE_REOPEN) || defined(CONFIG_SHARE_LOCKING)

#ifdef CONFIG_NO_FSYNC
#include "fsync.cc"
#endif

#if !defined(fileno) && defined(__BORLANDC__)
#define fileno(f) (f->fd)
#endif

int
ffsync(FILE *f) {
	if (fsync(fileno(f)) == -1) {
		return EOF;
	}
	return 0;
}

#endif /* defined(CONFIG_SYNC_BEFORE_REOPEN) || ... */
