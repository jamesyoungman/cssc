/*
 * filelock.h: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,2007 Free Software Foundation, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Defines the class file_lock.
 */

#ifndef CSSC__FILELOCK_H__
#define CSSC__FILELOCK_H__

#include "cleanup.h"
#include "mystring.h"

class file_lock : private cleanup {
	int locked;
	mystring name;

	void do_cleanup() { this->~file_lock(); }

public:
	file_lock(mystring zname);
	int failed() { return !locked; }
	~file_lock();
};

#endif /* __FILELOCK_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
