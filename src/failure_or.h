/*
 * failure_or.h: Part of GNU CSSC.
 *
 *  Copyright (C) 2019, 2024 Free Software Foundation, Inc.
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

#include <memory>
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
    // You should not create a FailureOr<T> from an instance of
    // Failure which does not actually represent an error, since this
    // leaves the FailureOr instance empty with a default-constructed
    // T.
    ASSERT(!fail.ok());
  }

  FailureOr(FailureBuilder f)
    : value_(),
      fail_(f)
  {
  }

  // We define the copy constructor in case T is actually a pointer
  // type.  FailureOr does not own *value_ if T is a pointer.
  FailureOr(const FailureOr& source)
    : value_(source.value_),
      fail_(source.fail_)
  {
  }

  // Alow copying where T is std::unique_ptr<Q>.
  FailureOr(FailureOr&& source)
    : value_(std::move(source.value_)),
      fail_(source.fail_)
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

  FailureOr& operator=(const FailureOr& other)
  {
    value_ = other.value_;
    fail_ = other.fail_;
    return *this;
  }

  FailureOr& operator=(const FailureOr&& other) noexcept
  {
    value_ = std::move(other.value_);
    fail_ = other.fail_;
    return *this;
  }

  constexpr const Failure& fail()  const
  {
    // We allow calls to fail() on OK instances so allow this pattern:
    // cssc::FailureOr<int> DoSomething();
    // auto done = DoSomething();
    // f = cssc::Update(f, done.fail());
    return fail_;
  }

  constexpr std::error_code code()  const
  {
    return fail_.code();
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

namespace std
{
  template <class T>
  void swap(cssc::FailureOr<T>& a, cssc::FailureOr<T>& b)
  {
    std::swap(a.value_, b.value_);
    std::swap(a.fail_, b.fail_);
  }
}  // namespace std


#endif /* CSSC__FAILURE_OR_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
