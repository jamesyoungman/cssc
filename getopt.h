/*
 * getopt.h: Part of GNU CSSC.
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
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Defines the class getopt.
 *
 * @(#) CSSC getopt.h 1.1 93/11/09 17:17:47
 *
 */

#ifndef CSSC__GETOPT_H__
#define CSSC__GETOPT_H__

#ifdef __GNUC__
#pragma interface
#endif

class getopt
{
public:
  enum
  { 
    END_OF_ARGUMENTS = 0,
    UNRECOGNIZED_OPTION = -1,
    MISSING_ARGUMENT = -2
  };

  int argc;
  char **argv;

  int index;
  char *cindex;
  const char *opts;
  int opterr;
  char *arg;

public:
  getopt(int ac, char **av, const char *s, int err = 1)
    : argc(ac), argv(av), index(1),
      cindex(NULL), opts(s), opterr(err) {}
  int next(void);
  
  int get_index(void) const;
  char *getarg (void) const;
};

#endif /* __GETOPT_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
