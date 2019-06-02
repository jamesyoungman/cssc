/*
 * base-reader.h: Part of GNU CSSC.
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
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 */
#ifndef CSSC__BASE_READER_H__
#define CSSC__BASE_READER_H__

#include <string.h>
#include <memory>

#include "failure.h"
#include "failure_or.h"
#include "linebuf.h"
#include "location.h"
#include "quit.h"

class sccs_file_reader_base
{
 public:
  explicit sccs_file_reader_base(const std::string& n, FILE *f, sccs_file_location pos)
    : f_(f),
      here_(pos),
    plinebuf{std::make_unique<cssc_linebuf>()}
  {}

  const std::string& name() const
    {
      return here_.name();
    }

  const sccs_file_location& here() const
  {
    return here_;
  };

  void check_arg() const
  {
    if (bufchar(2) != ' ')
      {
	corrupt(here(), "Missing arg");
      }
  }

  /* Checks the a control line has no arguments. */
  void check_noarg() const
  {
    if (bufchar(2) != '\0')
      {
	corrupt(here(), "Unexpected arg");
      }
  }

  cssc::FailureOr<char> read_line()
  {
    if (read_line_param())
      {
	if (ferror(f_))
	  {
	    errormsg_with_errno("%s: read error", name().c_str());
	  }
	return cssc::make_failure(cssc::errorcode::UnexpectedEOF);
      }

    if ( bufchar(0) == '\001')
      return bufchar(1);
    else
      return char(0);
  }

/*
 * Reads a line from the SCCS file.
 * Result:
 *   true if we read a line.   false for EOF or failure.
 * Output params:
 *   control_char: 0 if this is not a control (^A) line, otherwise the line type.
 */
  int read_line_param()
  {
    if (!plinebuf->read_line(f_).ok())
      {
	return 1;
      }
    here_.advance_line();
    // chomp the newline from the end of the line.
    // TODO: make me 8-bit clean!
    (*plinebuf)[strlen(plinebuf->c_str()) - 1] = '\0';
    return 0;
  }

  char bufchar(int pos) const
  {
    return (*plinebuf)[pos];
  }

 protected:
  void set_line_number(int num)
  {
    here_ = sccs_file_location(here_.name(), num);
  }

  std::unique_ptr<cssc_linebuf> plinebuf;
  sccs_file_location here_;

 private:
  FILE *f_;
};

unsigned short strict_atous(const sccs_file_location&, const char *s);


#endif /* CSSC__BASE_READER_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
