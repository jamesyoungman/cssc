/*
 * fileiter.h: Part of GNU CSSC.
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
 * Defines the class sccs_file_iter.
 *
 * @(#) CSSC fileiter.h 1.1 93/11/09 17:17:46
 *
 */

#ifndef CSSC__FILEITER_H__
#define CSSC__FILEITER_H__

#include "mylist.h"
#include "sccsname.h"

#define CONFIG_NO_DIRECTORY

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
# undef CONFIG_NO_DIRECTORY
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# undef CONFIG_NO_DIRECTORY
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# undef CONFIG_NO_DIRECTORY
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# undef CONFIG_NO_DIRECTORY
# endif
#endif


#ifdef __GNUC__
#pragma interface
#endif

/* This class is used to iterate over the list of SCCS files as
   specified on the command line. */

class sccs_file_iterator
{
public:
  enum sources { ARGS, STDIN, DIRECTORY };

private:
  enum sources source;

  char **argv;
  int argc;
  int is_unique;
  
#ifndef CONFIG_NO_DIRECTORY
  list<mystring> files;
  int pos;
#endif
  sccs_name name;

public:
  sccs_file_iterator(int ac, char **av, int ind = 1);
  
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
