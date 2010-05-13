/*
 * sccsname.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,2001,2007,2008 Free Software Foundation, Inc. 
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
 * Defines the class sccs_name.
 *
 * @(#) CSSC sccsname.h 1.1 93/11/09 17:17:50
 *
 */

#ifndef CSSC__SCCSNAME_H__
#define CSSC__SCCSNAME_H__

#include "mystring.h"
#include "filelock.h"

#ifdef __GNUC__
#pragma interface
#endif

mystring base_part(const mystring &name);
mystring canonify_filename(const char* fname);

class sccs_name
{
  mystring sname;		// name of the s. file.
  mystring gname;
  
  // We hold separate strings for the part before 
  // and the part after the character that changes:
  //  dir/s.foo.c
  //  dir/p.foo.c
  //  dir/z.foo.c
  //  dir/l.foo.c
  // In these cases, name_front is "dir/" and name_rear is ".foo.c".
  
  mystring name_front, name_rear;
  
  file_lock *lock_ptr;
  int lock_cnt;
  
  void create();

  void
  destroy()
  {
    if (lock_cnt > 0)
      delete lock_ptr;
  }

  sccs_name &operator =(sccs_name const &);
  sccs_name(sccs_name const &);
  
public:
  static int valid_filename(const char *name);
  /* The initialisers on the following line have been re-ordered
   * to follow the declaration order. 
   */
  sccs_name(): lock_ptr(0), lock_cnt(0)  {}
  sccs_name &operator =(const mystring& n); /* undefined */

  bool valid() const { return sname.length() > 0; }
  void make_valid();

  const char * c_str() const { return sname.c_str(); }

  mystring sub_file(char insertme) const;
  mystring sfile() const { return sname; }
  mystring gfile() const { return gname; }
  mystring lfile() const;
  
  mystring pfile() const { return sub_file('p'); }
  mystring qfile() const { return sub_file('q'); }
  mystring xfile() const { return sub_file('x'); }
  mystring zfile() const { return sub_file('z'); }
  
  int
  lock()
  { 
    if (lock_cnt++ == 0)
      {
	mystring zf = zfile();
	lock_ptr = new file_lock(zf);
	return lock_ptr->failed();
      }
    return 0;
  }

  void
  unlock()
  {
    if (--lock_cnt == 0)
      {
	delete lock_ptr;
      }
  }
  
  ~sccs_name()
  {
    destroy();
  }
};

#endif /* __SCCSNAME_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
