/*
 * rmdel.c: Part of GNU CSSC.
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
 * Removes a delta from an SCCS file.
 *
 */

#include "cssc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "pfile.h"
#include "my-getopt.h"
#include "version.h"
#include "delta.h"

const char main_rcs_id[] = "CSSC $Id: rmdel.cc,v 1.9 1998/01/17 11:42:42 james Exp $";

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

	class getopt opts(argc, argv, "r!V");
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
				     name.c_str());
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

// Explicit template instantiations.
template class range_list<sid>;
template class list<sccs_pfile::edit_lock>;
template class list<mystring>;
template class list<seq_no>;
template class list<delta>;
template class range_list<release>;

/* Local variables: */
/* mode: c++ */
/* End: */
