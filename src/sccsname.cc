/*
 * sccsname.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019 Free
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

std::string
base_part(const std::string& name)
{
  std::string dirname, basename;
  split_filename(name, dirname, basename);
  return basename;
}

int
sccs_name::valid_filename(const char *thename)
{
  ASSERT(0 != thename);

  if (thename && thename[0])
    {
      std::string base = base_part(std::string(thename));
      const int valid = (base.at(0) == 's' && base.at(1) == '.');
      return valid;
    }
  else
    {
      // empty filename; not valid.
      return 0;
    }
}

void
sccs_name::create()
{
  // derive all other names from "name",
  // when they're asked for.
  std::string dirname, basename;
  split_filename(sname, dirname, basename);

  name_front = dirname;

  if (basename.length() >= 2)
    {
      const char *s = basename.c_str();

      // name_rear does not contain the "s"
      // of the "s." but does contain the dot itself.
      name_rear = std::string(s+1);

      // The gname does not include the directory part,
      // or the leading "s.".
      gname = std::string(s+2);
    }
  else
    {
      name_rear = basename;
      gname = std::string("");
    }
}

// I don't think we EVER need this function.
// sccs_name::sccs_name(sccs_name const &from) : lock_cnt(from.lock_cnt)
// {
//   sname = from.sfile();
//   create();
// }

sccs_name &
sccs_name::operator =(const std::string& newname)
{
  ASSERT(newname.length() != 0);
  sname = newname;
  create();
  return *this;
}


std::string sccs_name::
sub_file(char insertme) const
{
  char prefix[2];
  prefix[0] = insertme;
  prefix[1] = 0;
  std::string ret = name_front + std::string(prefix) + name_rear;
  return ret;
}

std::string sccs_name::
lfile() const
{
  // Can't use sub_file since, like for the g-file,
  // we need to omit the directory.
  return std::string("l") + name_rear;
}


#ifdef CONFIG_FILE_NAME_GUESSING

void
sccs_name::make_valid()
{
  ASSERT(sname.length() != 0);
  ASSERT(!valid_filename(sname.c_str()));

  std::string dirpart, basepart;
  split_filename(sname, dirpart, basepart);


  // try dir/s.foo
  const std::string s_dot_foo = std::string("s.") + basepart;
  sname = dirpart + s_dot_foo;

  if (!is_readable(sname.c_str()))
    {
      // try dir/SCCS/s.foo
      const std::string tmp = dirpart + std::string("SCCS/") + s_dot_foo;

      if (is_readable(tmp.c_str()))
	sname = tmp;
    }

  create();
}

#endif /* CONFIG_FILE_NAME_GUESSING */

/* Local variables: */
/* mode: c++ */
/* End: */
