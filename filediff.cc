/*
 * filediff.cc: Part of GNU CSSC.
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
 *
 */
#include <config.h>

#include "cssc.h"
#include "cssc-assert.h"
#include "filediff.h"
#include "file.h"


FileDiff::FileDiff(const char *n1, const char *n2)
  : fp_(0), name1(n1), name2(n2)
{
}

FileDiff::~FileDiff()
{
  finish(fp_);
}

void
FileDiff::finish(FILE * &fp)
{
    ASSERT(fp == fp_);
    
    if (fp_)
        pclose(fp_);
    fp_ = 0;
    fp = 0;
}

FILE*
FileDiff::start()
{
  const mystring space(" ");
  const mystring quote("'");
  mystring cmd(mystring(CONFIG_DIFF_COMMAND) + 
               space + quote + name1 + quote + 
               space + quote + name2 + quote);

  give_up_privileges();
  fp_ = popen(cmd.c_str(), "r");
  restore_privileges();
  
  return fp_;
}

/* Local variables: */
/* mode: c++ */
/* End: */
