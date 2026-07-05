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

static void
append_string_to_file (const string& data,
		       const char *filename)
{
  write_string_to_file (data, filename, true, true);
}

static bool
check_regex_match (const string& pattern,
		   const string& actual,
		   std::stringstream& problems)
{
  char tmp_file_name[] = "/tmp/do_cmd_rx.XXXXXX";
  if (mkstemp (tmp_file_name)  < 0)
    {
      error (1, errno, "failed to create temporary file from template %s", tmp_file_name);
    }
  rewrite_file_body (actual, tmp_file_name, false);
  const vector<string> grep_args =
    {
      "-e", pattern, tmp_file_name
    };
  // Capture stdout so that we don't see output from successful tests.
  const auto grep_result = execute_program ("grep", grep_args, true);
  bool happy = (0 == grep_result.retval);
  if (!happy)
    {
      problems << "actual output did not match the specified regular expression";
    }
  if (0 != unlink (tmp_file_name))
    {
      happy = false;
      problems << "failed to remove temporary file " << tmp_file_name;
    }
  return happy;
}

static bool
check_literal_match (const string& pattern,
		     const string& actual,
		     std::stringstream& problems)
{
  if (actual == pattern)
    {
      return true;
    }
  problems << "actual output did not match expected output\n";
  problems << "expected output was:\n" << pattern << "\n";
  problems << "actual   output was:\n" << actual << "\n";
  return false;
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

  bool check (const string& actual, std::stringstream& problems) const
  {
    switch (match_type_)
      {
      case MatchType::Ignore:
	{
	  return true;
	}
      case MatchType::Literal:
	{
	  return check_literal_match (expected_, actual, problems);
	}
      case MatchType::Regex:
	{
	  return check_regex_match (expected_, actual, problems);
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
  TestOutcome(bool expect_failure, bool success, string message, string child_error_output)
    : success_(success),
      expect_failure_(expect_failure),
      message_(message),
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
    return stream;
  }

  const string& child_error_output() const
  {
    return child_error_output_;
  }

private:
  bool success_;
  bool expect_failure_;
  string message_;
  string child_error_output_;
};

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

static bool
retval_matches (int got, const optional<int>& maybe_expected, std::stringstream& problems)
{
  if (!maybe_expected.has_value())
    {
      return true;
    }
  const int expected = maybe_expected.value();
  if (got == expected)
    {
      return true;
    }
  problems << "expected child process exit value "
	   << expected
	   << " but got "
	   << got << '\n';
  return false;
}

static unique_ptr<TestOutcome>
perform_test(bool expect_failure,
	     const string& command,
	     const optional<int>& expected_retval,
	     OutputMatcher stdout_matcher,
	     OutputMatcher stderr_matcher)
{
  append_string_to_file (command + "\n", "last.command");

  auto result = execute_program ("sh", vector<string>({"sh", "-c", command }), true);

  rewrite_file_body (result.stdout_output, "got.stdout", true);
  rewrite_file_body (result.stderr_output, "got.stderr", true);

  std::stringstream problems;
  if (!retval_matches (result.retval, expected_retval, problems))
    {
      if (result.stderr_output.empty())
	{
	  problems << "standard error output was empty\n";
	}
      else
	{
	  problems << "standard error output was:\n"  << result.stderr_output;
	}
      return unique_ptr<TestOutcome>(new TestOutcome (expect_failure, false, problems.str(),
						      result.stderr_output));
    }
  if (!stdout_matcher.check (result.stdout_output, problems))
    {
      return unique_ptr<TestOutcome>(new TestOutcome (expect_failure, false, problems.str(),
					 result.stderr_output));
    }
  if (!stderr_matcher.check (result.stderr_output, problems))
    {
      return unique_ptr<TestOutcome>(new TestOutcome (expect_failure, false, problems.str(),
						      result.stderr_output));
    }
  return unique_ptr<TestOutcome>(new TestOutcome (expect_failure, true, "", result.stderr_output));
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
  rewrite_file_body (stdout_matcher.expected(), "expected.stdout", true);
  rewrite_file_body (stdout_matcher.expected(), "expected.stderr", true);

  status_update ();
  outcome = perform_test (expect_failure, command, expected_retval,
			  stdout_matcher, stderr_matcher);

  status_update ();
  remove_breadcrumbs ();
  return outcome->success() ? 0 : 1;
}
