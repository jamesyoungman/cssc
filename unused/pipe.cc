/*
 * pipe.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
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
#include "mylist.h"
#include "sysdep.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: pipe.cc,v 1.2 2001/09/29 19:39:42 james_youngman Exp $";
#endif

extern int create(mystring name, int mode);


/*
 * The old no-fork-provided implementation has been removed.
 * This may be a problem for AmigaOS, which has no working
 * fork().    Deal with that with temporary files, and new
 * code.  Likewise Win32.
 *
 * If you want a copy of the old code, mail me -- jay@gnu.org
 */


#ifdef USE_PIPE

/* Using pipes requires both fork() and pipe(). */

#ifndef HAVE_FORK
#error "HAVE_FORK must be defined if USE_PIPE is defined."
#endif
#ifndef HAVE_PIPE
#error "HAVE_PIPE must be defined if USE_PIPE is defined."
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

	ASSERT(&wait_pid::unlink);
}


/* Waits for until the this pid's process ends and then returns its
   exit status.  Records the exit status of any other processes
   that exits in the mean time. */

int
wait_pid::wait() {
	ASSERT(pid != (pid_t)-1 && pid != 0);

	if (reaped) {
		pid = (pid_t)-1;
		return status;
	}

	int st;
	pid_t r = ::wait(&st);
	while (r != pid) {
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
		  perror("wait()"); // The wait call failed.
		  return 0;	// unsure if this is the right value to return.
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

Pipe::Pipe()
{
  int fds[2];
  
  if (::pipe(fds) == -1)
    {
      quit(errno, "pipe() failed.");
    }
  else
    {
      pid = fork();
      if (pid == (pid_t)-1)
	{
	  quit(errno, "fork() failed.");
	}
      else
	{
	  if (pid == 0)
	    {
	      child = 1;
	      cleanup::set_in_child();
	      close(fds[0]);
	      fd = fds[1];
	      f = fdopen(fds[1], "w");
	    }
	  else
	    {
	      child = 0;
	      close(fds[1]);
	      fd = fds[0];
	      f = fdopen(fds[0], "r");
	    }
	  
	  if (f == NULL)
	    {
	      quit(errno, "fdopen() failed.");
	    }
	}
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
	      "-", gname, (const char *) 0);

	quit(errno, "execl(\"" CONFIG_DIFF_COMMAND "\") failed.");

}

#endif /* USE_PIPE */


/* Local variables: */
/* mode: c++ */
/* End: */
