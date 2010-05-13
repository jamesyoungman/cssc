/*
 * pf-add.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,2001,2005,2007 Free Software Foundation, Inc. 
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
 * Members of the class sccs_pfile for adding an edit lock to the file.
 *
 */

#include "cssc.h"
#include "pfile.h"
#include "except.h"
#include "file.h"


bool
sccs_pfile::add_lock(sid got, sid delta, 
		     sid_list &included, sid_list &excluded) {
	ASSERT(mode == APPEND);
	struct edit_lock new_lock;
	bool pfile_already_exists;
	

	if (edit_locks.empty())
	  pfile_already_exists = false;
	else
	  pfile_already_exists = true;

	new_lock.got = got;
	new_lock.delta = delta;
	new_lock.user = get_user_name();
	new_lock.date = sccs_date::now();
	new_lock.include = included;
	new_lock.exclude = excluded;

	edit_locks.push_back(new_lock);

	return update( pfile_already_exists );
}

/* Local variables: */
/* mode: c++ */
/* End: */
