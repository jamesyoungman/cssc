/*
 * run.c: Part of GNU CSSC.
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
 * Routines for running programmes.
 *
 */

#include "cssc.h"
#include "run.h"
#include "list.h"
#include "sysdep.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: run.cc,v 1.8 1997/07/07 21:16:29 james Exp $";
#endif

// According to the ANSI standard, id the argument to system()
// is not NULL, the return value is implementation-defined.
//
// Under Unix, system() returns -1 or 127 for two
// kinds of failure not involving the called program,
// and otherwise the return value of the program.
// Success is indicated by a zero return value.

// PROBLEM: system() returns an implementation-defined value
// unless its argument is NULL.  This means that we cannot use it where
// we care about the meaning of the return value.  This in turn means that
// MR validation will never fail (that is, it won't fail when it is
// supposed to, it will instead succeed all the time).
//
void call_system(const char *s)
{
  int failed;
  int ret;

  errno = 0;
  ret = system(s);

#ifdef SYSTEM_FAILS_RETURNING_MINUS_ONE
  failed = (-1 == ret);
#else
  failed = (ret != 0);
#endif
  
  if (errno)
    quit(errno, "call_system(\"%s\") failed, returning errno=%d.", s, ret);
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
	return 0;

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

#else /* HAVE_FORK */
	// We _do_ have fork().
	fflush(NULL);
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
	return ret;
	
#endif /* !(HAVE_FORK) && !(HAVE_SPAWN) */
}


/* Runs a programme to checks MRs. */

int
run_mr_checker(const char *prg, const char *arg1, list<mystring> mrs)
{
  // If the validation flag is set but has no value, PRG will be an
  // empty string and the validation should succeed silently.  This is
  // for compatibility with "real" SCCS.
  if (prg[0])
    {
      list<const char *> args;

      args.add(arg1);
      args += mrs;

      return run(prg, args);
    }
  else
    {
      return 0;
    }
}

/* Local variables: */
/* mode: c++ */
/* End: */
