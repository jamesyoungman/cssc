/*
 * do_cmd.cc: Part of GNU CSSC.
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

// Autotools requires us to include config.h first.
#include <config.h>

// C++ standard library header file includes
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

// C++ standard library headers corresponding to C standard library headers
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>

// POSIX header file includes
#include <unistd.h>
#include <sys/fcntl.h>

// gnulib header file includes
#include <error.h>
#include <regex.h>

// libcssc includes
#include "optional.h"

// Local includes
#include "echo_unescape.h"
#include "execute.h"
#include "filebody.h"

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::unique_ptr;
using cssc::optional;

static string program_name = "do_cmd";

const static vector<string> breadcrumb_files =
  {
    "last.command", "expected.stdout", "expected.stderr", "got.stdout", "got.stderr",
  };

static string get_expected_val (const string& expectation, bool is_file);
static void remove_breadcrumbs ();

class QualifiedLabel
{
public:
  QualifiedLabel(const string& test_name, const string& step_label)
    : qualified_label_(test_name + ":" + step_label)
  {
  }

  const string& str() const
  {
    return qualified_label_;
  }

  const char* c_str() const
  {
    return qualified_label_.c_str();
  }

private:
  string qualified_label_;
};

namespace {
  std::ostream& operator<< (std::ostream& stream, const QualifiedLabel& ql) {
    return stream << ql.str();
  }
}


// Open the file FILEMAME, write BODY to it, and close it.
//
// If CREATE_FILE is true, create the file if it does not already
// exist.  It is not an error for the file to already exist when
// CREATE_FILE is true.  If CREATE_FILE is false and the file does not
// already exist, fail.
//
// If APPEND is true, append the data at the end of the file.
// Otherwise, truncate the file and write the data at the satrt of the
// truncated file.
static void
write_string_to_file (const string& body,
		      const char *filename,
		      bool create_file,
		      bool append)
{
  int extra_flags = append ? O_APPEND : O_TRUNC;
  extra_flags |= create_file ? O_CREAT : 0;
  int fd = open (filename, O_WRONLY|extra_flags, 0400);
  if (fd < 1)
    {
      error (1, errno, "failed to open file %s", filename);
    }
  errno = 0;
  const ssize_t write_result = write (fd, body.c_str(), body.size());
  const int write_errno = errno;
  close (fd);
  if (write_result < 0)
    {
      error (1, write_errno, "failed to write to %s", filename);
    }
  const uintmax_t written = write_result;
  const uintmax_t wanted = body.length();
  if (written != wanted)
    {
      error (1, write_errno,
	     "short write on %s; tried to write %ju bytes, actually wrote %ju",
	     filename, wanted, written);
    }
}

static void
rewrite_file_body (const string& body,
		   const char *filename,
		   bool create_file)
{
  write_string_to_file (body, filename, create_file, false);
}

static string
join_string_vec(const vector<string>& v, const string& separator)
{
  string result;
  for (auto item : v)
    {
      if (!result.empty())
	{
	  result.append(separator);
	}
      result.append(item);
    }
  return result;
}

class MatchResult
{
public:
  MatchResult()
    : problems_()
  {
  }

  void add_problem(const std::string& message)
  {
    ASSERT(!message.empty());
    problems_.push_back(message);
  }

  bool ok() const
  {
    return problems_.empty();
  }

  string str() const
  {
    return join_string_vec(problems_, "\n");
  }

private:
  vector<string> problems_;
};


class MatchResults
{
public:
  MatchResults()
    : problems_()
  {
  }

  void add_match_result(const MatchResult match_result)
  {
    if (!match_result.ok())
      {
	problems_.push_back(match_result.str());
      }
  }

  bool ok() const
  {
    return problems_.empty();
  }

  string str() const
  {
    return join_string_vec(problems_, "\n");
  }

private:
  vector<string> problems_;
};

static MatchResult
check_regex_match (const string& pattern,
		   const string& actual)
{
  regex_t* re = static_cast<regex_t*> (malloc (sizeof(regex_t)));
  const int compile_result = regcomp (re, pattern.c_str(), REG_EXTENDED|REG_NOSUB);
  if (0 != compile_result)
    {
      char error_buf [512];
      regerror (compile_result, re, error_buf, sizeof(error_buf));
      error (1, 0, "regular expression %s is not valid: %s", pattern.c_str(), error_buf);
    }

  const int capture_limit = 1;
  regmatch_t matches[capture_limit];
  int match_result = regexec (re, actual.c_str(), capture_limit, matches, 0);

  regfree (re);
  free (re);

  MatchResult result;
  if (REG_NOMATCH == match_result)
    {
      std::stringstream ss;
      ss << "actual output did not match the specified regular expression; regular expression pattern was "
	 << pattern
	 << " but the actual output was "
	 << actual;
      result.add_problem (ss.str());
    }
  return result;
}

static MatchResult
check_literal_match (const string& pattern,
		     const string& actual)
{
  MatchResult result;
  if (actual != pattern)
    {
      std::stringstream ss;
      ss << "actual output did not match expected output\n"
	 << "expected output was:\n" << pattern << "\n"
	 << "actual   output was:\n" << actual;
      result.add_problem(ss.str());
    }
  return result;
}


enum class MatchType
  {
    Ignore,
    Literal,
    Regex,
  };

class OutputMatcher
{
public:
  OutputMatcher(MatchType match_type, string value, bool is_file)
    : match_type_(match_type),
      expected_(get_expected_val(value, is_file))

  {
    if (value ==  "IGNORE")
      {
	match_type_ = MatchType::Ignore;
      }
  }

  const string& expected() const
  {
    return expected_;
  }

  MatchResult check (const string& actual) const
  {
    switch (match_type_)
      {
      case MatchType::Ignore:
	{
	  return MatchResult();
	}
      case MatchType::Literal:
	{
	  return check_literal_match (expected_, actual);
	}
      case MatchType::Regex:
	{
	  return check_regex_match (expected_, actual);
	}
      default:
	{
	  int val = static_cast<int>(match_type_);
	  error (1, 0, "unexpected MatchType value %d", val);
	}
      }
  }

private:
  MatchType match_type_;
  string expected_;
};

class TestOutcome
{
public:
  TestOutcome(bool expect_failure,
	      vector<string> command,
	      const MatchResults match_results,
	      string child_error_output)
    : success_(match_results.ok()),
      command_(command),
      expect_failure_(expect_failure),
      message_(match_results.str()),
      child_error_output_(child_error_output)
  {
  }

  const string& message() const
  {
    return message_;
  }

  bool success() const
  {
    return success_;
  }

  const vector<string>& command() const
  {
    return command_;
  }

  const char* indicator() const
  {
    if (expect_failure_)
      {
	return success_ ? "XPASS" : "XFAIL";
      }
    return success_ ? "PASS" : "FAIL";
  }

  friend std::ostream& operator<< (std::ostream& stream, const TestOutcome& outcome) {
    const string& message = outcome.message();
    bool need_newline = message.empty();
    auto it = message.rbegin();
    if (it != message.rend())
      {
	need_newline = (*it) != '\n';
      }
    stream << outcome.indicator();
    if (!message.empty())
      {
	stream << ": " << message;
      }
    if (need_newline)
      {
	stream << "\n";
      }
    if (!outcome.success())
      {
	stream << "command line was: "
	       << join_string_vec(outcome.command(), " ")
	       << '\n';
	auto errors = outcome.child_error_output();
	if (errors.empty())
	  {
	    stream << "child process's standard error output was empty\n";
	  }
	else
	  {
	    stream << "child process's standard error output was:\n"
		   << errors;
	  }
      }
    return stream;
  }

  const string& child_error_output() const
  {
    return child_error_output_;
  }

private:
  bool success_;
  vector<string> command_;
  bool expect_failure_;
  string message_;
  string child_error_output_;
};


static int
configured_label_width()
{
  const int DEFAULT_WIDTH = 20;
  const char *env_val = getenv("CSSC_TEST_STEP_LABEL_MAX_WIDTH");
  if (env_val)
    {
      errno = 0;
      char *end = NULL;
      long val = strtol(env_val, &end, 10);
      if ((end && *end))
	return DEFAULT_WIDTH;	// not a number, or trailng text after the number
      if ((LONG_MAX == val || LONG_MIN == val) && errno)
	return DEFAULT_WIDTH;	// out of range for long
      if (val < 0 || val > INT_MAX)
	return DEFAULT_WIDTH;	// out of range for a field width
      return val;
    }
  return DEFAULT_WIDTH;
}

static MatchResult
check_retval_match (int got, const optional<int>& maybe_expected)
{
  MatchResult result;
  if (maybe_expected.has_value())
    {
      const int expected = maybe_expected.value();
      if (got != expected)
	{
	  std::stringstream ss;
	  ss << "expected child process exit value "
	     << expected
	     << " but got "
	     << got << '\n';
	  result.add_problem(ss.str());
	}
    }
  return result;
}

static unique_ptr<TestOutcome>
perform_test(bool expect_failure,
	     const string& command,
	     const optional<int>& expected_retval,
	     OutputMatcher stdout_matcher,
	     OutputMatcher stderr_matcher)
{
  MatchResults match_results;

  vector<string> args = vector<string>({"sh", "-c", command });
  auto result = execute_program ("sh", args, true);

  match_results.add_match_result (check_retval_match (result.retval, expected_retval));
  match_results.add_match_result (stdout_matcher.check (result.stdout_output));
  match_results.add_match_result (stderr_matcher.check (result.stderr_output));
  unique_ptr<TestOutcome> outcome (new TestOutcome (expect_failure,
						    args,
						    match_results,
						    result.stderr_output));
  if (outcome->success())
    {
      remove_breadcrumbs ();
    }
  else
    {
      rewrite_file_body (command, "last.command", true);
      rewrite_file_body (result.stdout_output, "got.stdout", true);
      rewrite_file_body (result.stderr_output, "got.stderr", true);
    }
  return outcome;
}

static bool
parse_expected_retval (const string& s, optional<int> *result)
{
  if (s == "IGNORE")
    {
      *result = optional<int>();
      return true;
    }
  char *end = NULL;
  long converted = strtol (s.c_str(), &end, 10);
  if (converted > 255 || converted < 0)
    {
      // out of range
      return false;
    }
  *result = static_cast<int>(converted);
  return true;
}



static string
get_expected_val_from_file_body (const char* filename)
{
  int fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      error (1, errno, "failed to open file %s", filename);
    }
  string body = read_file_body (fd, filename);
  close (fd);
  return body;
}

static string
get_expected_val_from_escaped_string (const string& expectation)
{
  // The extra byte of headroom here is for the null terminator
  // expected by echo_unescape() (which we may change to a newline).
  char *buf = new char[expectation.length() + 1];
  // c_str() will add a terminating NUL and we copy it.
  memcpy (buf, expectation.c_str(), expectation.length()+1);
  bool inhibit_newline = true;
  size_t output_len = echo_unescape (buf, buf, &inhibit_newline);
  if (!inhibit_newline)
    {
      buf[output_len++] = '\n';
    }
  string result = string(buf, output_len);
  delete[] buf;
  return result;
}



static string
get_expected_val (const string& expectation, bool is_file)
{
  if (is_file)
    {
      return get_expected_val_from_file_body (expectation.c_str());
    }
  else
    {
      return get_expected_val_from_escaped_string (expectation);
    }
}

static void
remove_breadcrumbs ()
{
  for (const auto& name : breadcrumb_files)
    {
      errno = 0;
      if (0 != unlink (name.c_str()))
	{
	  if (ENOENT != errno)
	    {
	      error (1, errno, "Failed to unlink %s", name.c_str());
	    }
	}
    }
}


static string
cmdline_description (int argc, char *argv[])
{
  std::stringstream ss;
  ss << "There were " << argc << " command-line arguments.\n";
  for (int i = 0; i < argc; ++i)
    {
      string arg(argv[i]);
      ss << "argv[" << i << "] = " << arg << " (length " << arg.length() << ")\n";
    }
  return ss.str();
}


int main(int argc, char *argv[])
{
  string cmdline_for_error_reporting = cmdline_description (argc, argv);

  if (argc > 0 && argv[0])
    {
      program_name = argv[0];
    }
  remove_breadcrumbs ();

  bool silent = false;
  MatchType stdout_match = MatchType::Literal;
  MatchType stderr_match = MatchType::Literal;
  bool stderr_expectation_is_file = false;
  bool stdout_expectation_is_file = false;
  bool expect_failure = false;
  optional<string> test_name;

  vector<string> positional_args;
  bool option = true;
  for (int i = 1; i < argc; ++i)
    {
      if (option)
	{
	  if (0 == strcmp("--", argv[i]))
	    {
	      option = false;
	      continue;
	    }
	  if (argv[i][0] == '-')
	    {
	      if (0 == strcmp("--silent", argv[i]))
		{
		  silent = true;
		}
	      else if (0 == strcmp("--nosilent", argv[i]))
		{
		  silent = false;
		}
	      else if (0 == strcmp("--stderr_regex", argv[i]))
		{
		  stderr_match = MatchType::Regex;
		}
	      else if (0 == strcmp("--nostderr_regex", argv[i]))
		{
		  stderr_match = MatchType::Literal;
		}
	      else if (0 == strcmp("--stderr_is_file", argv[i]))
		{
		  stderr_expectation_is_file = true;
		}
	      else if (0 == strcmp("--nostderr_is_file", argv[i]))
		{
		  stderr_expectation_is_file = false;
		}
	      else if (0 == strcmp("--stdout_regex", argv[i]))
		{
		  stdout_match = MatchType::Regex;
		}
	      else if (0 == strcmp("--nostdout_regex", argv[i]))
		{
		  stdout_match = MatchType::Literal;
		}
	      else if (0 == strcmp("--stdout_is_file", argv[i]))
		{
		  stdout_expectation_is_file = true;
		}
	      else if (0 == strcmp("--nostdout_is_file", argv[i]))
		{
		  stdout_expectation_is_file = false;
		}
	      else if (0 == strcmp("--test_name", argv[i]))
		{
		  if (argc > i)
		    {
		      test_name = argv[++i];
		    }
		  else
		    {
		      error (1, 0, "--test_name needs an argument");
		    }
		}
	      else if (0 == strcmp("--expect_failure", argv[i]))
		{
		  expect_failure = true;
		}
	      else if (0 == strcmp("--noexpect_failure", argv[i]))
		{
		  expect_failure = false;
		}
	      else
		{
		  error (1, 0, "unknown option %s", argv[i]);
		}
	      continue;
	    }
	  else
	    {
	      option = false;
	    }
	}
      // Process a non-option.
      positional_args.push_back(argv[i]);
    }

  if (!test_name.has_value())
    {
      error (1, 0, "Please specify --test_name");
    }
  if (positional_args.size() != 5)
    {
      error (1, 0,
	     "Bad command-line in test script %s. "
	     "Expected 5 positional arguments (label, command, "
	     "expected return value, expected stdout output, "
	     "expected stderr output), but got %d: %s\n"
	     "command-line was:\n%s",
	     test_name.value().c_str(),
	     static_cast<int>(positional_args.size()),
	     join_string_vec(positional_args, " ").c_str(),
	     cmdline_for_error_reporting.c_str());
    }
  const QualifiedLabel qualified_label(test_name.value(), positional_args[0]);
  unique_ptr<TestOutcome> outcome; // initially there is none
  const int label_width = configured_label_width ();
  auto status_update = [&silent, qualified_label, &outcome, label_width]()
  {
    std::ostream* output_dest = &std::cout;
    bool suppress_output = silent;
    if (outcome && !outcome->success())
      {
	suppress_output = false;
	output_dest = &std::cerr;
      }
    if (!suppress_output)
      {
	(*output_dest) << '\r' << std::setw(label_width) << std::left << qualified_label << "...";
	if (outcome)
	  {
	    (*output_dest) << *outcome;
	  }
      }
  };

  const string command = positional_args[1];
  optional<int> expected_retval;
  if (!parse_expected_retval (positional_args[2], &expected_retval))
    {
      error (1, 0, "%s: %s is not a valid expected return value expectation",
	     qualified_label.c_str(),
	     positional_args[2].c_str());
    }

  OutputMatcher stdout_matcher = OutputMatcher (stdout_match, positional_args[3], stdout_expectation_is_file);
  OutputMatcher stderr_matcher = OutputMatcher (stderr_match, positional_args[4], stderr_expectation_is_file);

  status_update ();
  outcome = perform_test (expect_failure, command, expected_retval,
			  stdout_matcher, stderr_matcher);
  if (!outcome->success())
    {
      // We only write these when a test step fails, for a small
      // performance improvement.
      rewrite_file_body (stdout_matcher.expected(), "expected.stdout", true);
      rewrite_file_body (stdout_matcher.expected(), "expected.stderr", true);
    }
  status_update ();
  return outcome->success() ? 0 : 1;
}
