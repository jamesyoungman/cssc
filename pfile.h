/*
 * pfile.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc. 
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
 * Definition of the class sccs_pfile.
 */

#ifndef CSSC__PFILE_H__
#define CSSC__PFILE_H__

#include "sccsname.h"
#include "sid.h"
#include "sccsdate.h"
#include "mylist.h"

class sccs_pfile {
public:
	enum _mode { READ, APPEND, UPDATE };
	enum find_status { FOUND, NOT_FOUND, AMBIGUOUS };

private:
	struct edit_lock {
		sid got, delta;
		mystring user;
		sccs_date date;
		sid_list include, exclude;
		int deleted;

		edit_lock(const char *g, const char *d, const char *u,
			  const char *dd, const char *dt, const char *i,
			  const char *x)
			: got(g), delta(d), user(u), date(dd, dt),
			  include(i), exclude(x), deleted(0) {}
		edit_lock() {}
	};

	sccs_name &name;
	mystring pname;
	enum _mode mode;
	
        mylist<edit_lock> edit_locks;

	int pos;

	NORETURN corrupt(int lineno, const char *msg) const  POSTDECL_NORETURN;

	static int
	write_edit_lock(FILE *out, struct edit_lock const &it) {
		if (it.got.print(out)
		    || putc_failed(putc(' ', out))
		    || it.delta.print(out)
		    || putc_failed(putc(' ', out))
		    || fputs_failed(fputs(it.user.c_str(), out))
		    || putc_failed(putc(' ', out))
		    || it.date.print(out))
		  {
		    return 1;
		  }

		if (!it.include.empty()
		    && ((fputs(" -i", out) == EOF || it.include.print(out)))) {
			return 1;
		}

		if (!it.exclude.empty()
		    && ((fputs(" -x", out) == EOF || it.exclude.print(out)))) {
			return 1;
		}

		if (putc('\n', out) == EOF) {
			return 1;
		}
		return 0;
	}

	
public:
	sccs_pfile(sccs_name &name, enum _mode mode);

	void rewind() { pos = -1; }

	int
	next() {
		while (++pos < edit_locks.length()) {
			if (!edit_locks[pos].deleted) {
				return 1;
			}
		}
		return 0;
	}



	struct edit_lock const *
	operator ->() const {
		return &edit_locks[pos];
	}


	int is_locked(sid id);
	int is_to_be_created(sid id);

	~sccs_pfile();

	/* pf-add.c */

	bool add_lock(sid got, sid delta,
		      sid_list &included, sid_list &excluded);

	/* pf-del.c */

	enum find_status find_sid(sid id);
	int  print_lock_sid(FILE *fp);  	
	void delete_lock() { edit_locks.select(pos).deleted = 1; }
	bool update();

};

#endif /* __PFILE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
