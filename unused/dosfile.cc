/*
 * dosfile.cc: Part of GNU CSSC.
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
 * DOS-specific routines for accessing files.
 *
 */
#include "cssc.h"
#include "sysdep.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <stdio.h>



#ifdef CONFIG_USE_ARCHIVE_BIT

#ifdef CONFIG_NO__CHMOD

#include "_chmod.cc"

#ifndef FA_ARCH
#define FA_ARCH 0x20
#endif

#endif /* CONFIG_NO__CHMOD */


/* Clears the archive file attribute bit of a file. */

void
clear_archive_bit(const char *name) {
	int attribs = _chmod(name, 0);
	if (attribs == -1 || _chmod(name, 1, attribs & ~FA_ARCH) == -1) {
		quit(errno, "%s: Can't clear archive file attribute.", name);
	}
}


/* Returns true if the archive file attribute bit of a file is set. */

inline int
test_archive_bit(const char *name) {
	int attribs = _chmod(name, 0);
	if (attribs == -1) {
		quit(errno, "%s: Can't test archive file attribute.", name);
	}
	return (attribs & FA_ARCH) == FA_ARCH;
}

#endif /* CONFIG_USE_ARCHIVE_BIT */




file_lock::file_lock(mystring zname): locked(0), name(zname) {
	f = fcreate(zname, CREATE_WRITE_LOCK | CREATE_EXCLUSIVE);
	if (f == NULL) {
		struct DOSERROR err;

		if (errno == EEXIST
		    || (dosexterr(&err) == EACCES
			&& err.de_class == 2 /* temporary situation */)) {
			return;
		}
		quit(errno, "%s: Can't create lock file.", 
		     zname.c_str());
	}
	if (fprintf(f, "%s\n", get_user_name()) < 0
	    || fflush_failed(fflush(f)) || ffsync(f) == EOF) {
		quit(errno, "%s: Write error.", zname.c_str());
	}

	locked = 1;
	return;
}

file_lock::~file_lock() {
	if (locked) {
		locked = 0;
		fclose(f);
		unlink(name);
	}
}
