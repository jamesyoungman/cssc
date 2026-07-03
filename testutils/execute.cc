/*
 * execute.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 2026 Free Software Foundation, Inc.
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
 */

#include "config.h"

// standard library and POSIX includes
#include <errno.h>
#include <string.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>

// gnulib includes
#include <error.h>

// local includes
#include "execute.h"
#include "filebody.h"

using std::string;
using std::vector;


extern "C"
{
  extern char **environ;
}

program_result::program_result(int exit_value, std::string output, std::string errors)
  : retval(exit_value), stdout_output(output), stderr_output(errors)
{
}

static bool
wait_for_child(pid_t child, int *status)
{
  pid_t done;
  for (done=wait(status); done != child; done=wait(status))
    {
      if (done == -1 && errno != EINTR)
	{
	  perror("wait()"); // probably ECHILD.
	  return false;
	}
    }
  return true;
}

static int
child_exitstatus (int status)
{
  if (WIFEXITED(status))
    {
      return WEXITSTATUS(status);
    }
  if (WIFSIGNALED(status))
    {
      int sig = WTERMSIG(status);
      return 128 + sig;
    }
  error (1, 0, "child does not seem to have exited");
  /*NOTREACHED*/
}

static void
free_stringvec_items(vector<const char*>& items)
{
  while (!items.empty())
    {
      char * p = const_cast<char*>(items.back());
      free (p);
      items.pop_back();
    }
}

program_result
execute_program(const string& program,
		const vector<string>& args,
		bool capture_output)
{
  vector<const char*> argv;
  for (auto arg : args)
    {
      argv.push_back(strdup(arg.c_str()));
    }
  argv.push_back(static_cast<const char*>(NULL));
  vector<const char*> env;
  for (const char **env_item = const_cast<const char**>(environ); *env_item; ++env_item)
    {
      env.push_back(strdup(*env_item));
    }
  env.push_back(static_cast<const char*>(NULL));
  posix_spawn_file_actions_t file_actions;
  char *out_tmpfile = strdup ("/tmp/cssc_rt_out.XXXXXX");
  char *err_tmpfile = strdup ("/tmp/cssc_rt_err.XXXXXX");
  if (0 != posix_spawn_file_actions_init (&file_actions))
    {
      error (1, errno, "posix_spawn_file_actions_init failed");
    }
  int tmp_out_fd = mkstemp (out_tmpfile);
  if (tmp_out_fd < 0)
    {
      error(1, errno, "failed to open temporary file for standard output");
    }
  int tmp_err_fd = mkstemp (err_tmpfile);
  if (tmp_err_fd < 0)
    {
      error(1, errno, "failed to open temporary file for standard error");
    }

  // We should retain the existing stdin connection.
  if (capture_output)
    {
      posix_spawn_file_actions_adddup2(&file_actions, tmp_err_fd, 2);
      posix_spawn_file_actions_adddup2(&file_actions, tmp_out_fd, 1);
    }
  posix_spawn_file_actions_addclose (&file_actions, tmp_out_fd);
  posix_spawn_file_actions_addclose (&file_actions, tmp_err_fd);

  posix_spawnattr_t attr;
  if (0 != posix_spawnattr_init(&attr))
    {
      error(1, errno, "posix_spawnattr_init failed");
    }

  pid_t child = 0;
  int res = posix_spawnp(&child, program.c_str(), &file_actions, &attr,
			 const_cast<char*const*>(argv.data()),
			 const_cast<char*const*>(env.data()));
  if (res < 0)
    {
      error (1, errno, "posix_spawn failed for %s", program.c_str());
    }
  int childstatus;
  if (!wait_for_child (child, &childstatus))
    {
      error (1, errno, "waiting for %s", program.c_str());
    }
  program_result result (child_exitstatus (childstatus),
			 read_file_body (tmp_out_fd, out_tmpfile),
			 read_file_body (tmp_err_fd, err_tmpfile));

  free_stringvec_items (argv);
  free_stringvec_items (env);

  close (tmp_out_fd);
  close (tmp_err_fd);
  free (out_tmpfile);
  free (err_tmpfile);
  posix_spawn_file_actions_destroy (&file_actions);
  posix_spawnattr_destroy (&attr);
  return result;
}
