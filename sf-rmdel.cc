/*
 * sf-rmdel.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_file used for marking a delta in the SCCS
 * files as removed.
 *
 */

#include "mysc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static char const sccs_id[] = "@(#) MySC sf-rmdel.c 1.1 93/11/09 17:18:05";
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
		     (char const *) name);
	}
	seq_no seq = delta->seq;

	delta_iterator iter(delta_table);
	while(iter.next()) {
		if (iter->prev_seq == seq) {
			quit(-1, "%s: Specified SID has a successor.",
			     (char const *) name);
		}
		if (is_seqlist_member(seq, iter->included)
		    || is_seqlist_member(seq, iter->excluded)
		    || is_seqlist_member(seq, iter->ignored)) {
			quit(-1, "%s: Specified SID is used in another delta.",
			     (char const *) name);
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
