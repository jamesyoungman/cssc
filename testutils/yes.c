/*
 * yes.c: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * clone of yes(1) -- some systems (e.g. Solaris) lack yes(1).
 */

#ifdef STDC_HEADERS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif


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



#ifndef EXIT_FAILURE
#define EXIT_FAILURE (1)
#endif

int
main(int argc, char *argv[])
{
  char *msg;
  unsigned long l;

  argv++;
  argc--;
  
  if (argc > 0)
    {
      l = total_len(argc, argv);
      msg = malloc(l + 1u);
      if (NULL == msg)
        {
          fprintf(stderr, "Ran out of memory.\n");
          return EXIT_FAILURE;
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

