/*
 * sf-write.c: Part of GNU CSSC.
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
 * Members of the class sccs_file used adding a new delta to the SCCS
 * file.
 * 
 */

#include "cssc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-add.cc,v 1.5 1997/11/15 20:05:53 james Exp $";
#endif

/* Insert a delta at the start of the delta table. */

void
sccs_file::_delta_table::prepend(struct delta const &it) {
	int i;
	if (left == 0) {
		struct delta *new_array
			= new struct delta[len + CONFIG_LIST_CHUNK_SIZE];
		if (len != 0) {
			for(i = 0; i < len; i++) {
				new_array[i + 1] = array[i];
			}
			delete[] array;
		}
		array = new_array;
		left = CONFIG_LIST_CHUNK_SIZE;
	} else {
		for(i = len; i > 0; i--) {
			array[i] = array[i - 1];
		}
	}
	array[0] = it;
	len++;
	left--;

	update_highest(it);
}


/* Starts an update of an SCCS file that includes a new entry to be
   prepended to delta table. */

FILE *
sccs_file::start_update(struct delta const &new_delta) {
	FILE *out = start_update();

	if (write_delta(out, new_delta) || write(out)) {
		xfile_error("Write error.");
	}
	return out;
}


/* Set the line counts in the prepended delta and then end the update. */

void
sccs_file::end_update(FILE *out, struct delta const &delta) {
	if (fflush_failed(fflush(out))) {
		xfile_error("Write error.");
	}

	rewind(out);

	if (printf_failed(fprintf(out, "\001h-----\n\001s %05u/%05u/%05u",
				  delta.inserted, delta.deleted,
				  delta.unchanged))) {
		xfile_error("Write error.");
	}

	end_update(out);

	delta_table.prepend(delta);
}

/* Local variables: */
/* mode: c++ */
/* End: */
