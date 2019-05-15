/*
 * l-split.h: Part of GNU CSSC.
 *
 *  Copyright (C) 2016, 2019 Free Software Foundation, Inc.
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
 *
 * Functions for spliting strings.
 *
 */
#ifndef CSSC_L_SPLIT_H
#define CSSC_L_SPLIT_H

#include <limits>
#include <vector>
#include <string>
#include <utility>

std::vector<std::string> split_mrs(const std::string& mrs);
std::vector<std::string> split_comments(const std::string& comments);


std::string::const_iterator
split_string(std::string::const_iterator first, std::string::const_iterator last,
	     char delimiter, std::string::size_type max_fields, std::vector<std::string>* output);
std::string::const_iterator
split_string(std::string::const_iterator first, std::string::const_iterator last,
	     char delimiter, std::vector<std::string>* output);

// Read a file, split it into lines and return them.  If successful, result.first
// is zero and the lines are in result.second.  On failure, result.first holds the
// resulting value of errno.
std::pair<int, std::vector<std::string>> read_file_lines(const char* file_name);

#endif
