/*
 * pipe.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class Pipe.
 *
 * @(#) MySC pipe.h 1.1 93/12/30 17:33:40
 *
 */

#ifndef __PIPE_H__
#define __PIPE_H__

#include <stdio.h>

#ifdef __GNUC__
#pragma interface
#endif

/* One of two definitions of the class Pipe are used depending if the
   system supports pipes (and fork) or not.  If pipes are not supported
   then a temporary file used to emulate one.  The members must be
   called in order: constructor, write_stream(), write_close(),
   read_stream(), read_close(), destructor. */

#ifndef HAVE_PIPE

class Pipe: cleanup {
	friend int run_diff(const char *gname, Pipe &in, Pipe &out);

	FILE *f;
	int fd;
	mystring name;

	void do_cleanup();

public:
	Pipe();

	FILE *write_stream() { return f; }
	void write_close() { fclose(f); }
	FILE *read_stream();
	int read_close();
	~Pipe() { assert(f == NULL); }
};

#else /*  HAVE_PIPE */

/* Definition of the class wait_pid.  It handles the problem of
   other child processes exiting while waiting a for a specific
   child process to exit. */

class wait_pid {
	pid_t pid;
	int reaped;
	int status;
	class wait_pid *next;
	static class wait_pid *head;

	void
	link(pid_t p) {
		pid = p;
		if (p != (pid_t)-1 && p != 0) {
			reaped = 0;
			next = head;
			head = this;
		}
	}

	void unlink();

	void operator =(wait_pid const &);

public:
	wait_pid(): pid(-1) {};

	wait_pid(int p) { link(p); }

	int
	operator =(int p) {
		assert(pid == (pid_t)-1);
		link(p);
		return p;
	}

	operator pid_t() {
		return pid;
	}

	int wait();

	~wait_pid() {
		if (pid != (pid_t)-1 && pid != 0) {
			unlink();
		}
	}
};

class Pipe {
	friend int run_diff(const char *gname, Pipe &in, Pipe &out);

	int fd;
	wait_pid pid;
	int child;
	FILE *f;

	static NORETURN _exit(int) POSTDECL_NORETURN;

public:
	Pipe();

	FILE *
	write_stream() {
		if (child) {
			return f;
		} else {
			return NULL;
		}
	}

	void
	write_close() {
		if (child) {
			fclose(f);
			_exit(0);
		}
	}

	FILE *
	read_stream() {
		assert(!child);
		return f;
	}

	int
	read_close() {
		assert(!child);
		fclose(f);
		f = NULL;
		return pid.wait();
	}


	~Pipe() {
		assert(!child);
		assert(f == NULL);
	}
};	

#endif /* HAVE_PIPE */

#endif /* __PIPE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
