/*
 * filediff.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1998, 2007, 2008, 2009, 2010, 2011, 2014, 2019 Free
 *  Software Foundation, Inc.
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
 *
 */
#include <config.h>
#include <string>
#include "cssc.h"
#include "cssc-assert.h"
#include "filediff.h"
#include "file.h"
#include "privs.h"


FileDiff::FileDiff(const char *n1, const char *n2)
  : fp_(nullptr), name1(n1), name2(n2)
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
    fp_ = nullptr;
    fp = nullptr;
}

FILE*
FileDiff::start()
{
  const std::string space(" ");
  const std::string quote("'");
  std::string cmd(std::string(CONFIG_DIFF_COMMAND) +
               space + quote + name1 + quote +
               space + quote + name2 + quote);

  TempPrivDrop guard();
  fp_ = popen(cmd.c_str(), "r");
  return fp_;
}

/* Local variables: */
/* mode: c++ */
/* End: */
