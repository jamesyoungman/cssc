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
#include "diff-state.h"
#include "cssc.h"


/* Quit with an appropriate error message when a read operation
   on the diff output fails. */
NORETURN
diff_state::diff_output_corrupt() {
        if (ferror(in)) {
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


/* Figure out what the new state should be by processing the
   diff output. */

inline void
diff_state::next_state()
{
  if (_state == diffstate::DELETE && change_left != 0)
    {
      if (read_line())
        {
          diff_output_corrupt();
        }
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): read %s", linebuf.c_str());
#endif

      if (strcmp(linebuf.c_str(), "---\n") != 0)
        {
          diff_output_corrupt("expected ---");
        }
      lines_left = change_left;
      change_left = 0;
      _state = diffstate::INSERT;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning INSERT [1]\n");
#endif
      return;
    }

  if (_state != diffstate::NOCHANGE)
    {
      if (read_line())
        {
          if (ferror(in))
            {
              diff_output_corrupt();
            }
          _state = diffstate::END;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning END [2]");
#endif
      return;
        }
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state()[3]: read %s", linebuf.c_str());
#endif

      /* Ignore "\ No newline at end of file" if it appears
         at the end of the diff output. */

      if (linebuf[0] == '\\')
        {
          if (!read_line())
            {
              diff_output_corrupt("Expected EOF");
            }
          if (ferror(in))
            {
              diff_output_corrupt();
            }
          _state = diffstate::END;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning END [4]\n");
#endif
          return;
        }

    }

  char *s = NULL;
  int line1, line2, line3, line4;
  char c;

  line1 = (int) strtol(linebuf.c_str(), &s, 10);
  line2 = line1;
  if (*s == ',')
    {
      line2 = (int) strtol(s + 1, &s, 10);
      if (line2 <= line1)
        {
          diff_output_corrupt("left end line");
        }
    }

  c = *s;

  ASSERT(c != '\0');

  if (c == 'a')
    {
      if (line1 >= in_lineno)
        {
          _state = diffstate::NOCHANGE;
          lines_left = line1 - in_lineno + 1;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning NOCHANGE [5]\n");
#endif
          return;
        }
      if (line1 + 1 != in_lineno)
        {
          diff_output_corrupt("left start line [case 1]");
        }
    }
  else
    {
      if (line1 > in_lineno)
        {
          _state = diffstate::NOCHANGE;
          lines_left = line1 - in_lineno;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning END\n");
#endif
          return;
        }
      if (line1 != in_lineno)
        {
          diff_output_corrupt("left start line [case 2]");
        }
    }

  line3 = (int) strtol(s + 1, &s, 10);
  if (c == 'd')
    {
      if (line3 != out_lineno)
        {
          diff_output_corrupt("right start line [case 1]");
        }
    }
  else
    {
      if (line3 != out_lineno + 1)
        {
          diff_output_corrupt("right start line [case 2]");
        }
    }

  line4 = line3;
  if (*s == ',')
    {
      line4 = (int) strtol(s + 1, &s, 10);
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
      _state = diffstate::INSERT;
      lines_left = line4 - line3 + 1;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning INSERT [6]\n");
#endif
      break;

    case 'd':
      _state = diffstate::DELETE;
      lines_left = line2 - line1 + 1;
#ifdef JAY_DEBUG
      fprintf(stderr, "next_state(): returning DELETE [7]\n");
#endif
      break;

    case 'c':
      _state = diffstate::DELETE;
      lines_left = line2 - line1 + 1;
      change_left = line4 - line3 + 1;
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
  if (_state != diffstate::INSERT)
    {
      in_lineno++;
    }

  if (_state != diffstate::END)
    {
      ASSERT(lines_left >= 0);
      if (lines_left == 0)
        {
          if (_state == diffstate::DELETE || _state == diffstate::INSERT)
            {
              fprintf(out, "\001E %d\n", seq);
            }
          next_state();
          if (_state == diffstate::INSERT)
            {
              fprintf(out, "\001I %d\n", seq);
            }
          else if (_state == diffstate::DELETE)
            {
              fprintf(out, "\001D %d\n", seq);
            }
        }
      lines_left--;
    }

  if (_state == diffstate::DELETE)
    {
      if (read_line())
        {
          diff_output_corrupt();
        }
      if (linebuf[0] != '<' || linebuf[1] != ' ')
        {
          diff_output_corrupt("expected <");
        }
    }
  else
    {
      if (_state == diffstate::INSERT)
        {
          if (read_line())
            {
              diff_output_corrupt();
            }
          if (linebuf[0] != '>' || linebuf[1] != ' ')
            {
              diff_output_corrupt("expected >");
            }
        }
      out_lineno++;
    }

  return _state;
}
