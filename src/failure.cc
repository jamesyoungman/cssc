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
#include "quit.h"

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
      case isit(error::NotAnSccsHistoryFileName):
	return "not an SCCS history file name";
      case isit(error::UnexpectedEOF):
	return "unexpected end-of-file";
      case isit(error::FileHasHardLinks):
	return "refusing to open for writing a file with a link count greater than 1";
      case isit(error::BodyIsBinary):
	return "file body cannot be stored in an SCCS history file without encoding";
      case isit(error::LockNotHeld):
	return "cannot continue without holding the lock on the SCCS file";
      case isit(error::DeclineToOverwriteOutputFile):
	return "refusing to overwrite output file";
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

  std::string Failure::to_string() const
  {
    if (detail_.empty())
      return code_.message();
    else
      return detail_ + "; " + code_.message();
  }

  const std::string& Failure::detail() const
  {
    return detail_;
  }


  FailureBuilder::FailureBuilder(std::error_code ec)
    : code_(ec),
      diagnose_(false),
      detail_(false)
  {
  }

  FailureBuilder::operator Failure() const
  {
    return build();
  }

  FailureBuilder& FailureBuilder::diagnose()
  {
    diagnose_ = true;
    return *this;
  }

  FailureBuilder::FailureBuilder(error e)
    : code_(static_cast<int>(e), cssc_category()),
      diagnose_(false),
      detail_(false)
  {
  }

  FailureBuilder::FailureBuilder(const FailureBuilder& other)
      : code_(other.code_),
	diagnose_(other.diagnose_),
	detail_(other.detail_)
    {
      os_ << other.os_.str();
    }

  FailureBuilder::FailureBuilder(const Failure& f)
    : code_(f.code()),
      diagnose_(false),
      detail_(f.detail().empty())
    {
      os_ << f.detail();
    }

  FailureBuilder::~FailureBuilder()
  {
    if (diagnose_)
      {
	errormsg("%s", build().to_string().c_str());
      }
  }

  Failure FailureBuilder::build() const
  {
    return detail_ ? Failure(code_, os_.str()) : Failure(code_);
  }

  FailureBuilder make_failure_builder_from_errno(int errno_val)
  {
    std::error_code ec(errno_val, std::generic_category());
    return FailureBuilder(ec);
  }

}  // namespace cssc
