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

#include <sstream>
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

    bool equivalent(const std::error_code& code,
		    int condition) const noexcept override;
  };

  const std::error_category& cssc_category();

  // errorcode is for storing in std::error_code
  enum class errorcode
    {
      NotAnSccsHistoryFile = 1000,
      NotAnSccsHistoryFileName,
      CannotEditBitkeeperFile,
      UnexpectedEOF,
      FileHasHardLinks,
      ControlCharacterAtStartOfLine,
      FileDoesNotEndWithNewline,
      BodyLineTooLong,
      LockNotHeld,
      DeclineToOverwriteOutputFile,
      // TODO: GetFileBodyFailed should be temporary, because it is too vague.
      GetFileBodyFailed,
    };

  // condition is for storing in std::error_condition
  enum class condition
    {
      BodyIsBinary = 50000
    };

  class Failure
  {
  public:
    explicit Failure(std::error_code ec)
      : code_(ec) {}
    explicit Failure(std::error_code ec, const std::string& detail)
      : code_(ec), detail_(detail) {}

    // The default constructor signals "OK".
    Failure()
    {
      ASSERT(ok());
    }

    // We deliberately do not have an implicit cast to bool so that we
    // don't end up in a situation where changing "bool foo()" to
    // "Failure foo()" results in the opposite interpretation.  This is
    // what happens if you use a std::error_code directly.
    bool ok() const
    {
      return !code_;
    }

    std::error_code code() const
    {
      return code_;
    }

    std::string to_string() const;

    // Most callers should call to_string() since it includes the
    // detailed error message from the std::error_code too.  The
    // detail() method is mainly intended for use in
    // FailureBuilder::FailureBuilder(const Failure&).
    const std::string& detail() const;

    static Failure Ok()
    {
      return Failure(std::error_code());
    }

  private:
    std::error_code code_;
    std::string detail_;
  };

  inline Failure Update(const Failure& orig, const Failure& next)
  {
    return orig.ok() ? next : orig;
  }

  std::error_condition make_error_condition(condition e);
  std::error_code make_error_code(errorcode e);

  inline Failure make_failure(errorcode e)
  {
    return Failure(std::error_code(static_cast<int>(e), cssc_category()));
  }

  inline Failure make_failure(errorcode e, const std::string& detail)
  {
    return Failure(std::error_code(static_cast<int>(e), cssc_category()),
		   detail);
  }

  inline Failure make_failure_from_errno(int errno_val)
  {
    ASSERT(errno_val != 0);
    return Failure(std::error_code(static_cast<int>(errno_val), std::generic_category()));
  }

  inline Failure make_failure_from_errno(int errno_val, const std::string& detail)
  {
    ASSERT(errno_val != 0);
    return Failure(std::error_code(static_cast<int>(errno_val), std::generic_category()),
		   detail);
  }

  class FailureBuilder
  {
  public:
    explicit FailureBuilder(std::error_code ec);
    FailureBuilder(const Failure& f);
    FailureBuilder(const FailureBuilder& other);
    operator Failure() const;
    ~FailureBuilder();
    FailureBuilder& diagnose();

    template <typename T> FailureBuilder& operator<<(const T& item)
    {
      os_ << item;
      detail_ = true;
      return *this;
    }

  protected:
    Failure build() const;

  private:
    std::ostringstream os_;
    std::error_code code_;
    bool diagnose_;
    bool detail_;
  };

  FailureBuilder make_failure_builder(errorcode e);
  FailureBuilder make_failure_builder(const Failure&);

  FailureBuilder make_failure_builder_from_errno(int errno_val);

  inline bool isEOF(const Failure& f)
  {
    const auto code = f.code();
    return code.category() == cssc_category() &&
      code.value() == static_cast<int>(errorcode::UnexpectedEOF);
  }

}  // namespace cssc

namespace std
{
  template <>
    struct is_error_code_enum<cssc::errorcode>
    : public true_type {};

  template <>
    struct is_error_condition_enum<cssc::condition>
    : public true_type {};
}
#endif /* CSSC__FAILURE_H__*/

/* Local variables: */
/* mode: c++ */
/* End: */
