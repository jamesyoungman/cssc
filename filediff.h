/*
 * filediff.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1998,2007 Free Software Foundation, Inc. 
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
 * Functions for diffing two files.
 */

#ifndef CSSC__FILEDIFF_H
#define CSSC__FILEDIFF_H

#include <stdio.h>

#ifdef __GNUC__
#pragma interface
#endif


class FileDiff
{
 public:
  FileDiff(const char *name1, const char *name2);
  ~FileDiff();
  
  FILE * start();
  void finish(FILE * &fp);

 private:
  FILE *fp_;
  mystring name1;
  mystring name2;
};



#endif

/* Local variables: */
/* mode: c++ */
/* End: */
