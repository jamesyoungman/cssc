/*
 * execute.h: Part of GNU CSSC.
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
#ifndef INC_CSSC_TESTUTILS_EXECUTE_H
#define INC_CSSC_TESTUTILS_EXECUTE_H 1

#include <string>
#include <vector>

struct program_result
{
  bool success;
  std::string stdout_output;
  std::string stderr_output;
};

program_result execute_program(const std::string& program,
			       const std::vector<std::string>& args,
			       bool capture_output);

#endif
