/*
 * sccsname.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019, 2024
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
 * Members of the class sccs_name.
 *
 */

#include <config.h>
#include <string>
#include "cssc.h"
#include "sccsname.h"
#include "cssc-assert.h"
#include "file.h"

#include <ctype.h>

#ifdef CONFIG_MSDOS_FILES
#error The CONFIG_MSDOS_FILES option is no longer supported.  See sccsname.cc.
// Please feel free to add the support yourself.  Don't forget
// to send the patches to <jay@gnu.org>.
#endif

namespace
{
  cssc::Failure invalid_name(const std::string& reason)
  {
    return cssc::make_failure(cssc::errorcode::NotAnSccsHistoryFileName, reason);
  }
}  // unnamed namespace

std::string
base_part(const std::string& name)
{
  std::string dirname, basename;
  split_filename(name, dirname, basename);
  return basename;
}

cssc::Failure
sccs_name::valid_filename(const char *thename)
{
  ASSERT(nullptr != thename);
  if (!thename[0])
    {
      return invalid_name("SCCS history file names may not be empty");
    }

  const std::string base = base_part(std::string(thename));
  if ((base.size() < 2)
      || (base[0] != 's') || (base[1] != '.'))
    {
      return invalid_name("SCCS history file names must begin with 's.'");
    }
  return cssc::Failure::Ok();
}

void
sccs_name::create()
{
  // derive all other names from "name",
  // when they're asked for.
  std::string dirname, basename;
  split_filename(sname_, dirname, basename);

  name_front_ = dirname;

  if (basename.length() >= 2)
    {
      const char *s = basename.c_str();

      // name_rear does not contain the "s"
      // of the "s." but does contain the dot itself.
      name_rear_ = std::string(s+1);

      // The gname does not include the directory part,
      // or the leading "s.".
      gname_ = std::string(s+2);
    }
  else
    {
      name_rear_ = basename;
      gname_ = std::string("");
    }
}

// I don't think we EVER need this function.
// sccs_name::sccs_name(sccs_name const &from) : lock_cnt(from.lock_cnt)
// {
//   sname = from.sfile();
//   create();
// }

sccs_name::sccs_name()
  : sname_(),
    gname_(),
    name_front_(),
    name_rear_(),
    lock_(),
    lock_cnt_(0)
{
}

sccs_name &
sccs_name::operator =(const std::string& newname)
{
  ASSERT(newname.length() != 0);
  sname_ = newname;
  create();
  return *this;
}


std::string sccs_name::
sub_file(char insertme) const
{
  char prefix[2];
  prefix[0] = insertme;
  prefix[1] = 0;
  std::string ret = name_front_ + std::string(prefix) + name_rear_;
  return ret;
}

std::string sccs_name::
lfile() const
{
  // Can't use sub_file since, like for the g-file,
  // we need to omit the directory.
  return std::string("l") + name_rear_;
}


#ifdef CONFIG_FILE_NAME_GUESSING

void
sccs_name::make_valid()
{
  ASSERT(sname_.length() != 0);
  ASSERT(valid_filename(sname_.c_str()).ok());

  std::string dirpart, basepart;
  split_filename(sname_, dirpart, basepart);


  // try dir/s.foo
  const std::string s_dot_foo = std::string("s.") + basepart;
  sname_ = dirpart + s_dot_foo;

  if (!is_readable(sname_.c_str()))
    {
      // try dir/SCCS/s.foo
      const std::string tmp = dirpart + std::string("SCCS/") + s_dot_foo;

      if (is_readable(tmp.c_str()))
	sname_ = tmp;
    }

  create();
}

#endif /* CONFIG_FILE_NAME_GUESSING */

/* Local variables: */
/* mode: c++ */
/* End: */
