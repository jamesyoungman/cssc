/*
 * filelock.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class file_lock.
 *
 * @(#) MySC filelock.h 1.1 93/11/09 17:17:47
 *
 */

#ifndef __FILELOCK_H__
#define __FILELOCK_H__

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
