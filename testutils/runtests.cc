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
#include <stdlib.h>
#include <unistd.h>

// gnulib includes
#include <error.h>
#include <getopt.h>

#include "execute.h"

using std::cerr;
using std::cout;
using std::string;
using std::vector;

string program_name = "runtests";

using std::vector;
using std::string;

// What to do when a test fails.
enum class FailResponse
  {
    // Stop immediately. Run no other tests.  Exit with a non-zero
    // exit status.
    StopImmediately,
    // Run the remaining tests in this directory, and then
    // exit  with a non-zero exit status.
    KeepGoing
  };

struct test_dir_result
{
  vector<string> passes;
  vector<string> failures;
  vector<string> skipped;

  test_dir_result()
    : passes(), failures(), skipped()
  {
  }
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

static string
join_strings(vector<string>& items, const string& separator)
{
  string::size_type capacity = 0;
  bool first = true;
  for (auto item : items)
    {
      if (first)
	first = false;
      else
	capacity += separator.length();
      capacity += item.length();
    }

  string result;
  result.reserve(capacity);
  first = true;
  for (auto item : items)
    {
      if (first)
	first = false;
      else
	result.append(separator);
      result.append(item);
    }
  return result;
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

static program_result
execute_shell_test(const string& /* dir_name */, const string& test_name, bool capture_output)
{
  const string shell = "sh";
  return execute_program(shell, {shell, test_name}, capture_output);
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
  if (0 == result.retval)
    {
      if (!capture_output)
	{
	  // Repeat the label so the reader knows which tests we're talking about.
	  print_test_label (cout, dir_name, test_name);
	}
      GreenText t(cout);
      cout << "PASS\n";
    }
  else
    {
      if (!capture_output)
	{
	  // Repeat the label so the reader knows which tests we're talking about.
	  print_test_label (cerr, dir_name, test_name);
	}
      RedText t(cout);
      cout << "FAIL\n";

      if (capture_output)
	{
	  cerr << "... failed test " << test_name << " generated "
	       << result.stdout_output.size()
	       << " bytes of standard output and "
	       << result.stderr_output.size()
	       << " bytes of error output.\n";
	  cerr << result.stderr_output;
	  cerr.flush();
	}
      else
	{
	  cerr << "... see above for error output from the failed test " << test_name << "\n";
	}
    }
  return 0 == result.retval;
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
run_tests(const string& subdir,
	  bool capture_output,
	  FailResponse on_failure,
	  const char **argv_tail)
{
  test_dir_result result;
  std::vector<string> todo;
  if (!tests_to_run(subdir, argv_tail, &todo))
    return false;

  if (0 != chdir(subdir.c_str()))
    {
      perror(subdir.c_str());
      return false;
    }
  bool skip_remaining_tests = false;
  for (auto test_file : todo)
    {
      if (string_ends_with(test_file, ".sh"))
	{
	  if (skip_remaining_tests)
	    {
	      result.skipped.push_back(test_file);
	    }
	  else if (run_one_test(subdir, test_file, capture_output, execute_shell_test))
	    {
	      result.passes.push_back(test_file);
	    }
	  else
	    {
	      result.failures.push_back(test_file);
	      if (on_failure == FailResponse::StopImmediately)
		{
		  cout << subdir << ": a test has failed; the remaining tests in this directory will be skipped.\n";
		  skip_remaining_tests = true;
		}
	    }
	}
    }
  if (!result.skipped.empty())
    {
      cout << subdir << ": " << result.skipped.size() << " tests have been skipped.\n";
    }
  if (!result.failures.empty())
    {
      cerr << subdir << ": ";
      RedText t(cerr);
      cerr << result.failures.size()
	   << " tests failed: "
	   << join_strings(result.failures, ", ")
	   << "\n";
      return false;
    }
  else if (!result.passes.empty())
    {
      cout << subdir << ": ";
      GreenText t(cerr);
      cout << "all " << result.passes.size() << " tests passed\n";
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
  int keep_going = 0;
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
      {
	"keep-going", false, &keep_going, 1,
      },
      {
	"nokeep-going", false, &keep_going, 0,
      },
    };
  bool bad_options = false;
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
  if (optind == argc)
    {
      cerr << "usage: " << program_name << " " << "[--capture|--nocapture] DIRECTORY [TEST...]\n";
      cerr << "You must specify the name of the directory in which to run tests.\n";
      return 1;
    }
  const char ** tail = const_cast<const char**>(&argv[optind+1]);
  if (run_tests(argv[optind],
		capture_output ? true : false,
		keep_going ? FailResponse::KeepGoing : FailResponse::StopImmediately,
		tail))
    {
      return 0;
    }
  else
    {
      return 1;
    }
}
