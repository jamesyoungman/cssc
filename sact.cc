/*
 * sact.c: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Prints the current edit locks for an SCCS file. 
 *
 */

#include "cssc.h"
#include "fileiter.h"
#include "pfile.h"
#include "version.h"

const char main_rcs_id[] = "CSSC $Id: sact.cc,v 1.5 1997/07/02 18:04:17 james Exp $";

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

// Explicit template instantiations.
template class range_list<sid>;
template class list<sccs_pfile::edit_lock>;
template class list<mystring>;

/* Local variables: */
/* mode: c++ */
/* End: */
