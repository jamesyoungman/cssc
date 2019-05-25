/*
 * failure_or.h: Part of GNU CSSC.
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
#ifndef CSSC__FAILURE_OR_H__
#define CSSC__FAILURE_OR_H__

#include <stdexcept>
#include <system_error>

#include "failure.h"

namespace cssc
{
  class EmptyFailureOr : public std::logic_error {
  public:
    explicit EmptyFailureOr(const std::string& msg)
      : std::logic_error(msg) {}
    explicit EmptyFailureOr(const char* msg)
      : std::logic_error(msg) {}
  };

  class NonEmptyFailureOr : public std::logic_error {
  public:
    explicit NonEmptyFailureOr(const std::string& msg)
      : std::logic_error(msg) {}
    explicit NonEmptyFailureOr(const char* msg)
      : std::logic_error(msg) {}
  };

// FailureOr is something of a reinvented wheel.  I decided not to try
// to use Boost.Outcome because it requires a very recent (as of
// mid-2019) version of GCC.  I decided not to use
// https://github.com/oktal/result because (at version
// fee9af7e0c775f0148810f9449fa8354b3eec569 at least) it seemed not to
// be able to hold a std::unique_ptr conveniently.
template <class T>
class FailureOr
{
 public:
  FailureOr(const T& val)
    : value_(val),
      fail_(Failure::Ok())
  {
  }

  FailureOr(T&& val)
    : value_(std::move(val)),
      fail_(Failure::Ok())
  {
  }

  FailureOr(Failure fail)
    : value_(), fail_(fail)
  {
  }

  FailureOr(FailureBuilder f)
    : value_(),
      fail_(f)
  {
  }

  constexpr bool ok()  const
  {
    return fail_.ok();
  }

  void assert_ok() const
  {
    if (!ok())
      {
	throw EmptyFailureOr("assert_ok() on an empty FailureOr instance");
      }
  }

  const T& operator*() const
  {
    assert_ok();
    return value_;
  }

  T& operator*()
  {
    assert_ok();
    return value_;
  }

  constexpr const Failure& fail()  const
  {
    if (ok())
      {
	throw NonEmptyFailureOr("call to fail() on a non-empty FailureOr instance");
      }
    return fail_;
  }

  constexpr std::error_code code()  const
  {
    auto code = fail_.code();
    if (!code)
      {
	throw NonEmptyFailureOr("call to code() on a non-empty FailureOr instance");
      }
    return code;
  }

  std::string to_string() const
  {
    return fail_.to_string();
  }

  const char * c_str() const
  {
    return to_string().c_str();
  }

 private:
  T value_;
  Failure fail_;
};

}  // namespace cssc

#endif /* CSSC__FAILURE_OR_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
