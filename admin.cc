/*
 * admin.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Administer and create SCCS files.
 *
 */ 

#include "cssc.h"
#include "sccsfile.h"
#include "sf-chkmr.h"
#include "fileiter.h"
#include "getopt.h"
#include "version.h"

const char main_rcs_id[] = "CSSC $Id: admin.cc,v 1.10 1997/05/31 10:23:22 james Exp $";

void
usage() {
	fprintf(stderr,
"usage: %s [-nrzV] [-a users] [-d flags] [-e users] [-f flags]\n"
"\t[-i file] [-m MRs] [-t file] [-y comments] file ...\n",
	prg_name);
}

int
main(int argc, char **argv) {
	int c;
	release first_release = NULL;		/* -r */
	int new_sccs_file = 0;			/* -n */
	const char *iname = NULL;		/* -i -I */
	const char *file_comment = NULL;	/* -t */
	list<mystring> set_flags, unset_flags;	/* -f, -d */
	list<mystring> add_users, erase_users;	/* -a, -e */
	mystring mrs, comments;			/* -m, -y */
	const int check = 0;			/* -h */
	int reset_checksum = 0;			/* -z */
	int suppress_mrs = 0;	                /* -m (no arg) */
	int suppress_comments = 0;              /* -y (no arg) */
	int empty_t_option = 0;	                /* -t (no arg) */ 
	
	if (argc > 0) {
		set_prg_name(argv[0]);
	} else {
		set_prg_name("admin");
	}

	class getopt opts(argc, argv, "ni!r!t!f!d:a:e:m!y!hzV");
	for(c = opts.next(); c != getopt::END_OF_ARGUMENTS; c = opts.next()) {
		switch (c) {
		default:
			quit(-2, "Unsupported option: '%c'", c);

		case 'n':
			new_sccs_file = 1;
			break;

		case 'i':
			if (strlen(opts.getarg()) > 0)
			  iname = opts.getarg();
			else
			  iname = "-";
			new_sccs_file = 1;
			break;

		case 'r':
		        {
			  const char *rel = opts.getarg();
			  first_release = release(rel);
			  if (!first_release.valid())
			    {
			      quit(-1, "Invaild release: '%s'", rel);
			    }
			  break;
			}

		case 't':
			file_comment = opts.getarg();
			empty_t_option = (0 == strlen(file_comment));
			break;
		       
		case 'f':
			set_flags.add(opts.getarg());
			break;

		case 'd':
			unset_flags.add(opts.getarg());
			break;

		case 'a':
			add_users.add(opts.getarg());
			break;

		case 'e':
			erase_users.add(opts.getarg());
			break;

		case 'm':
			mrs = opts.getarg();
			suppress_mrs = (mrs == "");
			break;

		case 'y':
			comments = opts.getarg();
			suppress_comments = (comments == "");
			break;

#if 0
		case 'h':
			check = 1;
			break;
#endif

		case 'z':
			reset_checksum = 1;
			break;

		case 'V':
			version();
			break;
		}
	}

	if (empty_t_option && new_sccs_file)
	  {
	    quit(-1, "The -t option must have an argument if -n or -i is used.");
	  }
	
	list<mystring> mr_list, comment_list;
	int first = 1;
	sccs_file_iterator iter(argc, argv, opts.get_index());

	while(iter.next()) {
		sccs_name &name = iter.get_name();

		if (reset_checksum && !check) {
			if (!name.valid()) {
				quit(-1, "%s: Not a SCCS file.",
				     (const char *) name);
			}
			sccs_file::update_checksum(name);
			continue;
		}
			
		/* enum */ sccs_file::_mode mode = sccs_file::UPDATE;
#if 0
		if (check) {
			mode = sccs_file::READ;
		} else
#endif
		if (new_sccs_file) {
			mode = sccs_file::CREATE;
		}

		sccs_file file(name, mode);

#if 0
		if (check) {
			file.check();
			continue;
		}
#endif

		file.admin(file_comment, set_flags, unset_flags,
			   add_users, erase_users);

		if (new_sccs_file) {
			if (iname != NULL && !first) {
				quit(-1, "The 'i' keyletter can't be used with"
					 " multiple files.");
			}

			if (first) {
			  // The real thing does not prompt the user here.
			  // Hence we don't either.
#if 0
				if (stdin_is_a_tty()) {
					if (!suppress_mrs && mrs == NULL
					    && file.mr_required()) {
						mrs = prompt_user("MRs? ");
					}
				}
#endif
				if (!file.mr_required() && mrs != NULL)
				  quit(-1, "MRs not enabled with 'v' flag, "
				       "can't use 'm' keyword.");
				
				mr_list = split_mrs(mrs);
				comment_list = split_comments(comments);
				first = 0;
			}

			if (file.mr_required()) {
				if (!suppress_mrs && mr_list.length() == 0) {
					quit(-1, "%s: MR number(s) must be "
						 " supplied.",
					     (const char *) name);
				}
				if (file.check_mrs(mr_list)) {
					quit(-1, "%s: Invalid MR number(s).",
					     (const char *) name);
				}
			}

			file.create(first_release, iname,
				    mr_list, comment_list,
				    suppress_comments);
		} else {
			file.update();
		}		
		first = 0;
	}

	return 0;
}

#undef EARLY_GXX
#ifdef __GNUG__
#if (__GNUG__ <= 2)
#if (__GNUC_MINOR__ <= 6)
#define EARLY_GXX
#endif
#endif
#endif

// Explicit template instantiations.
template class list<mystring>;
template class list<seq_no>;
template class list<sccs_file::delta>;
template list<char const*>& operator+=(list<char const *> &, list<mystring> const &);
template class list<char const *>;
template list<mystring>& operator+=(list<mystring> &, list<mystring> const &);
template list<mystring>& operator-=(list<mystring> &, list<mystring> const &);
template class range_list<release>;
#ifndef EARLY_GXX
template range_list<release> & range_list<release>::merge(range_list<release> const &);
template range_list<release> & range_list<release>::remove(range_list<release> const &);
#endif
/* Local variables: */
/* mode: c++ */
/* End: */
