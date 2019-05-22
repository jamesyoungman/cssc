/* yammer.c: Part of GNU CSSC.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2007,
 * 2008, 2009, 2010, 2001, 2014, 2019 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



/* This program is not installed as part of CSSC.  It's just used by
 * the test suite.  It takes two arguments, the first being a repeat
 * count, and the second being a string that should be repeated that
 * many times.  For example, "yammer 2 blah" should emit
 *   1 blah
 *   2 blah
 *
 * The program was designed to eliminate the requirement to do
 *   yes blah | nl | head -2
 * because some systems, for example NetBSD/SPARC 1.3.2, lack the "nl"
 * utility.
 *
 * No attempt has been made to make this program use the
 * Autoconf-generated configuration information.  I hope it won't need
 * it.
 *
 * This program was written during October 1998 by Greg A. Woods,
 * VE3TCP, <gwoods@acm.org>, <woods@planix.com>, <woods@wierd.com>.
 */
#include <config.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include "gettext.h"
#include "progname.h"

int
main(int argc, char *argv[])
{
  int i, j;
  const char * prog;

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

  prog = program_name ? program_name : "yammer";
  if (argc != 3)
    {
      fprintf(stderr, "usage: %s count string\n", prog);
      return 2;
    }
  else if ((i = atoi(argv[1])) <= 0)
    {
      fprintf(stderr, "%s: invalid count '%s'\n", prog, argv[1]);
      return 2;
    }
  else
    {
      for (j = 1; i > 0; i--, j++)
        {
          fprintf(stdout, "%d %s\n", j, argv[2]);
        }
      return 0;
    }
}
