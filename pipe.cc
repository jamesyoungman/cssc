/*
 * pipe.c: Part of GNU CSSC.
 * 
 * Defines the function _chmod for MS-DOS systems.
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
 * Members of the class Pipe.
 *
 */

#ifdef __GNUC__
#pragma implementation "pipe.h"
#endif

class Pipe;

#include "cssc.h"
#include "pipe.h"
#include "run.h"
#include "list.h"
#include "sysdep.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: pipe.cc,v 1.5 1997/07/02 18:02:14 james Exp $";
#endif

extern int create(mystring name, int mode); /* file.c */

#ifndef HAVE_PIPE

/* Deletes the temporary file if the programme quits prematurely. */

void 
Pipe::do_cleanup() {
	if (name != NULL) {
		if (fd != -1) {
			if (f != NULL) {
				fclose(f);
			} else {
				close(fd);
			}
			remove(name);
		}
	}
}

/* Constructor for class Pipe.  Creates a temporary file to emulate
   a pipe. */

Pipe::Pipe() {
#ifdef CONFIG_TEMP_DIRECTORY
	char *s = tempnam(CONFIG_TEMP_DIRECTORY, "cssc");
	name = s;
	if (s == NULL) {
		quit(-1, "tempnam() failed.");
	}
	free(s);
#else
	name = tmpnam(NULL);
	if (name == NULL) {
		quit(-1, "tmpnam() failed.");
	}
#endif

	fd = create(name, CREATE_EXCLUSIVE | CREATE_READ_ONLY);
	if (fd == -1) {
		quit(errno, "%s: Can't create temporary file.",
		     (const char *) name);
	}
	f = fdopen(fd, "w");
	if (f == NULL) {
		quit(errno, "fdopen() failed.");
	}
}


/* Returns the stream to use to read from the Pipe. */

FILE *
Pipe::read_stream() {
	fd = open(name, O_RDONLY);
	if (fd == -1) {
		quit(errno, "%s: Can't reopen temporary file.",
		     (const char *) name);
	}
	f = fdopen(fd, "r");
	if (f == NULL) {
		quit(errno, "fdopen() failed.");
	}
	return f;
}


/* Closes the read stream and deletes the temporary file. */

int
Pipe::read_close() {
	fclose(f);
	f = NULL;
	fd = -1;
	if (remove(name) == -1) {
		quit(errno, "%s: Can't remove temporary file.",
		     (const char *) name);
	}
	return 0;
}

/* Runs the diff command with a one pipe as stdin and another pipe
   as stdout. */

int	
run_diff(const char *gname, Pipe &pipe_in, Pipe &pipe_out) {
	int old_stdin = dup(0);
	if (old_stdin == -1) {
		quit(errno, "dup(0) failed.");
	}
	int old_stdout = dup(1);
	if (old_stdout == -1) {
		quit(errno, "dup(1) failed.");
	}

	pipe_in.read_stream();

	close(0);
	if (dup(pipe_in.fd) != 0) {
		quit(errno, "dup() != 0");
	}

	close(1);
	if (dup(pipe_out.fd) != 1) {
		quit(errno, "dup() != 1");
	}

	list<const char *> args;

#ifdef CONFIG_DIFF_SWITCHES
	args.add(CONFIG_DIFF_SWITCHES);
#endif
	args.add("-");
	args.add(gname);

	int ret = run(CONFIG_DIFF_COMMAND, args);

	close(0);
	close(1);
	if (dup(old_stdin) != 0 || dup(old_stdout) != 1) {
		quit(errno, "dup() failed.");
	}
	close(old_stdin);
	close(old_stdout);

	return ret;
}


#else /* HAVE_PIPE */

#ifndef HAVE_FORK
#error "HAVE_FORK must be defined if HAVE_PIPE is defined."
#endif


/* Head of list class wait_pid objects that haven't been reaped yet. */

class wait_pid *wait_pid::head = NULL;


/* Removes "this" from the list of unreaped wait_pid's */

void
wait_pid::unlink() {
	class wait_pid *p = head;

	if (p == this) {
		head = next;
		return;
	}

	while(p != NULL) {
		if (p->next == this) {
			p->next = next;
			return;
		}
		p = p->next;
	}

	assert(&wait_pid::unlink);
}


/* Waits for until the this pid's process ends and then returns its
   exit status.  Records the exit status of any other processes
   that exits in the mean time. */

int
wait_pid::wait() {
	assert(pid != (pid_t)-1 && pid != 0);

	if (reaped) {
		pid = (pid_t)-1;
		return status;
	}

	int st;
	pid_t r = ::wait(&st);
	while(r != pid) {
		if (r != (pid_t)-1) {
			class wait_pid *p = head;

			while(p != NULL) {
				if (p->pid == r) {
					unlink();
					p->reaped = 1;
					p->status = st;
					break;
				}
				p = p->next;
			}
		} else if (errno != EINTR) {
			quit(errno, "wait() failed.");
		}
		r = ::wait(&st);
	}

	unlink();
	pid = (pid_t)-1;

	return st;
}


/* This is used to keep system dependencies out of the include files. */

NORETURN
Pipe::_exit(int ret) {
	::_exit(ret);
}


/* Constructor for the class Pipe.  Creates a pipe and forks, giving
   the write end of the pipe to child process and the read end to the
   parent process. */

Pipe::Pipe() {
	int fds[2];

	if (::pipe(fds) == -1) {
		quit(errno, "pipe() failed.");
	}

	pid = fork();
	if (pid == (pid_t)-1) {
		quit(errno, "fork() failed.");
	}

	if (pid == 0) {
		child = 1;
		cleanup::set_in_child();
		close(fds[0]);
		fd = fds[1];
		f = fdopen(fds[1], "w");
	} else {
		child = 0;
		close(fds[1]);
		fd = fds[0];
		f = fdopen(fds[0], "r");
	}

	if (f == NULL) {
		quit(errno, "fdopen() failed.");
	}
}


/* Runs the diff command with a one pipe as stdin and another pipe
   as stdout. */

int
run_diff(const char *gname, Pipe &pipe_in, Pipe &pipe_out)
{
	    
	if (pipe_out.write_stream() == NULL) {
		return 0;
	}

	close(0);
	if (dup(pipe_in.fd) != 0) {
		quit(errno, "dup() != 0.");
	}

	close(1);
	if (dup(pipe_out.fd) != 1) {
		quit(errno, "dup() != 1.");
	}

	execl(CONFIG_DIFF_COMMAND, CONFIG_DIFF_COMMAND,
#ifdef CONFIG_DIFF_SWITCHES
	      CONFIG_DIFF_SWITCHES,
#endif
	      "-", (const char *) gname, (const char *) 0);

	quit(errno, "execl(\"" CONFIG_DIFF_COMMAND "\") failed.");

}

#endif /* HAVE_PIPE */

/* Local variables: */
/* mode: c++ */
/* End: */
