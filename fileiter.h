/*
 * fileiter.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999,2007 Free Software Foundation, Inc. 
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
 * Defines the class sccs_file_iter.
 *
 */

#ifndef CSSC__FILEITER_H__
#define CSSC__FILEITER_H__

#include "mylist.h"
#include "sccsname.h"

#ifdef __GNUC__
#pragma interface
#endif

class CSSC_Options;


/* This class is used to iterate over the list of SCCS files as
   specified on the command line. */
class sccs_file_iterator
{
public:
  enum sources { NONE = 0, ARGS, STDIN, DIRECTORY };

private:
  enum sources source;

  char **argv;
  int argc;
  int is_unique;
  
  mylist<mystring> files;
  int pos;
  sccs_name name;

public:
  // sccs_file_iterator(int ac, char **av, int ind = 1);
  sccs_file_iterator(const CSSC_Options&);
  
  int next();

  sccs_name &get_name() { return name; }

  // JAY mod: using is now a keyword; change the function name to 
  // using_source().
  enum sources using_source() { return source; }
  bool using_stdin() { return STDIN == source; }
  
  // unique() returns nonzero if more than exactly one file was
  // specified on the command line; zero if more than one was
  // specified or the names are gotten from a directory or pipe.
  int unique() const;
};

#endif /* __FILEITER_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
