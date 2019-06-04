/*
 * fileiter.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2007, 2008, 2009, 2010, 2011, 2014,
 *  2019 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#include <string>
#include <vector>
#include "sccsname.h"

class CSSC_Options;


/* This class is used to iterate over the list of SCCS files as
   specified on the command line. */
class sccs_file_iterator
{
public:
  enum class source { NONE = 0, ARGS, STDIN, DIRECTORY };
  sccs_file_iterator(const CSSC_Options&);

  bool next();

  sccs_name &get_name() { return name; }
  source using_source() { return source_; }
  bool empty() const { return source_ == source::NONE; }
  bool using_stdin() { return source::STDIN == source_; }

  // unique() returns nonzero if exactly one file was specified on the
  // command line; zero if more than one was specified or the names
  // are taken from a directory or pipe.
  bool unique() const;

private:
  source source_;
  bool is_unique;
  std::vector<std::string> files;
  std::vector<std::string>::size_type pos; // current iteration position
  sccs_name name;
};

#endif /* __FILEITER_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
