/*
 * runtests.cc: Part of GNU CSSC.
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

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include <dirent.h>
#include <errno.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// gnulib includes
#include <error.h>
#include <getopt.h>

extern "C"
{
  extern char **environ;
}

using std::cerr;
using std::cout;
using std::string;
using std::vector;

string program_name = "runtests";

struct test_dir_result
{
  int passes;
  int failures;

  test_dir_result() : passes(0), failures(0)
  {
  }
};

struct program_result
{
  bool success;
  string stdout_output;
  string stderr_output;
};

class OutputBookends
{
public:
  OutputBookends(std::ostream& os, string prefix, string suffix)
    : os_(os), suffix_(suffix)
  {
    os.write(prefix.c_str(), prefix.size());
  }

  ~OutputBookends()
  {
    os_.write(suffix_.c_str(), suffix_.size());
  }

private:
  std::ostream& os_;
  string suffix_;
};

class RedText
{
public:
  RedText(std::ostream& os) : be_(os, "\033[91m", "\033[0m") {}

private:
  OutputBookends be_;
};


class GreenText
{
public:
  GreenText(std::ostream& os) : be_(os, "\033[92m", "\033[0m") {}

private:
  OutputBookends be_;
};

static bool
string_ends_with(const string& input, const string& suffix)
{
  auto input_it = input.rbegin();
  auto suffix_it = suffix.rbegin();
  for (; input_it != input.rend() && suffix_it != suffix.rend(); ++input_it, ++suffix_it)
    {
      if (*input_it != *suffix_it)
	{
	  return false;
	}
    }
  return suffix_it == suffix.rend();
}

static bool
is_hidden_file_or_directory (const string& name)
{
  return name[0] == '.';
}

static bool
is_likely_test(const string& name)
{
  return string_ends_with (name, ".sh");
}


static bool
filenames(const string& dir_name, std::vector<string>* output)
{
  DIR *pd = opendir(dir_name.c_str());
  if (!pd)
    {
      perror(dir_name.c_str());
      return false;
    }
  for (;;)
    {
      errno = 0;
      struct dirent* entry = readdir(pd);
      if (entry)
	{
	  string name(entry->d_name);
	  if ((!is_hidden_file_or_directory (name)) && is_likely_test (name))
	    {
	      output->push_back(entry->d_name);
	    }
	}
      else
	{
	  switch (errno)
	    {
	    case EINVAL:
	      continue;
	    case 0:
	      return true;
	    default:
	      perror(dir_name.c_str());
	      return false;
	    }
	}
    }
}

typedef program_result (*test_runner)(const string&, const string&, bool);

static bool
child_was_successful(int status)
{
  return WIFEXITED(status) && 0 == WEXITSTATUS(status);
}

static string
capture_body (int fd, const string& filename)
{
  string result;
  static char buf[8192];
  if (0 != lseek (fd, SEEK_SET, 0))
    {
      error (1, errno, "lseek failed on %s", filename.c_str());
    }

  for (;;)
    {
      ssize_t nread = read (fd, buf, sizeof(buf));
      if (nread < 0)
	{
	  error (1, errno, "failed to read from %s", filename.c_str());
	}
      else if (0 == nread)
	{
	  break;
	}
      else
	{
	  string::size_type n = nread;
	  result.append(buf, n);
	}
    }

  return result;
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

static program_result
execute_program(const string& label,
		const string& program,
		const vector<string>& args,
		bool capture_output)
{
  const char *path = program.c_str();
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
      error(1, errno, "posix_spawn_file_actions_init failed");
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
  int fd_devnull = open("/dev/null", O_RDONLY);
  if (fd_devnull < 0)
    {
      error(1, errno, "failed to open /dev/null");
    }
  posix_spawn_file_actions_adddup2(&file_actions, fd_devnull, 0);
  posix_spawn_file_actions_addclose (&file_actions, fd_devnull);
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
  program_result result;
  result.success = child_was_successful (childstatus);
  result.stdout_output = capture_body (tmp_out_fd, out_tmpfile);
  result.stderr_output = capture_body (tmp_err_fd, err_tmpfile);

  free_stringvec_items (argv);
  free_stringvec_items (env);

  close (fd_devnull);
  close (tmp_out_fd);
  close (tmp_err_fd);
  free (out_tmpfile);
  free (err_tmpfile);
  posix_spawn_file_actions_destroy (&file_actions);
  posix_spawnattr_destroy (&attr);
  return result;
}

static program_result
execute_shell_test(const string& dir_name, const string& test_name, bool capture_output)
{
  const string label = dir_name + "/" + test_name;
  vector<string> args;
  const string shell = "sh";
  args.push_back(shell);
  args.push_back(test_name);
  return execute_program(label, shell, args, capture_output);
}


static void
print_test_label(std::ostream& os, const string& dir_name, const string& test_name)
{
  const string label = dir_name + "/" + test_name;
  os << std::left << std::setw(28) << label << "...";
}


static bool
run_one_test(const string& dir_name, const string& test_name, bool capture_output, test_runner runner)
{
  print_test_label (cout, dir_name, test_name);
  if (capture_output)
    {
      cout << std::flush;
    }
  else
    {
      cout << '\n';
    }
  auto result = runner(dir_name, test_name, capture_output);
  if (result.success)
    {
      if (!capture_output)
	{
	  // Repeat the label so the reader knows which tests we're talking about.
	  print_test_label (cout, dir_name, test_name);
	}
      GreenText t(cout);
      cout << "PASS";
    }
  else
    {
      if (!capture_output)
	{
	  // Repeat the label so the reader knows which tests we're talking about.
	  print_test_label (cerr, dir_name, test_name);
	}
      RedText t(cout);
      cout << "FAIL";
      cout.flush();
      if (capture_output)
	{
	  cerr << result.stderr_output;
	  cerr.flush();
	}
      else
	{
	  cerr << "see above for error output";
	}
    }
  cout << '\n';
  return result.success;
}


static bool
tests_to_run(const string& subdir,
	     const char *argv_tail[],
	     vector<string>* todo)
{
  if (argv_tail[0])
    {
      for (const char ** arg = argv_tail; *arg; ++arg)
	{
	  todo->push_back(*arg);
	}
      return true;
    }
  else
    {
      return filenames(subdir, todo);
    }
}


static bool
run_tests(const string& subdir, bool capture_output, const char **argv_tail)
{
  test_dir_result counts;
  std::vector<string> todo;
  if (!tests_to_run(subdir, argv_tail, &todo))
    return false;

  if (0 != chdir(subdir.c_str()))
    {
      perror(subdir.c_str());
      return false;
    }
  bool result = true;
  for (auto test_file : todo)
    {
      if (string_ends_with(test_file, ".sh"))
	{
	  if (run_one_test(subdir, test_file, capture_output, execute_shell_test))
	    {
	      ++counts.passes;
	    }
	  else
	    {
	      ++counts.failures;
	    }
	}
    }
  if (counts.failures > 0)
    {
      cerr << subdir << ": ";
      RedText t(cerr);
      cerr << counts.failures << " tests failed\n";
      return false;
    }
  else if (counts.passes > 0)
    {
      cout << subdir << ": ";
      GreenText t(cerr);
      cout << "all " << counts.passes << " tests passed\n";
      return true;
    }
  else
    {
      cerr << subdir << ": ";
      RedText t(cerr);
      cerr << " apparently contains no tests at all\n";
      return false;
    }
}

int main(int argc, char *argv[])
{
  if (argc > 0 && argv[0])
    {
      program_name = argv[0];
    }
  int capture_output = 1;
  if (getenv("CSSC_RUNTESTS_NO_CAPTURE"))
    {
      capture_output = 0;
    }
  const struct option long_options[] =
    {
      {
	"capture", false, &capture_output, 1,
      },
      {
	"nocapture", false, &capture_output, 0,
      },
    };
  bool bad_options = false;
  int option;
  int option_index = 0;
  for (;;)
    {
      int option = getopt_long (argc, argv, ":", long_options, &option_index);
      if (-1 == option)
	{
	  break;
	}
      switch (option)
	{
	case 0:
	  // Long option, OK.
	  break;
	case ':':
	  // Missing option argument.
	  bad_options = true;
	  cerr << program_name << ": option " << long_options[option_index].name << " requires an argument\n";
	  break;
	case '?':
	  // Unknown option character.
	  cerr << program_name << ": unknown option " << static_cast<char>(option) << "\n";
	  bad_options = true;
	  break;
	default:
	  cerr << program_name << ": getopt_long returned unexpected character with decimal value " << option << "\n";
	  bad_options = true;
	}
    }
  if (bad_options)
    {
      return 1;
    }
  if (argc < 2)
    {
      cerr << "usage: " << program_name << " " << "[--capture|--nocapture] DIRECTORY [TEST...]\n";
      return 1;
    }
  const char ** tail = const_cast<const char**>(&argv[optind+1]);
  if (run_tests(argv[optind], capture_output ? true : false, tail))
    {
      return 0;
    }
  else
    {
      return 1;
    }
}
