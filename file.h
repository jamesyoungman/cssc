/*
 * file.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,2001 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Declares system dependent routines for accessing files.
 *
 * @(#) CSSC file.h 1.1 93/11/09 17:17:46
 *
 */

#ifndef CSSC__FILE_H__
#define CSSC__FILE_H__

#include "filelock.h"

enum create_mode {
	CREATE_EXCLUSIVE     =  001,
	CREATE_READ_ONLY     =  002,
	CREATE_AS_REAL_USER  =  004,
	CREATE_FOR_UPDATE    =  010,
	CREATE_FOR_GET       =  020,
#ifdef CONFIG_SHARE_LOCKING
	CREATE_WRITE_LOCK    =  040
#else	
	CREATE_NFS_ATOMIC    = 0100
#endif
};

bool stdout_to_null();
FILE *stdout_to_stderr();
int stdin_is_a_tty();
FILE *open_null();
int is_readable(const char *name);
int file_exists(const char *name);
const char *get_user_name();
int user_is_group_member(gid_t gid);
FILE *fcreate(mystring name, int mode);
bool set_file_mode(mystring &gname, int mode);

#ifdef CONFIG_SYNC_BEFORE_REOPEN
int ffsync(FILE *f);
#endif

#ifdef CONFIG_UIDS
void give_up_privileges();
void restore_privileges();
#else
inline void give_up_privileges() {}
inline void restore_privileges() {}
#endif

#ifndef HAVE_REMOVE
int remove(const char *name);
#endif

#ifndef HAVE_RENAME
int rename(const char *from, const char *to);
#endif

#ifdef CONFIG_USE_ARCHIVE_BIT
void clear_archive_bit(const char *name);
#endif

#endif /* __FILE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
