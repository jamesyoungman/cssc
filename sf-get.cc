/*
 * sf-get.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of class sccs_file used in getting deltas. 
 * 
 */

#ifdef __GNUC__
#pragma implementation "seqstate.h"
#endif

#include "cssc.h"
#include "sccsfile.h"
#include "pfile.h"
#include "seqstate.h"

#ifndef HAVE_STRSTR
#include "strstr.cc"
#endif

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-get.cc,v 1.6 1997/05/31 10:24:44 james Exp $";
#endif

void
sccs_file::prepare_seqstate(seq_state &state, seq_no seq) {
	while(seq != 0) {
		if (state.is_predecessor(seq)) {
			quit(-1, "%s: Loop in deltas.", (const char *) name);
		}
		state.set_predecessor(seq);

		int len;
		int i;

		struct delta const &delta = delta_table[seq];

		len = delta.included.length();
		for(i = 0; i < len; i++) {
			state.pred_include(delta.included[i]);
		} 		

		len = delta.excluded.length();
		for(i = 0; i < len; i++) {
			state.pred_exclude(delta.excluded[i]);
		} 		

		seq = delta.prev_seq;
	}
}

void
sccs_file::get(mystring gname, class seq_state &state, struct subst_parms &parms,
#ifdef __GNUC__
	       subst_fn_t subst_fn,
#else
	       int (sccs_file::*subst_fn)(const char *,
					  struct subst_parms *) const,
#endif
	       int show_sid, int show_module, int debug) {
	assert(mode != CREATE);
	
	seek_to_body();

	/* The following statement is not correct. */
	/* "@I 1" should start the body of the SCCS file */

	if (read_line() != 'I') {
		corrupt("Expected '@I'");
	}
	check_arg();

	/* The check on the following line is certainly wrong, since
	 * the first body line need not refer to the first delta.  For
	 * example, SunOS 4.1.1's SCCS implementation donesn't always
	 * start with ^AI 1.
	 */
	unsigned short first_delta = strict_atous(linebuf + 3);
	state.start(first_delta, 1); /* 1 means "insert". */

	FILE *out = parms.out;

	while(1) {
		int c = read_line();

		if (c == -1) { 
			/* EOF */
			break;
		}

		if (c == 0) {
			/* A non-control line */

			if (debug) {
				if (state.include_line()) {
					putc('I', f);
				} else {
					putc('D', f);
				}
				putc(' ', f);
			} else if (!state.include_line()) {
				continue;
			}

			parms.out_lineno++;

			if (show_module) {
				mystring module = get_module_name();
				fprintf(out, "%s\t", (const char *) module);
			}

			if (show_sid) {
				delta_table[state.active_seq()].id.print(out);
				putc('\t', out);
			}

			int err;
			if (subst_fn != NULL) {
				err = (this->*subst_fn)(linebuf, &parms);
			} else {
				err = fputs(linebuf, out) == EOF;
				if (!parms.found_id 
				    && check_id_keywords(linebuf)) {
					parms.found_id = 1;
				}
			} 

			if (err || fputc('\n', out) == EOF) {
				quit(errno, "%s: Write error.",
				     (const char *) gname);
			}
			continue;
		}

		/* A control line */

		check_arg();
		seq_no seq = strict_atous(linebuf + 3);

		if (seq < 1 || seq > delta_table.highest_seqno()) {
			corrupt("Invalid serial number");
		}

		const char *msg = NULL;

		switch(c) {
		case 'E':
			msg = state.end(seq);
			break;

		case 'D':
		case 'I':
			msg = state.start(seq, c == 'I');
			break;

		default:
			corrupt("Unexpected control line");
			break;
		}

		if (msg != NULL) {
			corrupt(msg);
		}
	}

	fflush(out);
}

/* Local variables: */
/* mode: c++ */
/* End: */
