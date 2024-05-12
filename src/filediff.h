/*
 * filediff.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1998, 2007, 2008, 2009, 2010, 2011, 2014, 2019, 2024
 *  Free Software Foundation, Inc.
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
 * Functions for diffing two files.
 */

#ifndef CSSC__FILEDIFF_H
#define CSSC__FILEDIFF_H

#include <cstdio>
#include <string>

class FileDiff
{
 public:
  FileDiff(const char *name1, const char *name2);
  ~FileDiff();

  // Prohibit copying to avoid confision when two classes share the
  // same FILE pointer and hence, seek position.
  FileDiff(const FileDiff&) = delete;
  FileDiff& operator=(const FileDiff&) = delete;

  FILE * start();
  void finish(FILE * &fp);

 private:
  FILE *fp_;
  std::string name1_;
  std::string name2_;
};



#endif

/* Local variables: */
/* mode: c++ */
/* End: */
