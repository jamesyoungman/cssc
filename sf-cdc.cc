/*
 * sf-cdc.c: Part of GNU CSSC.
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
 * Members of the class sccs_file used for change the comments and
 * MRs of a delta. 
 *
 */

#include "cssc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-cdc.cc,v 1.4 1997/07/02 18:04:52 james Exp $";
#endif

/* Adds new MRs and comments to the specified delta. */

void
sccs_file::cdc(sid id, list<mystring> mrs, list<mystring> comments) {
	int i;
	int len;

	struct delta *p = (struct delta *) delta_table.find(id); /* !!! */
	if (p == NULL) {
		quit(-1, "%s: Requested SID doesn't exist.",
		     (const char *) name);
	}
	struct delta &delta = *p;

	list<mystring> not_mrs;
	len = mrs.length();
	for(i = 0; i < len; i++) {
		const char *s = mrs[i];
		if (s[0] == '!') {
			not_mrs.add(s + 1);
		} else {
			delta.mrs.add(s);
		}
	}

	delta.mrs -= not_mrs;

	delta.comments += comments;
}

/* Local variables: */
/* mode: c++ */
/* End: */
