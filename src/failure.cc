/*
 * failure.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 2019 Free Software Foundation, Inc.
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

#include "failure.h"

namespace
{
  constexpr int isit(cssc::error e)
  {
    return static_cast<int>(e);
  }
}  // unnamed namespace

namespace cssc
{
  std::string category_impl::message(int ev) const
  {
    switch (ev)
      {
      case isit(error::NotAnSccsHistoryFile):
	return "not an SCCS history file";
      case isit(error::UnexpectedEOF):
	return "unexpected end-of-file";
      case isit(error::FileHasHardLinks):
	return "refusing to open for writing a file with a link count greater than 1";
      case isit(error::BodyIsBinary):
	return "file body cannot be stored in an SCCS history file without encoding";
      default:
	return "unknown CSSC error";
      }
  }

  const std::error_category& cssc_category()
  {
    static category_impl instance;
    return instance;
  }

  std::error_code make_error_code(error e)
  {
    return std::error_code(static_cast<int>(e),
			   cssc_category());
  }

  std::error_condition make_error_condition(error e)
  {
    return std::error_condition(static_cast<int>(e),
				cssc_category());
  }

}  // namespace cssc
