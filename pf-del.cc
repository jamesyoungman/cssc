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
static const char rcs_id[] = "CSSC $Id: pf-del.cc,v 1.6 1997/11/15 20:07:03 james Exp $";
#endif

/* enum */ sccs_pfile::find_status
sccs_pfile::find_sid(sid id) {

	rewind();

	const char *user = get_user_name();
	sccs_pfile &it = *this;
	int found = -1;

	while(next()) {
		if (strcmp(user, it->user) == 0
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
	mystring qname = name.qfile();

	FILE *pf = fopen(qname, "w");
	if (pf == NULL) {
		quit(errno, "%s: Can't create temporary file.",
		     (const char *) qname);
	}

        int count = 0;

	rewind();
	while(next()) {
#ifdef __GNUC__
		if (write_edit_lock(pf, edit_locks[pos])) {
			quit(errno, "%s: Write error.", (const char *) qname);
		}
		    
#else
		if (write_edit_lock(pf, *operator->())) {
			quit(errno, "%s: Write error.", (const char *) qname);
		}
#endif	       
		count++;
	}

	if (fclose_failed(fclose(pf))) {
		quit(errno, "%s: Write error.", (const char *) qname);
	}

#ifndef TESTING	

	if (remove(pname) != 0) {
		quit(errno, "%s: Can't remove old p-file.",
		     (const char *) pname);
	}

	if (count == 0) {
		if (remove(qname) != 0) {
			quit(errno, "%s: Can't remove temporary file.",
			     (const char *) pname);
		}
	} else {
		if (rename(qname, pname) != 0) {
			quit(errno, "%s: Can't rename new p-file.",
			     (const char *) qname);
		}
	}
#endif
}

/* Local variables: */
/* mode: c++ */
/* End: */
