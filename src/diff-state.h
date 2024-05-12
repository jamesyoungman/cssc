/*
 * diff-state.h: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2007,
 *  2008, 2009, 2010, 2011, 2014, 2019, 2024 Free Software Foundation,
 *  Inc.
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
#ifndef CSSC__DIFF_STATE_H
#define CSSC__DIFF_STATE_H

#include <cstdio>

#include "defaults.h"
#include "delta.h"
#include "failure.h"
#include "linebuf.h"

enum class diffstate { START, NOCHANGE, DELETE, INSERT, END };

class diff_state
{
private:
  diffstate state_;
  int in_lineno_;
  int out_lineno_;
  int lines_left_;
  int change_left_;
  bool echo_diff_output_;

  FILE *in_;
  cssc_linebuf linebuf_;

  NORETURN diff_output_corrupt() POSTDECL_NORETURN;
  NORETURN diff_output_corrupt(const char *msg) POSTDECL_NORETURN;

  void next_state();
  cssc::Failure read_line()
    {
      cssc::Failure bad = linebuf_.read_line(in_);

      // If and only if we read in a new line, echo it.
      if (echo_diff_output_ && bad.ok())
        {
	  printf("%s", linebuf_.c_str());
        }
      return bad;
    }

  // Prohibit copying, to avoid two objects being able to consume data
  // from the same input.
  diff_state& operator=(const diff_state&) = delete;
  diff_state(const diff_state&) = delete;

public:
  diff_state(FILE *f, bool echo)
    : state_(diffstate::START),
      in_lineno_(0L), out_lineno_(0L),
      lines_left_(0), change_left_(0),
      echo_diff_output_(echo),
      in_(f),
      linebuf_()
    {
    }

  diffstate process(FILE *out, seq_no seq);

  const char *
  get_insert_line()
    {
      ASSERT(state_ == diffstate::INSERT);
      ASSERT(linebuf_[0] == '>' && linebuf_[1] == ' ');
      return linebuf_.c_str() + 2;
    }

  long in_line() { return in_lineno_; }
  long out_line() { return out_lineno_; }
};

#endif /* CSSC__DIFF_STATE_H */

/* Local variables: */
/* mode: c++ */
/* End: */
