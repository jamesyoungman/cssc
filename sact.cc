/*
 * sact.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Prints the current edit locks for an SCCS file. 
 *
 */

#include "mysc.h"
#include "fileiter.h"
#include "pfile.h"

char const main_sccs_id[] = "@(#) MySC sact.c 1.1 93/11/09 17:17:58";

void
usage() {
	fprintf(stderr,
"usage: %s [-V] file ...\n",
		prg_name);
}

int
main(int argc, char **argv) {
	if (argc > 0) {
		set_prg_name(argv[0]);
	} else {
		set_prg_name("sact");
	}

	if (argc > 1 && strcmp(argv[1], "-V") == 0) {
		version();
		argc--;
		argv++;
	}

	sccs_file_iterator iter(argc, argv);

	while(iter.next()) {
		sccs_name &name = iter.get_name();
		sccs_pfile pfile(name, sccs_pfile::READ);

		pfile.rewind();
		while(pfile.next()) {
			pfile->got.print(stdout);
			putchar(' ');
			pfile->delta.print(stdout);
			putchar(' ');
			fputs(pfile->user, stdout);
			putchar(' ');
			pfile->date.print(stdout);
			putchar('\n');
		}
	}

	return 0;
}

/* Local variables: */
/* mode: c++ */
/* End: */
