/*
 * sccsname.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class sccs_name.
 *
 * @(#) CSSC sccsname.h 1.1 93/11/09 17:17:50
 *
 */

#ifndef __SCCSNAME_H__
#define __SCCSNAME_H__

#ifdef __GNUC__
#pragma interface
#endif

const char *base_part(const char *s);

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

/*	sccs_name(mystring n): name(n), lock_cnt(0) { create(n); } /**/
	sccs_name(): _name(NULL), lock_cnt(0) {}

	int valid() const { return _name != NULL; }
	void make_valid();

	operator const char *() const { return (const char *) name; }
	sccs_name &operator =(mystring n); /* undefined */

#ifdef CONFIG_MSDOS_FILES

	mystring gfile() const { return gname; }

	mystring pfile() const { return _file('%'); }
	mystring qfile() const { return _file('^'); }
/*	mystring lfile() const { return base_part(_file('!')); } /**/
	mystring xfile() const { return _file('\''); }
	mystring zfile() const { return _file('&'); }

#else

	mystring gfile() const { return gname; }

	mystring pfile() const { return _file('p'); }
	mystring qfile() const { return _file('q'); }
/*	mystring lfile() const { return base_part(_file('l')); } /**/
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
