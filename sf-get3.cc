/*
 * sf-get3.c 
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_file used by get and delta.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-get3.cc,v 1.4 1997/05/31 22:48:43 james Exp $";
#endif

/* Prepare a seqstate for use by marking which sequence numbers are
   to be included and which are to be excluded. */

void
sccs_file::prepare_seqstate(seq_state &state, sid_list include,
			    sid_list exclude, sccs_date cutoff_date) {
	delta_iterator iter(delta_table);

	while(iter.next()) {
		sid const &id = iter->id;

		if (include.member(id)) {
			state.include(iter->seq);
		}
		if (exclude.member(id)) {
			state.exclude(iter->seq);
		}
		if (cutoff_date.valid() && iter->date > cutoff_date) {
			state.exclude(iter->seq);
		}
	}
}

/* Local variables: */
/* mode: c++ */
/* End: */
