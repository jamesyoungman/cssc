/*
 * sf-prs.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_file for printing selected parts of an
 * SCCS file.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-prs.cc,v 1.5 1997/05/10 14:49:57 james Exp $";
#endif

inline void
sccs_file::get(FILE *out, mystring name, seq_no seq) {
	struct subst_parms parms(out, NULL, delta(), 0, sccs_date(NULL));
	class seq_state state(delta_table.highest_seqno());
	prepare_seqstate(state, seq);
	get(name, state, parms);
}

/* Prints a list of sequence numbers on the same line. */

static void
print_seq_list(FILE *out, list<seq_no> const &list) {
	int i;
	int len = list.length();

	if (len > 0) {
		fprintf(out, "%u", list[0]);
		for(i = 1; i < len; i++) {
			fprintf(out, " %u", list[i]);
		}
	}
}


/* Prints a list of strings, one per line. */

static void
print_string_list(FILE *out, list<mystring> const &list) {
	int i;
	int len = list.length();

	for(i = 0; i < len; i++) {
		fprintf(out, "%s\n", (const char *) list[i]);
	}
}


/* Prints a string flag with it's name. */

static void
print_flag2(FILE *out, const char *s, mystring it) {
	if (it != NULL) {
		fprintf(out, "%s: %s\n", s, (const char *) it);
	}
}


/* Prints a boolean flag with it's name. */

static void
print_flag2(FILE *out, const char *s, int it) {
	if (it) {
		fprintf(out, "%s: yes\n", s);
	}
}


/* Prints a flag whose type has a print(FILE *) member with it's name. */

template <class TYPE>
void
print_flag2(FILE *out, const char *s, TYPE it) {
	if (it.valid()) {
		fprintf(out, "%s: ", s);
		it.print(out);
		putc('\n', out);
	}
}


/* Prints all the flags of an SCCS file. */

void
sccs_file::print_flags(FILE *out) const {
	print_flag2(out, (const char *) "Module Type", flags.type);
	print_flag2(out, (const char *) "MR Validation", flags.mr_checker);
	print_flag2(out, (const char *) "Keyword Validation",
		    flags.id_keywords);
	print_flag2(out, (const char *) "Branch", flags.branch);
	print_flag2(out, (const char *) "Module Name", flags.module);
	print_flag2(out, (const char *) "Floor", flags.floor);
	print_flag2(out, (const char *) "Ceiling", flags.ceiling);
	print_flag2(out, (const char *) "Default SID", flags.default_sid);
	print_flag2(out, (const char *) "Null Deltas", flags.null_deltas);
	print_flag2(out, (const char *) "Joint Editing", flags.joint_edit);
	if (flags.all_locked) {
		fputs("Locked Releases: a", out);
	} else {
		print_flag2(out, (const char *) "Locked Releases",
			    flags.locked);
	}
	print_flag2(out, (const char *) "User Keyword", flags.user_def);
	if (flags.encoded)
	  fputs("encoded\n", out);
}


/* Prints "yes" or "no" according to the value of a boolean flag. */

static void
print_yesno(FILE *out, int flag) {
	if (flag) {
		fputs("yes", out);
	} else {
		fputs("no", out);
	}
}


/* Prints the the value of string flag. */
template <class TYPE>
void
print_flag(FILE *out, TYPE it) {
	if (it.valid()) {
		it.print(out);
	} else {
		fputs("none", out);
	}
}

/* Prints the value of flag whose type has a print(FILE *) member. */

static void
print_flag(FILE *out, mystring s) {
	if (s == NULL) {
		fputs("none", out);
	} else {
		fputs(s, out);
	}
}


/* These macros are used to convert the one or two characters a prs
   data keyword in an unsigned value used in the switch statement 
   in the function below. */

#define KEY1(c)		((unsigned char)(c))
#define KEY2(c1, c2)	(((unsigned char)(c1)) * 256 + (unsigned char)(c2))

/* Prints selected parts of an SCCS file and the specified entry in the
   delta table. */

void
sccs_file::print_delta(FILE *out, const char *format,
		       struct delta const &delta) {
	const char *s = format;

	while(1) {
		char c = *s++;

		if (c == '\0') {
			break;
		}

		if (c != ':' || s[0] == '\0') {
			putc(c, out);
			continue;
		}

		const char *back_to = s;
		unsigned key = 0;

		if (s[1] == ':') {
			key = KEY1(s[0]);
			s += 2;
		} else if (s[2] == ':') {
			key = KEY2(s[0], s[1]);
			s += 3;
		} else {
			putc(':', out);
			continue;
		}

		switch(key) {
		default:
			s = back_to;
			putc(':', out);
			continue;
			
		case KEY2('D','t'):
			print_delta(out, ":DT: :I: :D: :T: :P: :DS: :DP:",
				    delta);
			break;

		case KEY2('D','L'):
			print_delta(out, ":Li:/:Ld:/:Lu:", delta);
			break;

		case KEY2('L','i'):
			fprintf(out, "%05u", delta.inserted);
			break;

		case KEY2('L','d'):
			fprintf(out, "%05u", delta.deleted);
			break;

		case KEY2('L','u'):
			fprintf(out, "%05u", delta.unchanged);
			break;

		case KEY2('D','T'):
			putc(delta.type, out);
			break;

		case KEY1('I'):
			delta.id.print(out);
			break;

		case KEY1('R'):
			delta.id.printf(out, 'R');
			break;

		case KEY1('L'):
			delta.id.printf(out, 'L');
			break;

		case KEY1('B'):
			delta.id.printf(out, 'B');
			break;

		case KEY1('S'):
			delta.id.printf(out, 'S');
			break;

		case KEY1('D'):
			delta.date.printf(out, 'D');
			break;

		case KEY2('D','y'):
			delta.date.printf(out, 'y');
			break;

		case KEY2('D','m'):
			delta.date.printf(out, 'o');
			break;

		case KEY2('D','d'):
			delta.date.printf(out, 'd');
			break;

		case KEY1('T'):
			delta.date.printf(out, 'T');
			break;

		case KEY2('T','h'):
			delta.date.printf(out, 'h');
			break;

		case KEY2('T','m'):
			delta.date.printf(out, 'm');
			break;

		case KEY2('T','s'):
			delta.date.printf(out, 's');
			break;

		case KEY1('P'):
			fputs(delta.user, out);
			break;

		case KEY2('D','S'):
			fprintf(out, "%u", delta.seq);
			break;

		case KEY2('D','P'):
			fprintf(out, "%u", delta.prev_seq);
			break;

		case KEY2('D', 'I'):
			print_delta(out, ":Dn:/:Dx:/:Dg:", delta);
			break;

		case KEY2('D','n'):
			print_seq_list(out, delta.included);
			break;

		case KEY2('D','x'):
			print_seq_list(out, delta.excluded);
			break;

		case KEY2('D','g'):
			print_seq_list(out, delta.ignored);
			break;

		case KEY2('M','R'):
			print_string_list(out, delta.mrs);
			break;

		case KEY1('C'):
			print_string_list(out, delta.comments);
			break;

		case KEY2('U','N'):
			print_string_list(out, users);
			break;

		case KEY2('F', 'L'):
			print_flags(out);
			break;
			
		case KEY1('Y'):
			print_flag(out, flags.type);
			break;
			
		case KEY2('M','F'):
			print_yesno(out, flags.mr_checker != NULL);
			break;

		case KEY2('M','P'):
			print_flag(out, flags.mr_checker);
			break;
			
		case KEY2('K','F'):
			print_yesno(out, flags.id_keywords != NULL);
			break;

		case KEY2('K','V'):
			print_flag(out, flags.id_keywords);
			break;

		case KEY2('B','F'):
			print_yesno(out, flags.branch);
			break;

		case KEY1('J'):
			print_yesno(out, flags.joint_edit);
			break;
			
		case KEY2('L','K'):
			if (flags.all_locked) {
				putc('a', out);
			} else {
				print_flag(out, flags.locked);
			}
			break;

		case KEY1('Q'):
			print_flag(out, flags.user_def);
			break;

		case KEY1('M'):
			print_flag(out, get_module_name());
			break;
			
		case KEY2('F','B'):
			print_flag(out, flags.floor);
			break;
			
		case KEY2('C','B'):
			print_flag(out, flags.ceiling);
			break;

		case KEY2('D','s'):
			print_flag(out, flags.default_sid);
			break;

		case KEY2('N','D'):
			print_yesno(out, flags.null_deltas);
			break;

		case KEY2('F','D'):
			print_string_list(out, comments);
			break;

		case KEY2('B','D'):
			seek_to_body();
			while(read_line() != -1) {
				fputs(linebuf, out);
				putc('\n', out);
			}
			break;

		case KEY2('G','B'):
			get(out, "-", delta.seq);
			break;

		case KEY1('W'):
			print_delta(out, ":Z::M:\t:I:", delta);
			break;

		case KEY1('A'):
			print_delta(out, ":Z::Y: :M: :I::Z:", delta);
			break;

		case KEY1('Z'):
			fputc('@', out);
			fputs("(#)", out);
			break;

		case KEY1('F'):
			fputs(base_part(name), out);
			break;

		case KEY2('P','N'):
			fputs(name, out);
			break;
		}
	}
}


/* Prints out parts of the SCCS file.  */

void		
sccs_file::prs(FILE *out, mystring format, sid rid, sccs_date cutoff,
	       enum when when, int all_deltas) {
	if (!rid.valid()) {
		rid = find_most_recent_sid(rid);
	}

	if (when != SIDONLY && !cutoff.valid()) {
		struct delta const *delta = delta_table.find(rid);
		if (delta == NULL) {
			quit(-1, "%s: Requested SID doesn't exist.",
			     (const char *) name);
		}
		cutoff = delta->date;
	}

	delta_iterator iter(delta_table);
	while(iter.next(all_deltas)) {
		switch(when) {
		case EARLIER:
			if (iter->date > cutoff) {
				continue;
			}
			break;

		case SIDONLY:
			if (rid != iter->id) {
				continue;
			}
			break;

		case LATER:
			if (iter->date < cutoff) {
				continue;
			}
			break;
		}

#ifdef __GNUC__
		print_delta(out, format, *(iter.*&sccs_file::delta_iterator::operator->)());
#else
		print_delta(out, format, *iter.operator->());
#endif
		putc('\n', out);
	}
}

// Explicit template instantiations.
template void print_flag2(FILE *out, const char *s, release);
template void print_flag2(FILE *out, const char *s, sid);
template void print_flag2(FILE *out, const char *s, range_list<release>);
template void print_flag (FILE *out, range_list<release>);
template void print_flag (FILE *out, release);
template void print_flag (FILE *out, sid);

/* Local variables: */
/* mode: c++ */
/* End: */
