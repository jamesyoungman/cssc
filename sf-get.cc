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

#include "mysc.h"
#include "sccsfile.h"
#include "pfile.h"
#include "seqstate.h"

#ifndef HAVE_STRSTR
#include "strstr.cc"
#endif

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC sf-get.c 1.1 93/11/09 17:18:01";
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
	
	const char *req_idkey = flags.id_keywords;
	if (req_idkey != NULL && req_idkey[0] == '\0') {
		req_idkey = NULL;
	}

	seek_to_body();

	/* "@I 1" should start the body of the SCCS file */

	if (read_line() != 'I') {
		corrupt("Expected '@I'");
	}
	check_arg();
	if (strict_atous(linebuf + 3) != 1) {
		corrupt("First delta missing?!?");
	}
		
	state.start(1, 1);

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
				if (req_idkey == NULL && !parms.found_id 
				    && check_id_keywords(linebuf)) {
					parms.found_id = 1;
				}
			} 

			if (!parms.found_id && req_idkey != NULL
			    && strstr(linebuf, req_idkey) != NULL) {
				parms.found_id = 1;
				req_idkey = NULL;
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
