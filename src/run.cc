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
 * Routines for running programs.
 *
 */
#include "config.h"

#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>
#include <errno.h>

#include "cssc.h"
#include "sccsfile.h"
#include "sysdep.h"
#include "quit.h"

#if !defined HAVE_FORK && !defined HAVE_SPAWN
#define NEED_CALL_SYSTEM
#else
#undef NEED_CALL_SYSTEM
#endif

namespace
{
// We compile call_system even on systems where it will not be needed
// in order to prevent bit-rot.
static bool call_system(const char *s)
{
  errno = 0;
  // According to the ANSI standard, if the argument to system() is
  // not NULL, the return value is implementation-defined.
  //
  // PROBLEM: system() returns an implementation-defined value unless
  // its argument is NULL.  This means that we cannot use it where we
  // care about the meaning of the return value.  This in turn means
  // that MR validation will never fail (that is, it won't fail when
  // it is supposed to, it will instead succeed all the time).
  int ret = system(s);

  bool ok;
#ifdef WEXITSTATUS
  // Under Unix, system() returns a value which should be examined with
  // the use of the WEXITSTATUS macro.  The diagnosed return value is -1
  // or 127 for two kinds of failure not involving the called program,
  // and otherwise the return value of the program.  Success is
  // indicated by a zero return value.   However, Unix also has fork() and
  // so we wouldn't be using this function on Unix.  Nevertheless it's a
  // valid choice to manually undefine HAVE_FORK, and so we support this
  // by the use of WEXITSTATUS.
  ok = (WEXITSTATUS(ret) == 0);
#else
# ifdef SYSTEM_FAILS_RETURNING_MINUS_ONE
  ok = (-1 != ret);
# else
  // XXX: this code probably won't work on many systems because we don't
  // know how to interpret the result of system().
  ok = (ret == 0);
# endif
#endif

  if (!ok && errno != 0)
    {
      errormsg_with_errno("call_system(\"%s\") failed (returned %d).", s, ret);
      return false;
    }
  return ok;
}


/* Runs a program and returns its exit status. */
int
run(const std::string& prg, std::vector<std::string>& args)
{
#ifdef NEED_CALL_SYSTEM
  auto cmdlen = prg.size();
  for (const auto& arg : args)
    cmdlen += arg.size() + 1;

  std::string (prg);
  s.reserve(cmdlen);

  for (const auto& arg : args)
    {
      s.push_back(' ');
      s.append(arg);
    }

  bool sysfail = !call_system(s.c_str());
  if (sysfail)
    return -1;
  else
    return 0;

#else /* !(HAVE_FORK) && !(HAVE_SPAWN) */
  // Prevent an unused-function warning by referring to call_system
  // but not calling it.
  (void) &call_system;
  std::vector<const char*> argv;
  argv.reserve(args.size()+2);
  argv.push_back(prg.c_str());

  for (const auto& arg : args)
    argv.push_back(arg.c_str());
  argv.push_back(nullptr);

#ifndef HAVE_FORK
  // Use spawn() instead then
  int ret = spawnvp(P_WAIT, const_cast<char *>(prg.c_str()),
		    const_cast<char **>(argv.data()));
  if (ret == -1)
    {
      errormsg_with_errno("spawnvp(\"%s\") failed.", prg.c_str());
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
  if (pid < 0)
    {
      fatal_quit(errno, "fork() failed.");
    }

  if (pid == 0)
    {
      cleanup::set_in_child();
      errno = 0;
      execvp(prg.c_str(), const_cast<char **>(argv.data()));
      fatal_quit(errno, "execvp(\"%s\") failed.", prg.c_str());
    }

  int ret;
  pid_t r = wait(&ret);
  while (r != pid)
    {
      if (r == -1 && errno != EINTR)
	{
	  perror("wait()"); // probably ECHILD.
	  break;
	}
      r = wait(&ret);
    }

#endif /* CONFIG_NO_FORK */
  return ret;
#endif /* !(HAVE_FORK) && !(HAVE_SPAWN) */
}

}  // unnamed namespace


bool
sccs_file::check_mrs(const std::vector<std::string>& mrs)
{
  ASSERT(nullptr != flags.mr_checker);
  // If the validation flag is set but has no value, PRG will be an
  // empty string and the validation should succeed silently.  This is
  // for compatibility with "real" SCCS.
  if (flags.mr_checker->empty())
    return 0;

  std::vector<std::string> args{name_.gfile()};
  args.reserve(mrs.size() + 1);
  std::copy(mrs.cbegin(), mrs.cend(), std::back_inserter(args));
  return run(*flags.mr_checker, args);
}


/* Local variables: */
/* mode: c++ */
/* End: */
