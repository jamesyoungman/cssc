/*
 * sccsname.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2001, 2007, 2008, 2009, 2010, 2011, 2014, 2019
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
 * Defines the class sccs_name.
 */

#ifndef CSSC__SCCSNAME_H__
#define CSSC__SCCSNAME_H__

#include <memory>
#include <string>

#include "failure.h"
#include "failure_or.h"
#include "filelock.h"
#include "quit.h"

std::string base_part(const std::string &name);
cssc::FailureOr<std::string> canonify_filename(const char* fname);

class sccs_name
{
  std::string sname;		// name of the s. file.
  std::string gname;

  // We hold separate strings for the part before
  // and the part after the character that changes:
  //  dir/s.foo.c
  //  dir/p.foo.c
  //  dir/z.foo.c
  //  dir/l.foo.c
  // In these cases, name_front is "dir/" and name_rear is ".foo.c".

  std::string name_front, name_rear;

  std::unique_ptr<file_lock> lock_;
  int lock_cnt;

  void create();

  sccs_name &operator =(sccs_name const &);
  sccs_name(sccs_name const &);

public:
  static cssc::Failure valid_filename(const char *name);
  // TODO: probably don't need both valid_filename and valid.
  bool valid() const { return sname.length() > 0; }
  /* The initialisers on the following line have been re-ordered
   * to follow the declaration order.
   */
  sccs_name(): lock_cnt(0)  {}
  sccs_name &operator =(const std::string& n); /* undefined */

  void make_valid();

  const char * c_str() const { return sname.c_str(); }

  std::string sub_file(char insertme) const;
  std::string sfile() const { return sname; }
  std::string gfile() const { return gname; }
  std::string lfile() const;

  std::string pfile() const { return sub_file('p'); }
  std::string qfile() const { return sub_file('q'); }
  std::string xfile() const { return sub_file('x'); }
  std::string zfile() const { return sub_file('z'); }

  cssc::Failure
  lock()
  {
    if (lock_cnt++ == 0)
      {
	std::string zf = zfile();
	lock_ = std::make_unique<file_lock>(zf);
	return lock_->is_locked();
      }
    return cssc::Failure::Ok();
  }

  void
  unlock()
  {
    // TODO: assert that it's locked?
    if (--lock_cnt == 0)
      {
	// Releasing the unique_ptr deletes the lock object which
	// releases the lock.
	lock_.release();
      }
  }

  ~sccs_name()
  {
    if (lock_cnt > 1)
      {
	warning("deleting sccs_name instance whle lock_cnt is %d "
		"(expected <= 1)", lock_cnt);
      }
  }
};

#endif /* __SCCSNAME_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
