/*
 * delta.c: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998 Free Software Foundation, Inc. 
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
 * Adds new deltas to SCCS files.
 *
 */

#include "cssc.h"
#include "my-getopt.h"
#include "fileiter.h"
#include "pfile.h"
#include "sccsfile.h"
#include "sf-chkmr.h"
#include "version.h"
#include "delta.h"

const char main_rcs_id[] = "CSSC $Id: delta.cc,v 1.14 1998/02/21 14:27:04 james Exp $";

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
	int suppress_mrs = 0;	// if -m given with no arg.
	int got_mrs = 0;	// if no need to prompt for MRs.
	int suppress_comments = 0; // if -y given with no arg.
	int got_comments = 0;

	if (argc > 0) {
		set_prg_name(argv[0]);
	} else {
		set_prg_name("delta");
	}

	class getopt opts(argc, argv, "r!sng!m!My!YpV");
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
			suppress_mrs = (mrs == "");
			got_mrs = 1;
			break;

		case 'M':
			mrs = "";
			suppress_mrs = 1;
			got_mrs = 1;
			break;

		case 'y':
			comments = opts.getarg();
			suppress_comments = (comments == "");
			got_comments = 1;
			break;

		case 'Y':
			comments = "";
			suppress_comments = 1;
			got_comments = 1;
			break;

		case 'V':
			version();
			break;
		}
	}

	sccs_file_iterator iter(opts);

     	if (silent) {
		stdout_to_null();
	}

	list<mystring> mr_list, comment_list;
	int first = 1;

	while(iter.next()) {
		sccs_name &name = iter.get_name();
		sccs_file file(name, sccs_file::UPDATE);
		sccs_pfile pfile(name, sccs_pfile::UPDATE);
		
		if (first)
		  {
		    if (stdin_is_a_tty())
		      {
			if (!suppress_mrs && !got_mrs && file.mr_required())
			  {
			    mrs = prompt_user("MRs? ");
			    got_mrs = 1;
			  }
			if (!suppress_comments && !got_comments)
			  {
			    comments = prompt_user("comments? ");
			    got_comments = 1;
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
				     name.c_str());
			}
			quit(-1, "%s: Specified SID hasn't been locked for"
			         " editing by you.",
			     name.c_str());
			break;

		case sccs_pfile::AMBIGUOUS:
			if (rid.valid()) {
				quit(-1, "%s: Specified SID is ambiguous.",
				     name.c_str());
			}
			quit(-1, "%s: You must specify a SID on the"
			         " command line.", name.c_str());
			break;

		default:
			abort();
		}

		if (!suppress_mrs && file.mr_required())
		  {
		    if (mr_list.length() == 0)
		      {
			quit(-1, "%s: MR number(s) must be supplied.",
			     name.c_str());
		      }
		    if (file.check_mrs(mr_list))
		      {
			/* In this case, _real_ SCCS prints the ID anyway.
                         */
			pfile->delta.print(stdout);
			putchar('\n');
			quit(-1, "%s: Invalid MR number(s).", name.c_str());
		      }
		  }
		else if (mr_list.length())
		  {
		    // MRs were specified and the MR flag is turned off.
		    pfile->delta.print(stdout);
		    putchar('\n');
		    quit(-1,
			 "%s: MR verification ('v') flag not set, MRs"
			 " are not allowed.\n",
			 name.c_str());
		  }
		
		mystring gname = name.gfile();

		file.add_delta(gname, pfile, mr_list, comment_list);

		if (!keep_gfile)
		  {
		    remove(gname.c_str());
		  }
	}

	return 0;
}

// Explicit template instantiations.
template class range_list<sid>;
template class list<mystring>;
template class list<delta>;
template class list<seq_no>;
template class list<sccs_pfile::edit_lock>;
template class list<char const*>;
//template list<char const*>& operator+=(list<char const *> &, list<mystring> const &);
template class range_list<release>;

#include "stack.h"
template class stack<unsigned short>;

/* Local variables: */
/* mode: c++ */
/* End: */
