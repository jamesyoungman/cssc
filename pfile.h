/*
 * pfile.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Definition of the class sccs_pfile.
 *
 * @(#) CSSC pfile.h 1.1 93/12/30 17:32:36
 *
 */

#ifndef __PFILE_H__
#define __PFILE_H__

#include "sccsname.h"
#include "sid.h"
#include "sccsdate.h"
#include "list.h"

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
	
	list<struct edit_lock> edit_locks;

	int pos;

	NORETURN corrupt(int lineno, const char *msg) const  POSTDECL_NORETURN;

	static int
	write_edit_lock(FILE *out, struct edit_lock const &it) {
		if (it.got.print(out) || putc(' ', out) == EOF
		    || it.delta.print(out) || putc(' ', out) == EOF
		    || fputs(it.user, out) == EOF || putc(' ', out) == EOF
		    || it.date.print(out)) {
			return 1;
		}

		if (it.include != NULL
		    && ((fputs(" -i", out) == EOF || it.include.print(out)))) {
			return 1;
		}

		if (it.exclude != NULL
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

#pragma warn -inl

	int
	next() {
		while (++pos < edit_locks.length()) {
			if (!edit_locks[pos].deleted) {
				return 1;
			}
		}
		return 0;
	}

#pragma warn .inl

	struct edit_lock const *
	operator ->() const {
		return &edit_locks[pos];
	}


	int is_locked(sid id);
	int is_to_be_created(sid id);

	~sccs_pfile();

	/* pf-add.c */

	void add_lock(sid got, sid delta,
		      sid_list &included, sid_list &excluded);

	/* pf-del.c */

	enum find_status find_sid(sid id);
	void delete_lock() { edit_locks.select(pos).deleted = 1; }
	void update();

};

#endif /* __PFILE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
