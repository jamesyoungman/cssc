/*
 * sf-write.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_file used adding a new delta to the SCCS
 * file.
 * 
 */

#include "cssc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-add.cc,v 1.3 1997/05/10 14:49:54 james Exp $";
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
	if (fflush(out) == EOF) {
		xfile_error("Write error.");
	}

	rewind(out);

	if (fprintf(out, "\001h-----\n\001s %05u/%05u/%05u",
		    delta.inserted, delta.deleted, delta.unchanged) == EOF) {
		xfile_error("Write error.");
	}

	end_update(out);

	delta_table.prepend(delta);
}

/* Local variables: */
/* mode: c++ */
/* End: */
