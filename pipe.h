/*
 * pipe.h: Part of GNU CSSC.
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
 * Defines the class Pipe.
 *
 * @(#) CSSC pipe.h 1.1 93/12/30 17:33:40
 *
 */

#ifndef CSSC__PIPE_H__
#define CSSC__PIPE_H__

#include <stdio.h>

#ifdef __GNUC__
#pragma interface
#endif

/* One of two definitions of the class Pipe are used depending if the
   system supports pipes (and fork) or not.  If pipes are not supported
   then a temporary file used to emulate one.  The members must be
   called in order: constructor, write_stream(), write_close(),
   read_stream(), read_close(), destructor. */

/*
 * In order to use pipe(), we must have it and we must
 * also have fork().
 */
#define USE_PIPE
#ifndef HAVE_FORK
#undef  USE_PIPE
#endif
#ifndef HAVE_PIPE
#undef  USE_PIPE
#endif

#ifndef USE_PIPE

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
	~Pipe() { ASSERT(f == NULL); }
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
		ASSERT(pid == (pid_t)-1);
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
		ASSERT(!child);
		return f;
	}

	int
	read_close() {
		ASSERT(!child);
		fclose(f);
		f = NULL;
		return pid.wait();
	}


	~Pipe() {
		ASSERT(!child);
		ASSERT(f == NULL);
	}
};	

#endif /* HAVE_PIPE */

#endif /* __PIPE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
