/*
 * delta.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Adds new deltas to SCCS files.
 *
 */

#include "mysc.h"
#include "getopt.h"
#include "fileiter.h"
#include "pfile.h"
#include "sccsfile.h"
#include "sf-chkmr.h"

const char main_sccs_id[] = "@(#) MySC delta.c 1.2 93/11/13 00:11:18";

void
usage() {
	fprintf(stderr,
"usage: %s [-nsMYV] [-m MRs] [-r SID] [-y comments] file ...\n",
		prg_name);
}

int
main(int argc, char **argv) {
	int c;
	sid rid = NULL;		/* -r */
	int silent = 0;		/* -s */
	int keep_gfile = 0;	/* -n */
#if 0
	sid_list ignore;	/* -g */
#endif
	mystring mrs;		/* -m -M */
	mystring comments;	/* -y -Y */
	
	if (argc > 0) {
		set_prg_name(argv[0]);
	} else {
		set_prg_name("delta");
	}

	class getopt opts(argc, argv, "r:sng:m:My:YpV");
	for(c = opts.next(); c != getopt::END_OF_ARGUMENTS; c = opts.next()) {
		switch (c) {
		default:
			quit(-2, "Unsupported option: '%c'", c);
		case 'r':
			rid = sid(opts.getarg());
			if (!rid.valid()) {
				quit(-2, "Invaild SID: '%s'", opts.getarg());
			}
			break;

		case 's':
			silent = 1;
			break;

		case 'n':
			keep_gfile = 1;
			break;

		case 'm':
			mrs = opts.getarg();
			break;

		case 'M':
			mrs = "";
			break;

		case 'y':
			comments = opts.getarg();
			break;

		case 'Y':
			comments = "";
			break;

		case 'V':
			version();
			break;
		}
	}

	sccs_file_iterator iter(argc, argv, opts.get_index());

     	if (silent) {
		stdout_to_null();
	}

	list<mystring> mr_list, comment_list;
	int first = 1;

	while(iter.next()) {
		sccs_name &name = iter.get_name();
		sccs_file file(name, sccs_file::UPDATE);
		sccs_pfile pfile(name, sccs_pfile::UPDATE);
		
		if (first) {
			if (stdin_is_a_tty()) {
				if (mrs == NULL && file.mr_required()) {
					mrs = prompt_user("MRs? ");
				}
				if (comments == NULL) {
					comments = prompt_user("comments? ");
				}
			}
			mr_list = split_mrs(mrs);
			comment_list = split_comments(comments);
			first = 0;
		}

		switch(pfile.find_sid(rid)) {
		case sccs_pfile::FOUND:
			break;

		case sccs_pfile::NOT_FOUND:
			if (!rid.valid()) {
				quit(-1, "%s: You have no edits outstanding.",
				     (const char *) name);
			}
			quit(-1, "%s: Specified SID hasn't been locked for"
			         " editing by you.",
			     (const char *) name);
			break;

		case sccs_pfile::AMBIGUOUS:
			if (rid.valid()) {
				quit(-1, "%s: Specified SID is ambiguous.",
				     (const char *) name);
			}
			quit(-1, "%s: You must specify a SID on the"
			         " command line.", (const char *) name);
			break;

		default:
			abort();
		}

		if (file.mr_required()) {
			if (mr_list.length() == 0) {
				quit(-1, "%s: MR number(s) must be supplied.",
				     (const char *) name);
			}
			if (file.check_mrs(mr_list)) {
				quit(-1, "%s: Invalid MR number(s).",
				     (const char *) name);
			}
		}

		mystring gname = name.gfile();

		file.add_delta(gname, pfile, mr_list, comment_list);

		if (!keep_gfile) {
#ifndef TESTING
			remove(gname);
#endif
		}
	}

	return 0;
}

// Explicit template instantiations.
template class range_list<sid>;
template class list<mystring>;
template class list<sccs_file::delta>;
template class list<seq_no>;
template class list<sccs_pfile::edit_lock>;
template class list<char const*>;
template list<char const*>& operator+=(list<char const *> &, list<mystring> const &);
template class range_list<release>;

#include "stack.h"
template class stack<unsigned short>;

/* Local variables: */
/* mode: c++ */
/* End: */
