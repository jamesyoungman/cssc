/*
 * pf-del.c: Part of GNU CSSC.
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
 * Members of the class sccs_pfile used when removing an edit lock from
 * a p-file.
 *
 */

#include "cssc.h"
#include "pfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: pf-del.cc,v 1.7 1997/11/18 23:22:24 james Exp $";
#endif

/* enum */ sccs_pfile::find_status
sccs_pfile::find_sid(sid id) {

	rewind();

	const char *username = get_user_name();
	sccs_pfile &it = *this;
	int found = -1;

	while(next()) {
		if (strcmp(username, it->user.c_str()) == 0
		    && (id.is_null() || id == it->got || id == it->delta)) {
			if (found != -1) {
				return AMBIGUOUS;
			}
			found = pos;
		}
	}

	if (found == -1) {
		return NOT_FOUND;
	}

	pos = found;

	return FOUND;
}

void
sccs_pfile::update() {
	const char* qname = name.qfile().c_str();

	FILE *pf = fopen(qname, "w");
	if (pf == NULL) {
		quit(errno, "%s: Can't create temporary file.",
		     qname);
	}

        int count = 0;

	rewind();
	while(next()) {
#ifdef __GNUC__
		if (write_edit_lock(pf, edit_locks[pos])) {
			quit(errno, "%s: Write error.", qname);
		}
		    
#else
		if (write_edit_lock(pf, *operator->())) {
			quit(errno, "%s: Write error.", qname);
		}
#endif	       
		count++;
	}

	if (fclose_failed(fclose(pf))) {
		quit(errno, "%s: Write error.", qname);
	}

#ifndef TESTING	

	if (remove(pname.c_str()) != 0) {
		quit(errno, "%s: Can't remove old p-file.",
		     pname.c_str());
	}

	if (count == 0) {
		if (remove(qname) != 0) {
			quit(errno, "%s: Can't remove temporary file.",
			     pname.c_str());
		}
	} else {
		if (rename(qname, pname.c_str()) != 0) {
			quit(errno, "%s: Can't rename new p-file.",
			     qname);
		}
	}
#endif
}

/* Local variables: */
/* mode: c++ */
/* End: */
