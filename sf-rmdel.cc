/*
 * sf-rmdel.c: Part of GNU CSSC.
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
 * Members of the class sccs_file used for marking a delta in the SCCS
 * files as removed.
 *
 */

#include "cssc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-rmdel.cc,v 1.4 1997/07/02 18:05:27 james Exp $";
#endif

static int
is_seqlist_member(seq_no seq, list<seq_no> const &seq_list) {
	int i;
	int len = seq_list.length();
	for(i = 0; i < len; i++) {
		if (seq == seq_list[i]) {
			return 1;
		}
	}
	return 0;
}

void
sccs_file::rmdel(sid id) {
	struct delta *delta = (struct delta *) delta_table.find(id);
	if (delta == NULL) {
		quit(-1, "%s: Specified SID not found in SCCS file.",
		     (const char *) name);
	}
	seq_no seq = delta->seq;

	delta_iterator iter(delta_table);
	while(iter.next()) {
		if (iter->prev_seq == seq) {
			quit(-1, "%s: Specified SID has a successor.",
			     (const char *) name);
		}
		if (is_seqlist_member(seq, iter->included)
		    || is_seqlist_member(seq, iter->excluded)
		    || is_seqlist_member(seq, iter->ignored)) {
			quit(-1, "%s: Specified SID is used in another delta.",
			     (const char *) name);
		}
	}

	delta->type = 'R';
	
	FILE *out = start_update();
	if (write(out)) {
		xfile_error("Write error.");
	}

	enum { COPY, DELETE, INSERT} state = COPY;
	int c = read_line();
	while(c != -1) {
		if (c != 0) {
			check_arg();
			if (strict_atous(linebuf + 3) == seq) {
				if (state == COPY && c == 'I') {
					state = INSERT;
				} else if (state == COPY && c == 'D') {
					state = DELETE;
				} else if (state != COPY && c == 'E') {
					state = COPY;
				} else {
					corrupt("Unexpected control line");
				}
				c = read_line();
				continue;
			}
			if (state == INSERT) {
				corrupt("Non-terminal delta!?!");
			}
		}
		if (state != INSERT) {
			fputs(linebuf, out);
			putc('\n', out);
		}
		c = read_line();
	}

	if (state != COPY) {
		corrupt("Unexpected EOF");
	}

	end_update(out);
}		      
	
/* Local variables: */
/* mode: c++ */
/* End: */
