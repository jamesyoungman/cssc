/*
 * linebuf.c: Part of GNU CSSC.
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
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Members of the class cssc_linebuf.
 *
 */

#ifdef __GNUC__
//#pragma implementation "linebuf.h"
#endif

#include "cssc.h"
#include "linebuf.h"


// Use a small chunk size for testing...
#define CONFIG_LINEBUF_CHUNK_SIZE (1024)

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: linebuf.cc,v 1.9 1998/02/11 07:49:15 james Exp $";
#endif

cssc_linebuf::cssc_linebuf()
  : buf((char *) xmalloc(CONFIG_LINEBUF_CHUNK_SIZE)),
    buflen(CONFIG_LINEBUF_CHUNK_SIZE)
{
}


int
cssc_linebuf::read_line(FILE *f)
{
  buf[buflen - 2] = '\0';

  char *s = fgets(buf, buflen, f);
  while (s != NULL)
    {
      char c = buf[buflen - 2];
      if (c == '\0' || c == '\n')
	return 0;

      buf = (char *) xrealloc(buf, CONFIG_LINEBUF_CHUNK_SIZE + buflen);
      s = buf + buflen - 1;
      buflen += CONFIG_LINEBUF_CHUNK_SIZE;
      buf[buflen - 2] = '\0';
		
      s = fgets(s, CONFIG_LINEBUF_CHUNK_SIZE + 1, f);
    }
  
  return 1;
}


int cssc_linebuf::write(FILE *f) const
{
  size_t len = strlen(buf);
  return fwrite_failed(fwrite(buf, sizeof(char), len, f), len);
}

int
cssc_linebuf::split(int offset, char **args, int len, char c)
{
  char *start = buf + offset;
  char *end = strchr(start, c);
  int i;

  for (i = 0; i < len; i++)
    {
      args[i] = start;
      if (0 == end)
	{
	  if (start[0] != '\0')
	    i++;
	  return i;		// no more delimiters.
	}
      *end++ = '\0';
      start = end;
      end = strchr(start, c);
    }

  return i;
}

/* Local variables: */
/* mode: c++ */
/* End: */
