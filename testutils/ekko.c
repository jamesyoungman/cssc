/*
 * ekko.c: Part of GNU CSSC.   This file was derived from the src/echo.c
 *         file in GNU sh-utils.  That was derived from part of GNU Bash.
 *         If you actually want an echo(1) lookalike, see GNU sh-utils.
 *         This program has been lobotomised :-)
 *
 * echo.c, derived from code echo.c in Bash.
 *
 *  Copyright (C) 1987, 1989, 1991, 1992, 1993, 1994, 1995, 1996, 1998,
 *  2007, 2019, 2024 Free Software Foundation, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "progname.h"
#include "gettext.h"


/* echo [-neE] [arg ...]
Output the ARGs.  If -n is specified, the trailing newline is
suppressed.  If the -e option is given, interpretation of the
following backslash-escaped characters is turned on:
        \a      alert (bell)
        \b      backspace
        \c      suppress trailing newline
        \f      form feed
        \n      new line
        \r      carriage return
        \t      horizontal tab
        \v      vertical tab
        \\      backslash
        \num    the character whose ASCII code is NUM (octal).

You can explicitly turn off the interpretation of the above characters
on System V systems with the -E option.
*/

/* If defined, interpret backslash escapes if -e is given.  */
#define V9_ECHO

/* If defined, interpret backslash escapes unless -E is given.
   V9_ECHO must also be defined.  */
#define V9_DEFAULT

#if defined (V9_ECHO)
#  if defined (V9_DEFAULT)
#    define VALID_ECHO_OPTIONS "neE"
#  else
#    define VALID_ECHO_OPTIONS "ne"
#  endif /* !V9_DEFAULT */
#else /* !V9_ECHO */
#  define VALID_ECHO_OPTIONS "n"
#endif /* !V9_ECHO */

/* Print the words in LIST to standard output.  If the first word is
   `-n', then don't print a trailing newline.  We also support the
   echo syntax from Version 9 unix systems. */

int
main (int argc, char **argv)
{
  int display_return = 1, do_v9 = 0;

  set_program_name (argv[0]);
  if (NULL == setlocale(LC_ALL, ""))
    {
      /* If we can't set the locale as the user wishes,
       * emit an error message and continue.   The error
       * message will of course be in the "C" locale.
       */
      perror("Error setting locale");
    }
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

/* System V machines already have a /bin/sh with a v9 behaviour.  We
   use the identical behaviour for these machines so that the
   existing system shell scripts won't barf. */
#if defined (V9_ECHO) && defined (V9_DEFAULT)
  do_v9 = 1;
#endif

  --argc;
  ++argv;

  while (argc > 0 && *argv[0] == '-')
    {
      register char *temp;
      register int i;

      /* If it appears that we are handling options, then make sure that
         all of the options specified are actually valid.  Otherwise, the
         string should just be echoed. */
      temp = argv[0] + 1;

      for (i = 0; temp[i]; i++)
        {
          if (strrchr (VALID_ECHO_OPTIONS, temp[i]) == 0)
            goto just_echo;
        }

      if (!*temp)
        goto just_echo;

      /* All of the options in TEMP are valid options to ECHO.
         Handle them. */
      while (*temp)
        {
          if (*temp == 'n')
            display_return = 0;
#if defined (V9_ECHO)
          else if (*temp == 'e')
            do_v9 = 1;
#if defined (V9_DEFAULT)
          else if (*temp == 'E')
            do_v9 = 0;
#endif /* V9_DEFAULT */
#endif /* V9_ECHO */
          else
            goto just_echo;

          temp++;
        }
      argc--;
      argv++;
    }

just_echo:

  if (argc > 0)
    {
#if defined (V9_ECHO)
      if (do_v9)
        {
          while (argc > 0)
            {
              register char *s = argv[0];
              register int c;

              while ((c = *s++))
                {
                  if (c == '\\' && *s)
                    {
                      switch (c = *s++)
                        {
                        case 'a': c = '\007'; break;
                        case 'b': c = '\b'; break;
                        case 'c': display_return = 0; continue;
                        case 'f': c = '\f'; break;
                        case 'n': c = '\n'; break;
                        case 'r': c = '\r'; break;
                        case 't': c = '\t'; break;
                        case 'v': c = (int) 0x0B; break;
                        case '0': case '1': case '2': case '3':
                        case '4': case '5': case '6': case '7':
                          c -= '0';
                          if (*s >= '0' && *s <= '7')
                            c = c * 8 + (*s++ - '0');
                          if (*s >= '0' && *s <= '7')
                            c = c * 8 + (*s++ - '0');
                          break;
                        case '\\': break;
                        default:  putchar ('\\'); break;
                        }
                    }
                  putchar(c);
                }
              argc--;
              argv++;
              if (argc > 0)
                putchar(' ');
            }
        }
      else
#endif /* V9_ECHO */
        {
          while (argc > 0)
            {
              fputs (argv[0], stdout);
              argc--;
              argv++;
              if (argc > 0)
                putchar (' ');
            }
        }
    }
  if (display_return)
    putchar ('\n');
  exit (0);
}
