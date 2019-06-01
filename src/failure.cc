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
  constexpr int isit(cssc::errorcode e)
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
      case isit(errorcode::NotAnSccsHistoryFile):
	return "not an SCCS history file";
      case isit(errorcode::NotAnSccsHistoryFileName):
	return "not an SCCS history file name";
      case isit(errorcode::CannotEditBitkeeperFile):
	return "editing BitKeeper files is currently not supported";
      case isit(errorcode::UnexpectedEOF):
	return "unexpected end-of-file";
      case isit(errorcode::FileHasHardLinks):
	return "refusing to open for writing a file with a link count greater than 1";
      case isit(errorcode::ControlCharacterAtStartOfLine):
	return "file body cannot be stored in an SCCS history file without encoding, because there is a control character (ASCII code 1) at the start of a line";
      case isit(errorcode::BodyLineTooLong):
	return "file body cannot be stored in an SCCS history file without encoding, because it contains a very long line";
      case isit(errorcode::FileDoesNotEndWithNewline):
	return "file body cannot be stored in an SCCS history file without encoding, because it does not end with newline";
      case isit(errorcode::LockNotHeld):
	return "cannot continue without holding the lock on the SCCS file";
      case isit(errorcode::DeclineToOverwriteOutputFile):
	return "refusing to overwrite output file";
      case isit(errorcode::InternalErrorNoEncodedFlagFound):
	return "internal error: failed to find encoded flag to adjust it in the output file";
      case isit(errorcode::DeclineToCreateHistoryFileThatAlreadyExists):
	return "refusing to create a history file because it already exists";
      case isit(errorcode::UsagePreconditionFailureSidNotFound):
	return "the selected revision is not present in the history file";
      case isit(errorcode::UsagePreconditionFailureDeltaHasSuccessor):
	return "the selected revision cannot be removed as it has a successor in the history file";
      case isit(errorcode::UsagePreconditionFailureDeltaInUse):
	return "the selected revision cannot be removed as it is referred to by another delta in the history file";
      case isit(errorcode::HistoryFileCorrupt):
	return "format/parsing error in history file";
      default:
	return "unknown CSSC error";
      }
  }

  std::error_code make_error_code(errorcode e)
  {
    return std::error_code(static_cast<int>(e),
			   cssc_category());
  }

  std::error_condition make_error_condition(condition e)
  {
    return std::error_condition(static_cast<int>(e),
				cssc_category());
  }

  bool category_impl::equivalent(const std::error_code& code, int condition) const noexcept
  {
    if (static_cast<int>(condition::BodyIsBinary) == condition)
      {
	if (code.category() != cssc_category())
	  {
	    return false;
	  }

	switch (code.value())
	  {
	  case isit(errorcode::ControlCharacterAtStartOfLine):
	  case isit(errorcode::BodyLineTooLong):
	  case isit(errorcode::FileDoesNotEndWithNewline):
	    return true;
	  default:
	    return false;
	  }
      }
  }


  const std::error_category& cssc_category()
  {
    static category_impl instance;
    return instance;
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

  // This doesn't do anything that you can't do by invoking the
  // FailureBuilder constructor, but the consistency of style in the
  // other factory functions leads one to assume this function would
  // also exist.
  FailureBuilder make_failure_builder(const cssc::Failure& f)
  {
    return FailureBuilder(f);
  }

  // This also is just here for consistency.
  FailureBuilder make_failure_builder(errorcode e)
  {
    return FailureBuilder(e);
  }

}  // namespace cssc
