/*
 * rmdel.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Removes a delta from an SCCS file.
 *
 */

#include "mysc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "pfile.h"
#include "getopt.h"

const char main_sccs_id[] = "@(#) MySC rmdel.c 1.1 93/11/09 17:17:58";

void
usage() {
	fprintf(stderr,
"usage: %s [-V] -r SID file ...\n",
		prg_name);
}

int
main(int argc, char **argv) {
	int c;
	sid rid = NULL;

	if (argc > 0) {
		set_prg_name(argv[0]);
	} else {
		set_prg_name("rmdel");
	}

	class getopt opts(argc, argv, "r:V");
	for(c = opts.next(); c != getopt::END_OF_ARGUMENTS; c = opts.next()) {
		switch (c) {
#if 0		
		default:
			quit(-2, "Unsupported option: '%c'", c);
#endif

		case 'r':
			rid = sid(opts.getarg());
			if (!rid.valid()) {
				quit(-2, "Invaild SID: '%s'", opts.getarg());
			}
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

	int tossed_privileges = 0;

	while(iter.next()) {
		if (tossed_privileges) {
			restore_privileges();
			tossed_privileges = 0;
		}

		sccs_name &name = iter.get_name();
		sccs_file file(name, sccs_file::UPDATE);
		sccs_pfile pfile(name, sccs_pfile::READ);

		pfile.rewind();
		while(pfile.next()) {
			if (pfile->got == rid) {
				quit(-1, "%s: Requested SID is locked"
				         " for editing.",
				     (const char *) name);
			}
		}
		
		if (!file.is_delta_creator(get_user_name(), rid)) {
			give_up_privileges();
			tossed_privileges = 1;
		}

		file.rmdel(rid);
	}

	return 0;
}

template class range_list<sid>;


/* Local variables: */
/* mode: c++ */
/* End: */
