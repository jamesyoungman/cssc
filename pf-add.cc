/*
 * pf-add.cc: Part of GNU CSSC.
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
 * Members of the class sccs_pfile for adding an edit lock to the file.
 *
 */

#include "cssc.h"
#include "pfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: pf-add.cc,v 1.9 1998/06/14 15:26:54 james Exp $";
#endif

bool
sccs_pfile::add_lock(sid got, sid delta, 
		     sid_list &included, sid_list &excluded) {
	ASSERT(mode == APPEND);

	struct edit_lock new_lock;

	new_lock.got = got;
	new_lock.delta = delta;
	new_lock.user = get_user_name();
	new_lock.date = sccs_date::now();
	new_lock.include = included;
	new_lock.exclude = excluded;
	new_lock.deleted = 0;

	edit_locks.add(new_lock);

	FILE *pf;
	if (edit_locks.length() == 0)
	  {
	    pf = fcreate(pname, CREATE_EXCLUSIVE);
	    if (pf == NULL)
	      {
		perror(pname.c_str());
		return false;
	      }
	  }
	else
	  {
	    pf = fopen(pname.c_str(), "a");
	    if (pf == NULL)
	      {
		perror(pname.c_str());
		return false;
	      }
	  }

	if (write_edit_lock(pf, new_lock) || fclose_failed(fclose(pf)))
	  {
	    perror(pname.c_str());
	    return false;
	  }
	return true;
}

/* Local variables: */
/* mode: c++ */
/* End: */
