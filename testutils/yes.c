/*
 * yes.c: Part of GNU CSSC.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2007,
 * 2008, 2009, 2010, 2001, 2014, 2019, 2024 Free Software Foundation,
 * Inc.
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
 *
 * clone of yes(1) -- some systems (e.g. Solaris) lack yes(1).
 */
#include <config.h>

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "gettext.h"
#include "progname.h"

unsigned long
total_len(int argc, char * argv[])
{
  unsigned long l = 0u;
  int i;
  for (i=0; i<argc; ++i)
    l += strlen(argv[i]);
  return l;
}

void
concat(char *buf, int argc, char * argv[])
{
  int i;
  buf[0] = '\0';
  for (i=0; i<argc; ++i)
    strcat(buf, argv[i]);
}



int
main(int argc, char *argv[])
{
  char *msg;
  unsigned long l;

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

  /* make sure SIGPIPE is not ignored */
  signal(SIGPIPE, SIG_DFL);
  argv++;
  argc--;

  if (argc > 0)
    {
      l = total_len(argc, argv);
      msg = malloc(l + 1u);
      if (NULL == msg)
        {
          fprintf(stderr, "Ran out of memory.\n");
          return 1;
        }
      concat(msg, argc, argv);

      for (;;)
        puts(msg);              /* this adds a trailing newline */
    }
  else
    {
      for (;;)
        puts("yes");            /* this adds a trailing newline */
    }

  /*NOTREACHED*/
  return 0;
}
