/*
 * diff-state.h: Part of GNU CSSC.
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
#ifndef CSSC__DIFF_STATE_H
#define CSSC__DIFF_STATE_H

#include <stdio.h>

#include "defaults.h"
#include "delta.h"
#include "linebuf.h"

class diff_state
{
public:
  enum state { START, NOCHANGE, DELETE, INSERT, END };

private:
  enum state _state;
  int in_lineno, out_lineno;
  int lines_left;
  int change_left;
  bool echo_diff_output;

  FILE *in;
  cssc_linebuf linebuf;

  NORETURN diff_output_corrupt() POSTDECL_NORETURN;
  NORETURN diff_output_corrupt(const char *msg) POSTDECL_NORETURN;

  void next_state();
  int read_line()
    {
      int read_ret = linebuf.read_line(in);

      if (echo_diff_output)
        {
          // If and only if we read in a new line, echo it.
          if (0 == read_ret)
            {
              printf("%s", linebuf.c_str());
            }
        }
      return read_ret;
    }

public:
  diff_state(FILE *f, bool echo)
    : _state(START),
      in_lineno(0), out_lineno(0),
      lines_left(0), change_left(0),
      echo_diff_output(echo),
      in(f)
    {
    }

  enum state process(FILE *out, seq_no seq);

  const char *
  get_insert_line()
    {
      ASSERT(_state == INSERT);
      ASSERT(linebuf[0] == '>' && linebuf[1] == ' ');
      return linebuf.c_str() + 2;
    }

  int in_line() { return in_lineno; }
  int out_line() { return out_lineno; }
};

#endif /* CSSC__DIFF_STATE_H */

/* Local variables: */
/* mode: c++ */
/* End: */
