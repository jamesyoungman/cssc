/*
 * pipe.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class pipe.
 *
 */

#ifdef __GNUC__
#pragma implementation "pipe.h"
#endif

#include "mysc.h"
#include "pipe.h"
#include "run.h"
#include "list.h"
#include "sysdep.h"

#ifdef CONFIG_SCCS_IDS
static char const sccs_id[] = "@(#) MySC pipe.c 1.1 93/11/09 17:17:57";
#endif

extern int create(string name, int mode); /* file.c */

#ifdef CONFIG_NO_PIPE

/* Deletes the temporary file if the programme quits prematurely. */

void 
pipe::do_cleanup() {
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

/* Constructor for class pipe.  Creates a temporary file to emulate
   a pipe. */

pipe::pipe() {
#ifdef CONFIG_TEMP_DIRECTORY
	char *s = tempnam(CONFIG_TEMP_DIRECTORY, "mysc");
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
		     (char const *) name);
	}
	f = fdopen(fd, "w");
	if (f == NULL) {
		quit(errno, "fdopen() failed.");
	}
}


/* Returns the stream to use to read from the pipe. */

FILE *
pipe::read_stream() {
	fd = open(name, O_RDONLY);
	if (fd == -1) {
		quit(errno, "%s: Can't reopen temporary file.",
		     (char const *) name);
	}
	f = fdopen(fd, "r");
	if (f == NULL) {
		quit(errno, "fdopen() failed.");
	}
	return f;
}


/* Closes the read stream and deletes the temporary file. */

int
pipe::read_close() {
	fclose(f);
	f = NULL;
	fd = -1;
	if (remove(name) == -1) {
		quit(errno, "%s: Can't remove temporary file.",
		     (char const *) name);
	}
	return 0;
}

/* Runs the diff command with a one pipe as stdin and another pipe
   as stdout. */

int	
run_diff(char const *gname, pipe &in, pipe &out) {
	int old_stdin = dup(0);
	if (old_stdin == -1) {
		quit(errno, "dup(0) failed.");
	}
	int old_stdout = dup(1);
	if (old_stdout == -1) {
		quit(errno, "dup(1) failed.");
	}

	in.read_stream();

	close(0);
	if (dup(in.fd) != 0) {
		quit(errno, "dup() != 0");
	}

	close(1);
	if (dup(out.fd) != 1) {
		quit(errno, "dup() != 1");
	}

	list<char const *> args;

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


#else /* CONFIG_NO_PIPE */

#ifdef CONFIG_NO_FORK
#error "CONFIG_NO_FORK may not be defined if CONFIG_NO_PIPE isn't defined."
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
	assert(pid != -1 && pid != 0);

	if (reaped) {
		pid = -1;
		return status;
	}

	int st;
	int r = ::wait(&st);
	while(r != pid) {
		if (r != -1) {
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
	pid = -1;

	return st;
}


/* This is used to keep system dependencies out of the include files. */

NORETURN
pipe::_exit(int ret) {
	::_exit(ret);
}


/* Constructor for the class pipe.  Creates a pipe and forks, giving
   the write end of the pipe to child process and the read end to the
   parent process. */

pipe::pipe() {
	int fds[2];

	if (::pipe(fds) == -1) {
		quit(errno, "pipe() failed.");
	}

	pid = fork();
	if (pid == -1) {
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
run_diff(char const *gname, pipe &in, pipe &out) {
	if (out.write_stream() == NULL) {
		return 0;
	}

	close(0);
	if (dup(in.fd) != 0) {
		quit(errno, "dup() != 0.");
	}

	close(1);
	if (dup(out.fd) != 1) {
		quit(errno, "dup() != 1.");
	}

	execl(CONFIG_DIFF_COMMAND, CONFIG_DIFF_COMMAND,
#ifdef CONFIG_DIFF_SWITCHES
	      CONFIG_DIFF_SWITCHES,
#endif
	      "-", (char const *) gname, (char const *) 0);

	quit(errno, "execl(\"" CONFIG_DIFF_COMMAND "\") failed.");

}

#endif /* CONFIG_NO_PIPE */

/* Local variables: */
/* mode: c++ */
/* End: */
