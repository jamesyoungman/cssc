/*
 * comments.cc: Part of GNU CSSC.
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
 *
 * CSSC was originally based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 */

#include <config.h>

#include "comments.h"		// declarations in this file.

#include <vector>
#include <string>

#include <string.h>

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
