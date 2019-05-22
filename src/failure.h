/*
 * failure.h: Part of GNU CSSC.
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
#ifndef CSSC__FAILURE_H__
#define CSSC__FAILURE_H__

#include "cssc-assert.h"
#include <system_error>
#include <type_traits>

namespace cssc
{
  class category_impl : public std::error_category
  {
  public:
    const char* name() const noexcept override
    {
      return "cssc";
    }

    std::string message(int ev) const override;

    // None of our errors are equivalent to any of the errors in
    // std::errc, so we define no conversions.
  };

  const std::error_category& cssc_category();

  enum class error
    {
      NotAnSccsHistoryFile = 1000,
        UnexpectedEOF,
        FileHasHardLinks,
    };

  inline std::error_code make_error(error e)
    {
      return std::error_code(static_cast<int>(e), cssc_category());
    }

  inline std::error_code make_error_from_errno(int errno_val)
  {
    ASSERT(errno_val != 0);
    return std::error_code(static_cast<int>(errno_val), std::generic_category());
  }

}  // namespace cssc

namespace std
{
  template <>
    struct is_error_code_enum<cssc::error>
    : public true_type {};
}

#endif /* CSSC__FAILURE_H__*/

/* Local variables: */
/* mode: c++ */
/* End: */
