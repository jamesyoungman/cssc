/*
 * run.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Routines for running programmes.
 *
 */

#include "cssc.h"
#include "run.h"
#include "list.h"
#include "sysdep.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: run.cc,v 1.5 1997/05/10 14:49:53 james Exp $";
#endif

// According to the ANSI standard, id the argument to system()
// is not NULL, the return value is implementation-defined.
//
// Under Unix, system() returns -1 or 127 for two
// kinds of failure not involving the called program,
// and otherwise the return value of the program.
// Success is indicated by a zero return value.

void call_system(const char *s)
{
  int failed;
  int ret = system(s);

#ifdef SYSTEM_FAILS_RETURNING_MINUS_ONE
  failed = (-1 == ret);
#else
  failed = (ret != 0);
#endif
  
  if (failed)
    quit(errno, "system(\"%s\") failed, returning %d.", s, ret);
}

/* Runs a programme and returns its exit status. */

int
run(const char *prg, list<const char *> const &args) {
	int i;
	int len = args.length();

#if !(HAVE_FORK) && !(HAVE_SPAWN)

	int cmdlen = strlen(prg) + 1;
	
	for(i = 0; i < len; i++) {
		cmdlen += strlen(args[i]) + 1;
	}

	char *s = (char *) xmalloc(cmdlen + 1);

	strcpy(s, prg);
	
	for(i = 0; i < len; i++) {
		strcat(s, " ");
		strcat(s, args[i]);
	}

	call_system(s);
	free(s);

#else /* !(HAVE_FORK) && !(HAVE_SPAWN) */

	const char *  *argv = (const char *  *) xmalloc((len + 2) 
						      * sizeof(const char *  *));

	argv[0] = prg;

	for(i = 0; i < len; i++) {
		argv[i + 1] = args[i];
	}
	
	argv[i + 1] = NULL;

#ifndef HAVE_FORK
	// Use spawn() instead then
	int ret = spawnvp(P_WAIT, (char *) prg, (char **) argv);
	if (ret == -1) {
		quit(errno, "spawnvp(\"%s\") failed.", prg);
	}

#else /* CONFIG_NO_FORK */
	// We _do_ have fork().
	pid_t pid = fork(); 
	if (pid < 0) {
		quit(errno, "fork() failed.");
	}

	if (pid == 0) {
		cleanup::set_in_child();
		execvp(prg, (char **) argv);
		quit(errno, "execvp(\"%s\") failed.", prg);
	}

	int ret;
	pid_t r = wait(&ret);
	while(r != pid) {
		if (r == -1 && errno != EINTR) {
			quit(errno, "wait() failed.");
		}
		r = wait(&ret);
	}

#endif /* CONFIG_NO_FORK */

	free(argv);

#endif /* !(HAVE_FORK) && !(HAVE_SPAWN) */

	return ret;
}


/* Runs a programme to checks MRs. */

int
run_mr_checker(const char *prg, const char *arg1, list<mystring> mrs) {
	list<const char *> args;

	args.add(arg1);
	args += mrs;

	return run(prg, args);
}

/* Local variables: */
/* mode: c++ */
/* End: */
