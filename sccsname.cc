/* 
 * sccsname.cc: Part of GNU CSSC.
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
 * Members of the class sccs_name.
 *
 */

#ifdef __GNUC__
#pragma implementation "sccsname.h"
#endif

#include "cssc.h"
#include "sccsname.h"

#include <ctype.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sccsname.cc,v 1.8 1998/05/09 16:10:58 james Exp $";
#endif

#ifdef CONFIG_MSDOS_FILES 
#error The CONFIG_MSDOS_FILES option is no longer supported.  See sccsname.cc.
// Please feel free to add the support yourself.  Don't forget
// to send the patches to <jay@gnu.org>.
#endif

mystring
base_part(const mystring &name)
{
  mystring dirname, basename;
  split_filename(name, dirname, basename);
  return basename;
}

int
sccs_name::valid_filename(const char *thename)
{
  ASSERT(0 != thename);
  
  if (thename && thename[0])
    {
      mystring base = base_part(mystring(thename));
      const int valid = (base.at(0) == 's' && base.at(1) == '.');
#if 0
      fprintf(stderr, "valid_filename returning %d\n", valid);
#endif
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
#if 0
  fprintf(stderr, "sccs_name::create(): sname='%s'\n", sname.c_str());
#endif  

  // derive all other names from "name",
  // when they're asked for.
  mystring dirname, basename;
  split_filename(sname, dirname, basename);

#if 0  
  fprintf(stderr, "sccs_name::create(): dirname='%s', basename='%s'\n",
	  dirname.c_str(), basename.c_str());
#endif
  
  name_front = dirname;

  if (basename.length() >= 2)
    {
      // name_rear does not contain the "s" 
      // of the "s." but does contain the dot itself.
      name_rear.assign(basename, 1);

      // The gname does not include the directory part,
      // or the leading "s.".
      gname.assign(basename, 2);
    }
  else
    {
      name_rear = basename;
      gname = mystring("");
    }
}

// I don't think we EVER need this function.
// sccs_name::sccs_name(sccs_name const &from) : lock_cnt(from.lock_cnt)
// {
//   sname = from.sfile();
//   create();
// }

sccs_name &
sccs_name::operator =(const mystring &newname)
{
  ASSERT(newname.length() != 0);

#if 0
  fprintf(stderr, "sccs_name::operator=(const mystring&): newname='%s'\n",
		newname.c_str());
#endif
  
  sname = newname;
  create();
  return *this;
}


mystring sccs_name::
sub_file(char insertme) const
{
  char prefix[2];
  prefix[0] = insertme;
  prefix[1] = 0;
  mystring ret = name_front + mystring(prefix) + name_rear;
#if 0
  fprintf(stderr,
	  "sccs_name::sub_file('%c') returning \"%s\".\n",
	  insertme, ret.c_str());
#endif
  return ret;
}


#ifdef CONFIG_FILE_NAME_GUESSING

void
sccs_name::make_valid()
{
  ASSERT(sname.length() != 0);
  ASSERT(!valid_filename(sname.c_str()));

#if 0
  fprintf(stderr, "Making filename '%s' valid...\n",
	  sname.c_str());
#endif
  
  mystring dirpart, basepart;
  split_filename(sname, dirpart, basepart);
  
  
  // try dir/s.foo
  const mystring s_dot_foo = mystring("s.") + basepart;
  sname = dirpart + s_dot_foo;
  
  if (!is_readable(sname.c_str()))
    {
      // try dir/SCCS/s.foo
      const mystring tmp = dirpart + mystring("SCCS/") + s_dot_foo;
      
      if (is_readable(tmp.c_str()))
	sname = tmp;
    }
  
  create();
}
	
#endif /* CONFIG_FILE_NAME_GUESSING */

/* Local variables: */
/* mode: c++ */
/* End: */
