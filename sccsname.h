/*
 * sccsname.h: Part of GNU CSSC.
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
 * Defines the class sccs_name.
 *
 * @(#) CSSC sccsname.h 1.1 93/11/09 17:17:50
 *
 */

#ifndef CSSC__SCCSNAME_H__
#define CSSC__SCCSNAME_H__

#ifdef __GNUC__
#pragma interface
#endif

const char *base_part(const char *s);
mystring canonify_filename(const char* fname);

class sccs_name {
	mystring name;
	char *_name, *change;
	mystring gname;

	file_lock *lock_ptr;
	int lock_cnt;

	char *
	_file(char c) const {
		*change = c;
		return _name;
	}

	void create();

	sccs_name(sccs_name const &);
	sccs_name &operator =(sccs_name const &);

	void
	destroy() {
		if (_name != NULL) {
			free(_name);
		}
		if (lock_cnt > 0) {
			delete lock_ptr;
		}
	}

public:
	static int valid_filename(const char *name);
#if 0
	sccs_name(mystring n): name(n), lock_cnt(0) { create(n); }
#endif
	sccs_name(): _name(NULL), lock_cnt(0) {}

	int valid() const { return _name != NULL; }
	void make_valid();

	operator const char *() const { return (const char *) name; }
	sccs_name &operator =(mystring n); /* undefined */

#ifdef CONFIG_MSDOS_FILES

	mystring gfile() const { return gname; }

	mystring pfile() const { return _file('%'); }
	mystring qfile() const { return _file('^'); }
#if 0
	mystring lfile() const { return base_part(_file('!')); }
#endif
	mystring xfile() const { return _file('\''); }
	mystring zfile() const { return _file('&'); }

#else

	mystring gfile() const { return gname; }

	mystring pfile() const { return _file('p'); }
	mystring qfile() const { return _file('q'); }
#if 0
	mystring lfile() const { return base_part(_file('l')); }
#endif
	mystring xfile() const { return _file('x'); }
	mystring zfile() const { return _file('z'); }

#endif

	int
	lock() { 
		if (lock_cnt++ == 0) {
			lock_ptr = new file_lock(zfile());
			return lock_ptr->failed();
		}
		return 0;
	}

	void
	unlock() {
		if (--lock_cnt == 0) {
			delete lock_ptr;
		}
	}

	~sccs_name() {
		destroy();
	}
};

#endif /* __SCCSNAME_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
