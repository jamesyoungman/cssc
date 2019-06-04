/*
 * run.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2002, 2007, 2008, 2009, 2010,
 *  2011, 2014, 2019 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Routines for running programmes.
 *
 */
#include "config.h"

#include <string>
#include <vector>
#include <cstdio>
#include <errno.h>

#include "cssc.h"
#include "run.h"
#include "sccsfile.h"
#include "sysdep.h"
#include "quit.h"

#if !defined HAVE_FORK && !defined HAVE_SPAWN
#define NEED_CALL_SYSTEM
#else
#undef NEED_CALL_SYSTEM
#endif

// According to the ANSI standard, id the argument to system()
// is not NULL, the return value is implementation-defined.
//
// PROBLEM: system() returns an implementation-defined value
// unless its argument is NULL.  This means that we cannot use it where
// we care about the meaning of the return value.  This in turn means that
// MR validation will never fail (that is, it won't fail when it is
// supposed to, it will instead succeed all the time).
//
// Under Unix, system() returns a value which should be examined with
// the use of the WEXITSTATUS macro.  The diagnosed return value is -1
// or 127 for two kinds of failure not involving the called program,
// and otherwise the return value of the program.  Success is
// indicated by a zero return value.   However, Unix also has fork() and
// so we wouldn't be using this function on Unix.  Nevertheless it's a
// valid choice to manually undefine HAVE_FORK, and so we support this
// by the use of WEXITSTATUS.
//
// XXX: this code probably won't work on many systems because we don't
// know how to interpret the result of system().

#ifdef NEED_CALL_SYSTEM
static bool call_system(const char *s)
{
  int failed;
  int ret;

  errno = 0;
  ret = system(s);

#ifdef WEXITSTATUS
  failed = (WEXITSTATUS(ret) != 0);
#else
#ifdef SYSTEM_FAILS_RETURNING_MINUS_ONE
  failed = (-1 == ret);
#else
  failed = (ret != 0);
#endif
#endif

  if (errno)
    {
      errormsg_with_errno("call_system(\"%s\") failed (returned %d).", s, ret);
      return false;
    }

  if (failed)
      return false;
  else
      return true;
}
#endif


/* Runs a programme and returns its exit status. */

int
run(const char *prg, std::vector<const char *> const &args) {
        const size_t len = args.size();

#ifdef NEED_CALL_SYSTEM

        int cmdlen = strlen(prg) + 1;

        for (size_t i = 0; i < len; i++) {
                cmdlen += strlen(args[i]) + 1;
        }

        char *s = new char[cmdlen+1];

        strcpy(s, prg);

        for (size_t i = 0; i < len; i++) {
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

        for (size_t i = 0; i < len; i++) {
                argv[i + 1] = args[i];
        }
        argv[len+1] = NULL;

#ifndef HAVE_FORK
        // Use spawn() instead then
        int ret = spawnvp(P_WAIT, (char *) prg, (char **) argv);
        if (ret == -1)
          {
            errormsg_with_errno("spawnvp(\"%s\") failed.", prg);
          }

#else /* HAVE_FORK */
        // We _do_ have fork().

        // SunOS 4.1.3 appears not to like fflush(NULL).
#if 0
        fflush(NULL);
#else
        fflush(stdout);
        fflush(stderr);
#endif
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

bool
sccs_file::check_mrs(const std::vector<std::string>& mrs)
{
  ASSERT(nullptr != flags.mr_checker);
  return 0 != run_mr_checker(flags.mr_checker->c_str(),
			     name.gfile().c_str(), mrs);
}

/* Runs a program to check MRs. */
int
run_mr_checker(const char *prg, const char *arg1, const std::vector<std::string>& mrs)
{
  // If the validation flag is set but has no value, PRG will be an
  // empty string and the validation should succeed silently.  This is
  // for compatibility with "real" SCCS.
  if (prg[0])
    {
      std::vector<const char *> args;

      args.push_back(arg1);

      for (const auto& mr : mrs)
	{
	  args.push_back(mr.c_str());
	}

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
