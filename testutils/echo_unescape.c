/*
 * echo_unescape.c: Part of GNU CSSC.
 *
 *  Copyright (C) 2026 Free Software Foundation, Inc.
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
#include <config.h>

#include <stdbool.h>		// bool
#include <stdlib.h>		// size_t
/* #include <stdio.h>		// fprintf */

struct unescape_result
{
  size_t output_len;
  bool inhibit_newline;
  char chars_to_emit[2];
};

const struct unescape_result inhibit_newline = { 0, true, {0, 0} };

static int decode_octal_escape(const char **p, int c)
{
  c -= '0';
  if (**p >= '0' && **p <= '7')
    c = c * 8 + (*(*p)++ - '0');
  if (**p >= '0' && **p <= '7')
    c = c * 8 + (*(*p)++ - '0');
  return c;
}


static struct unescape_result handle_backslash(const char **p)
{
  /* fprintf (stderr, "handle_backslash: **p=%c\n", **p); */

  struct unescape_result result = { 0, false, {0, 0} };
  int c = *(*p)++;
  switch (c)
    {
    case 'a':
      c = '\007';
      break;
    case 'b':
      c = '\b';
      break;
    case 'c':
      result.inhibit_newline = true;
      result.output_len = 0;
      return result;
    case 'f':
      c = '\f';
      break;
    case 'n':
      c = '\n';
      break;
    case 'r':
      c = '\r';
      break;
    case 't':
      c = '\t';
      break;
    case 'v':
      c = (int) 0x0B;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      c = decode_octal_escape(p, c);
      break;
    case '\\':
      break;
    default:
      // Unrecogised backslash escape, pass it through.  That means we
      // emit two chars.
      result.chars_to_emit[0] = '\\';
      ++result.output_len;
      break;
    }
  result.chars_to_emit[result.output_len++] = c;
  /*
     fprintf (stderr,
     "handle_backslash: result = { output_len=%zd, inhibit_newline=%s, chars_to_emit={%3d,%3d} }\n",
     result.output_len,
     (result.inhibit_newline ? "true" : "false"),
     result.chars_to_emit[0],
     result.chars_to_emit[1]);
   */
  return result;
}


size_t echo_unescape(const char *input, char *output, bool *inhibit_newline)
{
  size_t output_count = 0;
  for (;;)
    {
      register int c = *input++;
      if (!c)
	{
	  break;
	}
      if (c == '\\' && *input)
	{
	  /* fprintf (stderr, "echo_unescape: c=%c, *s=%c\n", c, *input); */
	  struct unescape_result result = handle_backslash(&input);
	  if (result.inhibit_newline)
	    *inhibit_newline = true;
	  if (result.output_len > 0)
	    output[output_count++] = result.chars_to_emit[0];
	  if (result.output_len > 1)
	    output[output_count++] = result.chars_to_emit[1];
	}
      else
	{
	  output[output_count++] = c;
	}
    }
  return output_count;
}
