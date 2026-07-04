/*
 * mr-list.cc: Part of GNU CSSC.
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 * Functions for non-destructively splitting a string into a list of
 * MRs. These functions were previously defined in l-split.cc.
 */
#include <config.h>

#include <string>
#include <vector>

#include <string.h>		// strlen, strtok, memcpy


#include "mr-list.h"		// declares these functions.

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
	  p = strtok(nullptr, delims);
	}
      delete[] s;
    }

  return mr_list;
}
