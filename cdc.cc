/*
 * cdc.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Changes the comments and MRs of a delta.
 *
 */

#include "mysc.h"
#include "getopt.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "sf-chkmr.h"

const char main_sccs_id[] = "@(#) MySC cdc.c 1.1 93/11/09 17:17:53";

void
usage() {
	fprintf(stderr,
"usage: %s [-MYV] [-m MRs] [-y comments] -r SID file ...\n",
		prg_name);
}

int
main(int argc, char **argv) {
	int c;
	sid rid = NULL;
	mystring mrs;
	mystring comments;
	
	if (argc > 0) {
		set_prg_name(argv[0]);
	} else {
		set_prg_name("cdc");
	}

	class getopt opts(argc, argv, "r:m:My:YV");
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

	if (!rid.valid()) {
		quit(-2, "A SID must be specified on the command line.");
	}

	sccs_file_iterator iter(argc, argv, opts.get_index());

	list<mystring> mr_list, comment_list;
	int first = 1;
	int tossed_privileges = 0;

	while(iter.next()) {
		if (tossed_privileges) {
			restore_privileges();
			tossed_privileges = 0;
		}

		sccs_name &name = iter.get_name();
		sccs_file file(name, sccs_file::UPDATE);
		
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

		if (file.mr_required() && mr_list.length() != 0) {
			if (file.check_mrs(mr_list)) {
				quit(-1, "%s: Invalid MR number(s).",
				     (const char *) name);
			}
		}

		if (!file.is_delta_creator(get_user_name(), rid)) {
			give_up_privileges();
			tossed_privileges = 1;
		}

		file.cdc(rid, mr_list, comment_list);
		file.update();
	}

	return 0;
}

// Explicit template instantiations.
template class list<mystring>;
template class list<seq_no>;
template class list<sccs_file::delta>;
template class range_list<release>;
template class list<const char*>;
template list<mystring>& operator+=(list<mystring> &, list<mystring> const &);
template list<mystring>& operator-=(list<mystring> &, list<mystring> const &);
template list<char const*>& operator+=(list<char const *> &, list<mystring> const &);

/* Local variables: */
/* mode: c++ */
/* End: */
