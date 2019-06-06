/*
 * diff-state.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2002, 2007, 2008, 2009, 2010,
 *  2011, 2014, 2019 Free Software Foundation, Inc.
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
 * CSSC was originally based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 */
#include "config.h"

#include <limits>
#include <errno.h>

#include "cssc.h"
#include "diff-state.h"
#include "except.h"
#include "failure.h"


/* Quit with an appropriate error message when a read operation
   on the diff output fails. */
NORETURN
diff_state::diff_output_corrupt() {
        if (ferror(in_)) {
                fatal_quit(errno, "(diff output): Read error.");
        }
        fatal_quit(-1, "(diff output): Unexpected EOF.");
}


/* Quit with a cryptic error message indicating that something
   is wrong with the diff output. */

NORETURN
diff_state::diff_output_corrupt(const char *msg)
{
  fatal_quit(-1, "Diff output corrupt. (%s)", msg);
}


inline long get_num(const char *p,  char **endp)
{
  errno = 0;
  long val = strtol(p, endp, 10);
  if ((val == std::numeric_limits<long>::max()
       || val == std::numeric_limits<long>::min())
      && (errno != 0))
    {
      // We have a range error.
      throw CsscInternalRangeException(p);
    }
  return val;
}


/* Figure out what the new state should be by processing the
   diff output. */

inline void
diff_state::next_state()
{
  if (state_ == diffstate::DELETE && change_left_ != 0)
    {
      if (!read_line().ok())
        {
          diff_output_corrupt();
        }
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): read %s", linebuf_.c_str());
#endif

      if (strcmp(linebuf_.c_str(), "---\n") != 0)
        {
          diff_output_corrupt("expected ---");
        }
      lines_left_ = change_left_;
      change_left_ = 0;
      state_ = diffstate::INSERT;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning INSERT [1]\n");
#endif
      return;
    }

  if (state_ != diffstate::NOCHANGE)
    {
      if (!read_line().ok())
        {
          if (ferror(in_))
            {
              diff_output_corrupt();
            }
          state_ = diffstate::END;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning END [2]");
#endif
      return;
        }
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state()[3]: read %s", linebuf_.c_str());
#endif

      /* Ignore "\ No newline at end of file" if it appears
         at the end of the diff output. */

      if (linebuf_[0] == '\\')
        {
	  // if we can read a line, we weren't at EOF.
	  auto status = read_line();
          if (status.ok() || !cssc::isEOF(status))
            {
              diff_output_corrupt("Expected EOF");
            }
          if (ferror(in_))
            {
              diff_output_corrupt();
            }
          state_ = diffstate::END;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning END [4]\n");
#endif
          return;
        }

    }

  char *s = nullptr;
  long line1, line2, line3, line4;
  char c;

  line1 = get_num(linebuf_.c_str(), &s);
  line2 = line1;
  if (*s == ',')
    {
      line2 = get_num(s + 1, &s);
      if (line2 <= line1)
        {
          diff_output_corrupt("left end line");
        }
    }

  c = *s;

  ASSERT(c != '\0');

  if (c == 'a')
    {
      if (line1 >= in_lineno_)
        {
          state_ = diffstate::NOCHANGE;
          lines_left_ = line1 - in_lineno_ + 1;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning NOCHANGE [5]\n");
#endif
          return;
        }
      if (line1 + 1 != in_lineno_)
        {
          diff_output_corrupt("left start line [case 1]");
        }
    }
  else
    {
      if (line1 > in_lineno_)
        {
          state_ = diffstate::NOCHANGE;
          lines_left_ = line1 - in_lineno_;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning END\n");
#endif
          return;
        }
      if (line1 != in_lineno_)
        {
          diff_output_corrupt("left start line [case 2]");
        }
    }

  line3 = get_num(s + 1, &s);
  if (c == 'd')
    {
      if (line3 != out_lineno_)
        {
          diff_output_corrupt("right start line [case 1]");
        }
    }
  else
    {
      if (line3 != out_lineno_ + 1)
        {
          diff_output_corrupt("right start line [case 2]");
        }
    }

  line4 = line3;
  if (*s == ',')
    {
      line4 = get_num(s + 1, &s);
      if (line4 <= line3)
        {
          diff_output_corrupt("right end line");
        }
    }

  if (*s != '\n')
    {
      diff_output_corrupt("EOL");
    }

  switch (c)
    {
    case 'a':
      state_ = diffstate::INSERT;
      lines_left_ = line4 - line3 + 1;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning INSERT [6]\n");
#endif
      break;

    case 'd':
      state_ = diffstate::DELETE;
      lines_left_ = line2 - line1 + 1;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning DELETE [7]\n");
#endif
      break;

    case 'c':
      state_ = diffstate::DELETE;
      lines_left_ = line2 - line1 + 1;
      change_left_ = line4 - line3 + 1;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning DELETE [8]\n");
#endif
      break;

    default:
      diff_output_corrupt("unknown operation");
    }
}


/* Figure out whether a line is being inserted, deleted or left unchanged.
   Output new control lines accordingly. */

diffstate
diff_state::process(FILE *out, seq_no seq)
{
  if (state_ != diffstate::INSERT)
    {
      in_lineno_++;
    }

  if (state_ != diffstate::END)
    {
      ASSERT(lines_left_ >= 0);
      if (lines_left_ == 0)
        {
          if (state_ == diffstate::DELETE || state_ == diffstate::INSERT)
            {
              fprintf(out, "\001E %d\n", seq);
            }
          next_state();
          if (state_ == diffstate::INSERT)
            {
              fprintf(out, "\001I %d\n", seq);
            }
          else if (state_ == diffstate::DELETE)
            {
              fprintf(out, "\001D %d\n", seq);
            }
        }
      lines_left_--;
    }

  if (state_ == diffstate::DELETE)
    {
      if (!read_line().ok())
        {
          diff_output_corrupt();
        }
      if (linebuf_[0] != '<' || linebuf_[1] != ' ')
        {
          diff_output_corrupt("expected <");
        }
    }
  else
    {
      if (state_ == diffstate::INSERT)
        {
          if (!read_line().ok())
            {
              diff_output_corrupt();
            }
          if (linebuf_[0] != '>' || linebuf_[1] != ' ')
            {
              diff_output_corrupt("expected >");
            }
        }
      out_lineno_++;
    }

  return state_;
}
