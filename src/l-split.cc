/*
 * l-split.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 2007, 2009, 2010, 2011, 2014, 2019 Free
 *  Software Foundation, Inc.
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
 * Functions for non-destructively spliting a string into a list of
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


std::vector<std::string>
split_mrs(const std::string& mrs)
{
  std::vector<std::string> mr_list;
  const char *delims = " \t\n";

  if (!mrs.empty())
    {
      char *s = new char[strlen(mrs.c_str()) + 1];
      memcpy( s, mrs.c_str(), strlen(mrs.c_str()) + 1);
      char *p = strtok(s, delims);

      while (p)
	{
	  mr_list.push_back(std::string(p));
	  p = strtok(NULL, delims);
	}
      delete[] s;
    }

  return mr_list;
}

// TODO: write a unit test for this, then update it to use something
// more appropriate than new char[].  Perhaps also use std::find or
// similar rather than strchr().
std::vector<std::string>
split_comments(const std::string& comments)
{
  std::vector<std::string> comment_list;

  if (!comments.empty())
    {
      char *s = new char[strlen(comments.c_str()) + 1];
      memcpy( s, comments.c_str(), strlen(comments.c_str()) + 1);

      char* start = s;
      char* end = strchr(s, '\n');
      while (end != NULL)
	{
	  *end++ = '\0';
	  comment_list.push_back(start);
	  start = end;
	  end = strchr(start, '\n');
	}

      if (*start != '\0')
	{
	  comment_list.push_back(start);
	}

      delete[] s;
    }

  return comment_list;
}

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

private:
  FILE* f_;
};

std::pair<int, std::vector<std::string>>
read_file_lines(const char* file_name)
{
  std::vector<std::string> result, tmp;
  errno = 0;
  FILE *f = fopen(file_name, "r");
  if (f == nullptr)
    {
      return std::make_pair(errno, result);
    }
  FileCloser closer(f);

  int ch;
  string s;
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
      return std::make_pair(errno, result);
    }
  std::swap(tmp, result);
  return std::make_pair(errno, result);
}



/* Local variables: */
/* mode: c++ */
/* End: */
