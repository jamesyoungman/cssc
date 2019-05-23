/*
 * parser.h: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2007,
 *  2008, 2009, 2010, 2011, 2014, 2019 Free Software Foundation, Inc.
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
 */
#ifndef CSSC__PARSER_H__
#define CSSC__PARSER_H__

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base-reader.h"
#include "body-scanner.h"
#include "failure.h"
#include "failure_or.h"
#include "optional.h"
#include "location.h"
#include "mode.h"

class cssc_linebuf;
class cssc_delta_table;
class delta;

struct parsed_flag
{
  parsed_flag(const sccs_file_location& loc, char f, const std::string& v)
    : where(loc), letter(f), value{cssc::optional<std::string>(v)} {}
  parsed_flag(const sccs_file_location& loc, char f)
    : where(loc), letter(f) {}

  sccs_file_location where;
  char letter;
  cssc::optional<std::string> value;
};

class ParserOptions
{
public:
  explicit ParserOptions()
  : silent_checksum_error_(false)
  {
  }

  ParserOptions(const ParserOptions& other) = default;

  ParserOptions& set_silent_checksum_error(bool state)
  {
    silent_checksum_error_ = state;
    return *this;
  }

  bool silent_checksum_error() const
  {
    return silent_checksum_error_;
  }

private:
  bool silent_checksum_error_;
};


class sccs_file_parser : public sccs_file_reader_base
{
private:
  struct constructor_cookie {};
  using string = std::string;

public:
  static ParserOptions Defaults()
  {
    return ParserOptions();
  }

  struct open_result
  {
    template <class T> using optional = cssc::optional<T>;
    std::unique_ptr<sccs_file_parser> parser;

    int computed_sum;		// computed from reading the (whole) file.
    int stored_sum;		// from the header
    // if checksum_valid is false, stored_sum is either uninitialised
    // (e.g. malformed header line) or does not equal computed_sum.
    bool checksum_valid_;
    bool is_bk;
    bool is_executable;
    std::unique_ptr<cssc_delta_table> delta_table;
    std::vector<string> users;
    std::vector<parsed_flag> flags;
    std::vector<std::string> comments;
    std::unique_ptr<sccs_file_body_scanner> body_scanner;
  };

  // Open an SCCS file.  Result is null on failure.
  static cssc::FailureOr<std::unique_ptr<open_result> >
  open_sccs_file(const string& name, sccs_file_open_mode, ParserOptions);

  NORETURN corrupt_file(const char *fmt, ...) const POSTDECL_NORETURN;
  void saw_unknown_feature(const char *fmt, ...) const;

  // The purpose of the constructor_cookie is to allow make_unique to
  // use a public constructor without allowing make_unique to be used
  // outside the class.
  sccs_file_parser(const string& name, sccs_file_open_mode, FILE *f, constructor_cookie c);

protected:
  std::unique_ptr<sccs_file_parser::open_result>
  parse_header(FILE*, ParserOptions);


  std::unique_ptr<delta> read_delta();
  unsigned long strict_atoul_idu(const sccs_file_location& loc, const char *s) const;
  void check_bk_comment(char ch, char arg) const;

private:
  sccs_file_parser& operator=(const sccs_file_parser&) = delete;

  sccs_file_open_mode mode_;
  bool is_bk_file_;
};


#endif /* CSSC__PARSER_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
