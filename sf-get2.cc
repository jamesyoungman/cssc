/*
 * sf-get2.c 
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_file only used by get.
 *
 */

#include "mysc.h"
#include "sccsfile.h"
#include "pfile.h"
#include "seqstate.h"

#include <ctype.h>

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC sf-get2.c 1.3 93/12/31 15:16:23";
#endif

/* Returns the SID of the delta to retrieve that best matches the
   requested SID. */

bool
sccs_file::find_requested_sid(sid requested, sid &found) const {
	if (requested.is_null()) {
		requested = flags.default_sid;
	}

	sid best;
	bool got_best = false;
	
	/* Find the delta with the highest SID that matches the
	   requested SID */

	delta_iterator iter(delta_table);

	while(iter.next()) {
		sid const &id = iter->id;
		if (id == requested) {
			/* Found an exact match. */
			found = requested;
			return true;
		}

		if ( !got_best && requested.is_null() )
		  {
		    best = id;
		    got_best = true;
		  }		
		else if (requested.partial_match(id) && id > best)
		  {
		    best = id;
		    got_best = true;
		  }
	}
	if (got_best)
	  {
	    found = best;
	    return true;
	  }
	else if (!requested.release_only())
	  {
	    // JAY TODO: if !requested.release_only(),
	    // should we return best at all?
	    return best;
	  }

	/* If a match wasn't found above and the requested SID
	   only mentions a release then look for the delta with 
           the highest SID that's lower than the requested SID. */

	best = sid();
	got_best = false;
	
	iter.rewind();
	while(iter.next()) {
		sid const &id = iter->id;
		if (!got_best) {
		  best = id;
		} else if (id > best && id < requested) {
		  best = id;
		}
	}
	if (got_best)
	  {
	    found = best;
	    return true;
	  }
	else
	  {
	    return false;		// TODO: what???
	  }
}


/* Returns the SID of the new delta to be created. */

sid
sccs_file::find_next_sid(sid requested, sid got, int branch,
			 sccs_pfile &pfile) const {
	branch = branch && flags.branch;

	if (!branch
	    && !got.is_trunk_successor(delta_table.highest_release()))  {
		if (requested > delta_table.highest_release()) {
			assert(requested.release_only());
			return requested.successor();
		}

		sid next = got.successor();

		if (!pfile.is_to_be_created(next)
		    && delta_table.find(next) == NULL) {
			return next;
		}
	}

	sid highest = got;

	delta_iterator iter(delta_table);
	while(iter.next()) {
		sid const &id = iter->id;

		if (id.branch_greater_than(highest)) {
			highest = id;
		}
	}

	pfile.rewind();
	while(pfile.next()) {
		sid const &id = pfile->delta;
		
		if (id.branch_greater_than(highest)) {
			highest = id;
		}
	}

	return highest.next_branch();
}


/* Quits if the user isn't authorized to make deltas, if the release
   requested is locked or if the requested SID has an edit lock and
   the joint edit flag isn't set. */

void
sccs_file::test_locks(sid got, sccs_pfile &pfile) const {
	int i;
	int len;

	const char *user = get_user_name();

	len = users.length();
	if (len != 0) {
		int found = 0;

		for(i = 0; i < len; i++) {
			const char *s = users[i];
			char c = s[0];

			if (c == '!') {
				s++;
				if (isdigit(c)) {
					if (user_is_group_member(atoi(s))) {
						found = 0;
						break;
					}
				} else if (strcmp(s, user) == 0) {
					found = 0;
					break;
				}
			} else {
				if (isdigit(c)) {
					if (user_is_group_member(atoi(s))) {
						found = 1;
					}
				} else if (strcmp(s, user) == 0) {
					found = 1;
					break;
				}
			}
		}
		
		if (!found) {
			quit(-1, "%s: You are not authorized to make deltas.",
			     (const char *) name);
		}
	}

	if (flags.all_locked 
	    || (flags.floor.valid() && flags.floor > got)
	    || (flags.ceiling.valid() && flags.ceiling < got)
	    || flags.locked.member(got)) {
		quit(-1, "%s: Requested release is locked.",
		     (const char *) name);
	}
	
	if (pfile.is_locked(got) && !flags.joint_edit) {
		quit(-1, "%s: Requested SID locked by '%s'.\n",
		     (const char *) name,
		     (const char *) pfile->user);
	}
}


/* Write a line of a file after substituting any id keywords in it.
   Returns true if an error occurs. */

int
sccs_file::write_subst(const char *start, struct subst_parms *parms) const {
	struct delta const &delta = parms->delta;
	FILE *out = parms->out;

	const char *percent = strchr(start, '%');
	while(percent != NULL) {
		char c = percent[1];
		if (c != '\0' && percent[2] == '%') {
			if (start != percent
			    && fwrite(start, percent - start, 1, out) != 1) {
				return 1;
			}

			percent += 3;

			int err = 0;

			switch(c) {
				const char *s;

			case 'M':
			{
				mystring module = get_module_name();
				err = (fputs(module, out) == EOF);
			}
				break;
			
			case 'I':
				err = delta.id.print(out);
				break;

			case 'R':
				err = delta.id.printf(out, 'R');
				break;

			case 'L':
				err = delta.id.printf(out, 'L');
				break;

			case 'B':
				err = delta.id.printf(out, 'B');
				break;

			case 'S':
				err = delta.id.printf(out, 'S');
				break;

			case 'D':
				err = parms->now.printf(out, 'D');
				break;
				
			case 'H':
				err = parms->now.printf(out, 'H');
				break;

			case 'T':
				err = parms->now.printf(out, 'T');
				break;

			case 'E':
				err = delta.date.printf(out, 'D');
				break;

			case 'G':
				err = delta.date.printf(out, 'H');
				break;

			case 'U':
				err = delta.date.printf(out, 'T');
				break;

			case 'Y':
				s = flags.type;
				if (s != NULL) {
					err = fputs(s, out) == EOF;
				}
				break;

			case 'F':
				err = fputs(base_part(name), out)
				      == EOF;
				break;

			case 'P':
				err = fputs(name, out) == EOF;
				break;

			case 'Q':
				s = flags.user_def;
				if (s != NULL) {
					err = fputs(s, out) == EOF;
				}
				break;

			case 'C':
				err = fprintf(out, "%d",
					      parms->out_lineno) == EOF;
				break;

			case 'Z':
				if (fputc('@', out) == EOF
				    || fputs("(#)", out) == EOF) {
					err = 1;
				} else {
					err = 0;
				}
				break;

			case 'W':
				s = parms->wstring;
				if (s == NULL) {
					s = "%Z""%%M""%\t%""I%";
				} else {
					/* protect against recursion */
					parms->wstring = NULL; 
				}
				err = write_subst(s, parms);
				if (parms->wstring == NULL) {
					parms->wstring = s;
				}
				break;

			case 'A':
				err = write_subst("%Z""%%Y""% %M""% %I"
						  "%%Z""%", parms);
				break;

			default:
				start = percent - 3;
				percent = percent - 1;
				continue;
			}

			parms->found_id = 1;

			if (err) {
				return 1;
			}
			start = percent;
		} else {
			percent++;
		}
		percent = strchr(percent, '%');
	}

	return fputs(start, out) == EOF;
}

/* Output the specified version to a file with possible modifications.
   Most of the actual work is done with a seqstate object that 
   figures out whether or not given line of the SCCS file body
   should be included in the output file. */
	
/* struct */ sccs_file::get_status
sccs_file::get(FILE *out, mystring gname, sid id, sccs_date cutoff,
	       sid_list include, sid_list exclude,
	       int keywords, const char *wstring,
	       int show_sid, int show_module, int debug) {

	seq_state state(delta_table.highest_seqno());
	struct delta const *delta = delta_table.find(id);
	assert(delta != NULL);
	prepare_seqstate(state, delta->seq);
	prepare_seqstate(state, include, exclude, cutoff);

	struct subst_parms parms(out, wstring, *delta, 0, sccs_date::now());

#ifdef __GNUC__
    if (keywords)
        {
        get(gname, state, parms, &sccs_file::write_subst,
            show_sid, show_module, debug);
        }
    else
        {
        get(gname, state, parms, (subst_fn_t) 0,
            show_sid, show_module, debug);
        }
#else
    if (keywords)
        {
        get(gname, state, parms,
            (int (sccs_file::*)(const char *, struct subst_parms *) const) 0,
            show_sid, show_module, debug);
        }
    else
        {
        get(gname, state, parms, &sccs_file::write_subst,
	    show_sid, show_module, debug);
        }
#endif

	if (!parms.found_id) {
		if (flags.id_keywords != NULL
		    && *(const char *)flags.id_keywords != '\0') {
			fprintf(stderr, "%s: Warning: Required keywords \"%s\""
				        " missing.",
				(const char *) name,
				(const char *) flags.id_keywords);
		}
		fprintf(stderr, "%s: Warning: No id keywords.\n",
			(const char *) name);
	}
				     
	/* Set the return status. */
	struct get_status status;

	status.lines = parms.out_lineno;
	
	seq_no seq;	
	for(seq = 1; seq <= delta_table.highest_seqno(); seq++) {
		if (state.is_explicit(seq)) {
			if (state.is_included(seq)) {
				status.included.add(delta_table[seq].id);
			} else if (state.is_excluded(seq)) {
				status.excluded.add(delta_table[seq].id);
			}
		}
	}
			
	return status;
}

/* Local variables: */
/* mode: c++ */
/* End: */
