/*
 * run.cc: Part of GNU CSSC.
 * 
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
#include "mylist.h"
#include "sysdep.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: run.cc,v 1.14 1998/09/02 21:03:29 james Exp $";
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
static bool call_system(const char *s)
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
    {
      errormsg_with_errno(errno, "call_system(\"%s\") failed.", s, ret);
      return false;
    }
  return true;
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

	char *s = new char[cmdlen+1];

	strcpy(s, prg);
	
	for(i = 0; i < len; i++) {
		strcat(s, " ");
		strcat(s, args[i]);
	}

	bool sysfail = !call_system(s);
	delete [] s;
	if (sysfail)
	  return -1;
	else
	  return 0;

#else /* !(HAVE_FORK) && !(HAVE_SPAWN) */

	const char *  *argv = new const char*[len+2];
	
	argv[0] = prg;

	for(i = 0; i < len; i++) {
		argv[i + 1] = args[i];
	}
	
	argv[i + 1] = NULL;

#ifndef HAVE_FORK
	// Use spawn() instead then
	int ret = spawnvp(P_WAIT, (char *) prg, (char **) argv);
	if (ret == -1)
	  {
	    errormsg_with_errno(errno, "spawnvp(\"%s\") failed.", prg);
	  }

#else /* HAVE_FORK */
	// We _do_ have fork().
	fflush(NULL);
	pid_t pid = fork(); 
	if (pid < 0) {
		fatal_quit(errno, "fork() failed.");
	}

	if (pid == 0) {
		cleanup::set_in_child();
		execvp(prg, (char **) argv);
		fatal_quit(errno, "execvp(\"%s\") failed.", prg);
	}

	int ret;
	pid_t r = wait(&ret);
	while (r != pid) {
		if (r == -1 && errno != EINTR) {
		  perror("wait()"); // probably ECHILD.
		  break;
		}
		r = wait(&ret);
	}

#endif /* CONFIG_NO_FORK */

	delete [] argv;
	return ret;
	
#endif /* !(HAVE_FORK) && !(HAVE_SPAWN) */
}

inline list<const char*> &
operator +=(list<const char*> &l1, list<mystring> const &l2)
{
  int len = l2.length();
  int i;
  for(i = 0; i < len; i++)
    {
      // This add operation would be push_back() under STL.
      // When everybody supports STL, we'll switch.
      l1.add(l2[i].c_str());
    }
  return l1;
}

/* Runs a program to check MRs. */

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

      int len = mrs.length();
      for(int i = 0; i < len; i++)
	args.add(mrs[i].c_str()); // STL's push_back

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
