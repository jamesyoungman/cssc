/*
 * prs.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Prints selected parts of an SCCS file.
 *
 */

#include "mysc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "getopt.h"

const char main_sccs_id[] = "@(#) MySC prs.c 1.1 93/11/09 17:17:57";

void
usage() {
	fprintf(stderr,
"usage: %s [-aelDRV] [-c cutoff] [-d format] [-r SID] file ...\n",
		prg_name);
}

int
main(int argc, char **argv) {
	int c;
	mystring format = ":Dt:\t:DL:\nMRs:\n:MR:COMMENTS:\n:C:";
	sid rid = NULL;
	/* enum */ sccs_file::when selected = sccs_file::SIDONLY;
	int all_deltas = 0;
	sccs_date cutoff;
	int default_processing = 1;

	if (argc > 0) {
		set_prg_name(argv[0]);
	} else {
		set_prg_name("prs");
	}

	class getopt opts(argc, argv, "d:Dr:Relc:aV");
	for(c = opts.next(); c != getopt::END_OF_ARGUMENTS; c = opts.next()) {
		switch (c) {
		default:
			quit(-2, "Unsupported option: '%c'", c);

		case 'd':
			format = opts.getarg();
			default_processing = 0;
			break;

		case 'D':
			default_processing = 0;
			break;

		case 'r':
			rid = sid(opts.getarg());
			if (!rid.valid() || rid.partial_sid()) {
				quit(-2, "Invaild SID: '%s'", opts.getarg());
			}
			default_processing = 0;
			break;

		case 'R':
			rid = NULL;
			default_processing = 0;
			break;
			
		case 'c':
			cutoff = sccs_date(opts.getarg());
			if (!cutoff.valid()) {
				quit(-2, "Invalid cutoff date: '%s'",
				     opts.getarg());
			}
			break;

		case 'e':
			selected = sccs_file::EARLIER;
			default_processing = 0;
			break;

		case 'l':
			selected = sccs_file::LATER;
			default_processing = 0;
			break;

		case 'a':
			all_deltas = 1;
			break;

		case 'V':
			version();
			break;
		}

	}

	if (selected == sccs_file::SIDONLY && cutoff.valid()) {
		quit(-2, "Either the -e or -l switch must used with a"
			 " cutoff date.");
	}

	if (default_processing) {
		selected = sccs_file::EARLIER;
	}

	sccs_file_iterator iter(argc, argv, opts.get_index());

	while(iter.next()) {
		sccs_name &name = iter.get_name();
		sccs_file file(name, sccs_file::READ);

		if (default_processing) {
			printf("%s:\n\n", (const char *) name);
		}
		file.prs(stdout, format, rid, cutoff, selected, all_deltas);
	}

	return 0;
}
template class range_list<sid>;

/* Local variables: */
/* mode: c++ */
/* End: */
