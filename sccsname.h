/*
 * sccsname.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class sccs_name.
 *
 * @(#) MySC sccsname.h 1.1 93/11/09 17:17:50
 *
 */

#ifndef __SCCSNAME_H__
#define __SCCSNAME_H__

#ifdef __GNUC__
#pragma interface
#endif

char const *base_part(char const *s);

class sccs_name {
	string name;
	char *_name, *change;
	string gname;

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
	static int valid_filename(char const *name);

/*	sccs_name(string n): name(n), lock_cnt(0) { create(n); } /**/
	sccs_name(): _name(NULL), lock_cnt(0) {}

	int valid() const { return _name != NULL; }
	void make_valid();

	operator char const *() const { return (char const *) name; }
	sccs_name &operator =(string n); /* undefined */

#ifdef CONFIG_MSDOS_FILES

	string gfile() const { return gname; }

	string pfile() const { return _file('%'); }
	string qfile() const { return _file('^'); }
/*	string lfile() const { return base_part(_file('!')); } /**/
	string xfile() const { return _file('\''); }
	string zfile() const { return _file('&'); }

#else

	string gfile() const { return gname; }

	string pfile() const { return _file('p'); }
	string qfile() const { return _file('q'); }
/*	string lfile() const { return base_part(_file('l')); } /**/
	string xfile() const { return _file('x'); }
	string zfile() const { return _file('z'); }

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
