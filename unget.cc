/*
 * unget.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Removes edit locks from SCCS files.
 *
 */

#include "cssc.h"
#include "fileiter.h"
#include "pfile.h"
#include "getopt.h"
#include "version.h"

const char main_rcs_id[] = "CSSC $Id: unget.cc,v 1.6 1997/05/27 19:21:55 james Exp $";

void
usage() {
	fprintf(stderr,
"usage: %s [-nsV] [-r SID] file ...\n",
		prg_name);
}

int
main(int argc, char **argv) {
	int c;
	sid rid = NULL;
	int silent = 0;
	int keep_gfile = 0;

	if (argc > 0) {
		set_prg_name(argv[0]);
	} else {
		set_prg_name("unget");
	}

	class getopt opts(argc, argv, "r:snV");
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

		case 'V':
			version();
			break;
		}
	}

	sccs_file_iterator iter(argc, argv, opts.get_index());

     	if (silent) {
		stdout_to_null();
	}

	while(iter.next()) {
		sccs_name &name = iter.get_name();
		sccs_pfile pfile(name, sccs_pfile::UPDATE);

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
			if (!rid.valid()) {
				quit(-1, "%s: Specified SID is ambiguous.",
				     (const char *) name);
			}
			quit(-1, "%s: You must specify a SID on the"
			         " command line.", (const char *) name);
			break;

		default:
			abort();
		}
		if (!iter.unique())
		  printf("\n%s:\n", (const char*)name);
		pfile.print_lock_sid(stdout);
		fputc('\n', stdout);

		pfile.delete_lock();
		pfile.update();

		if (!keep_gfile) {
			mystring gname = name.gfile();
#ifndef TESTING
			remove(gname);
#endif
		}
	}

	return 0;
}


// Explicit template instantiations.
template class range_list<sid>;
template class list<sccs_pfile::edit_lock>;
template class list<mystring>;



/* Local variables: */
/* mode: c++ */
/* End: */
