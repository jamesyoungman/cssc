/*
 * filelock.h: Part of GNU CSSC.
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
 * Defines the class file_lock.
 *
 * @(#) CSSC filelock.h 1.1 93/11/09 17:17:47
 *
 */

#ifndef CSSC__FILELOCK_H__
#define CSSC__FILELOCK_H__

#ifdef __GNUC__
#pragma interface
#endif

#ifdef CONFIG_NO_LOCKING

class file_lock {
public:
	file_lock(mystring) {}
	int failed() { return 0; }
	~file_lock() {}
};

#else /* CONFIG_NO_LOCKING */

class file_lock: cleanup {
	int locked;
	mystring name;
#ifdef CONFIG_SHARE_LOCKING
	FILE *f;
#endif

	void do_cleanup() { this->~file_lock(); }

public:
	file_lock(mystring zname);
	int failed() { return !locked; }
	~file_lock();
};

#endif /* CONFIG_NO_LOCKING */

#endif /* __FILELOCK_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
