/*
 * filediff.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1998, Free Software Foundation, Inc. 
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Functions for diffing two files.
 *
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cssc.h"
#include "filediff.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: filediff.cc,v 1.6 2001/12/01 21:57:58 james_youngman Exp $";
#endif


FileDiff::FileDiff(const char *n1, const char *n2)
  : fp(0), name1(n1), name2(n2)
{
}

FileDiff::~FileDiff()
{
  finish();
}

void
FileDiff::finish()
{
  if (fp)
    pclose(fp);
  fp = 0;
}

FILE*
FileDiff::start()
{
  FILE *fp;
  
  const mystring space(" ");
  const mystring quote("'");
  mystring cmd(mystring(CONFIG_DIFF_COMMAND) + 
	       space + quote + name1 + quote + 
	       space + quote + name2 + quote);

  give_up_privileges();
  fp = popen(cmd.c_str(), "r");
  restore_privileges();
  
  return fp;
}



/* Local variables: */
/* mode: c++ */
/* End: */
