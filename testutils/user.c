/*
 * user.c: Part of GNU CSSC.
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
 * Program for getting the user's login name.
 */

#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>
#include <pwd.h>

#ifdef STDC_HEADERS
#include <stdio.h>
#include <string.h>
#endif

const char usage_str[] = "usage: \"user name\" or \"user group\"\n";


int main(int argc, char *argv[])
{
  if (2 == argc)
    {
      if (0 == strcmp(argv[1], "name"))
	{
	  struct passwd *p;
	  const char *pn = "unknown";
	  p = getpwuid(getuid());
	  if (p)
	    pn = p->pw_name;
	  
	  fprintf(stdout, "%s\n", pn);
	  return 0;
	}
      else if (0 == strcmp(argv[1], "group"))
	{
	  fprintf(stdout, "%ld\n", (long)getgid());
	  return 0;
	}
      else
	{
	  fprintf(stderr, "%s", usage_str);
	  return 1;
	}
    }
  else
    {
      fprintf(stderr, "%s", usage_str);
      return 1;
    }
}

