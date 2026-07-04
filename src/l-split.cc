/*
 * l-split.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 2007, 2009, 2010, 2011, 2014, 2019, 2024,
 *  2026 Free Software Foundation, Inc.
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
 * Functions for non-destructively splitting a string into a list of
 * strings.
 *
 */

#include <config.h>

#include <errno.h>

#include "cssc.h"
#include "sccsfile.h"		// declares these functions.
#include "l-split.h"		// declares these functions.

#include <string.h>
#include <string>


class FileCloser
{
public:
  explicit FileCloser(FILE* fp)
    : f_(fp) {}

  ~FileCloser()
  {
    fclose(f_);
    f_ = nullptr;
  }

  // We don't want the file doubly closed,
  // so prohibit copying.
  FileCloser(const FileCloser&) = delete;
  FileCloser& operator=(const FileCloser&) = delete;

private:
  FILE* f_;
};

cssc::FailureOr<std::vector<std::string>>
read_file_lines(const char* file_name)
{
  std::vector<std::string> tmp;
  errno = 0;
  FILE *f = fopen(file_name, "r");
  if (f == nullptr)
    {
      return cssc::make_failure_builder_from_errno(errno)
	<< "failed to read from " << file_name;
    }
  FileCloser closer(f);

  int ch;
  std::string s;
  bool empty = true;
  while ((ch=getc(f)) != EOF)
    {
      if (ch == '\n')
	{
	  tmp.push_back(s);
	  s.clear();
	  empty = true;
	}
      else
	{
	  s.push_back(static_cast<char>(ch));
	  empty = false;
	}
    }
  if (!empty)
    {
      tmp.push_back(s);
    }
  if (ferror(f))
    {
      return cssc::make_failure_builder_from_errno(errno)
	<< "failed to read from " << file_name;
    }
  return tmp;
}


std::string::const_iterator
split_string(std::string::const_iterator first, std::string::const_iterator last,
	     char delimiter, std::vector<std::string>* output,
	     std::string::size_type field_limit)
{
  bool empty = false;			// when true, current is empty
  std::string current;
  current.reserve(last - first);
  auto emit = [output, &current, &field_limit]()
    {
      output->push_back(current);
      current.clear();
      --field_limit;
    };

  while (field_limit > 0u)
    {
      char ch;
      if (first == last)
	{
	  ch = delimiter;
	  emit();
	  empty = true;
	}
      else
	{
	  ch = *first++;
	  if (ch == delimiter)
	    {
	      emit();
	      empty = false;
	    }
	  else
	    {
	      current.push_back(ch);
	    }
	}
      if (first == last)
	{
	  break;
	}
    }
  if (field_limit > 0u && !empty)
    {
      emit();
    }
  return first;
}

/* Local variables: */
/* mode: c++ */
/* End: */
