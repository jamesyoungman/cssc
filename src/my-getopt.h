/*
 * my-getopt.h: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,1998,2007 Free Software Foundation, Inc.
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
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Defines the class CSSC_Options.
 *
 */

#ifndef CSSC__MY_GETOPT_H__
#define CSSC__MY_GETOPT_H__

class CSSC_Options
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

private:
  void reorder();		// reorder argv so that options come first.

public:
  CSSC_Options(int ac, char **av, const char *s, int err = 2);
  int next(void);
  int get_index(void) const;
  int get_argc(void) const;
  char **get_argv(void) const;
  char *getarg (void) const;
};

#endif /* CSSC__MY_GETOPT_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
