/*
 * get.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Extract a requested delta from a SCCS file.
 * 
 */

#include "cssc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "pfile.h"
#include "getopt.h"
#include "version.h"

const char main_rcs_id[] = "$Id: get.cc,v 1.7 1997/05/13 17:40:15 james Exp $";

/* Prints a list of included or excluded SIDs. */

static void
print_id_list(const char *s, list<sid> const &list) {
	int i, len;
	
	len = list.length();
	if (len > 0) {
#if 0
		/* Printing out the IDs all on one line instead of
                 *  one per line should make the output more clear
	         * ... but for the moment we'll go for compatibilty. 
		 * In the long run we'll use an environment variable.
		 * -- JAY
		 */
	     
		printf("%s:", s);
		for(i = 0; i < len; i++) {
			putchar(' ');
			list[i].print(stdout);
		}
		putchar('\n');
#else
		/* Compatible output */
		printf("%s:\n", s);
		for(i = 0; i < len; i++) {
			list[i].print(stdout);
			putchar('\n');
		}
#endif
	}
}

void
usage() {
	fprintf(stderr,
"usage: %s [-begkmnpstV] [-c date] [-r SID] [-i range] [-w string]\n"
"\t[-x range] [-G gfile] file ...\n",
                prg_name);
}

int
main(int argc, char **argv) {
	int c;
	sid rid = NULL;			/* -r */
	int for_edit = 0;		/* -e */
	int branch = 0;			/* -b */
	int suppress_keywords = 0;      /* -k */
	int use_stdout = 0;		/* -p */
	int silent = 0;			/* -s */
	int no_output = 0;		/* -g */
	const char *wstring = NULL;	/* -w */
	sid_list include, exclude;	/* -i, -x */
	sccs_date cutoff;		/* -c */
	int show_sid = 0;		/* -m */
	int show_module = 0;		/* -n */
	int debug = 0;			/* -D */
	mystring gname;		        /* -G */
	int got_gname = 0;              /* -G */
#if 0
	int seq_no = 0;			/* -a */
#endif

	if (argc > 0) {
		set_prg_name(argv[0]);
	} else {
		set_prg_name("get");
	}

	class getopt opts(argc, argv, "r:c:i:x:ebklpsmngtw:a:DVG:");
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

		case 'c':
			cutoff = sccs_date(opts.getarg());
			if (!cutoff.valid()) {
				quit(-2, "Invalid cutoff date: '%s'",
				     opts.getarg());
			}
			break;

		case 'i':
			include = sid_list(opts.getarg());
			if (!include.valid()) {
				quit(-2, "Invalid inclusion list: '%s'",
				     opts.getarg());
			}
			break;

		case 'x':
			exclude = sid_list(opts.getarg());
			if (!exclude.valid()) {
				quit(-2, "Invalid exclusion list: '%s'",
				     opts.getarg());
			}
			break;

		case 'e':
			for_edit = 1;
			suppress_keywords = 1;
			break;

		case 'b':
			branch = 1;
			break;

		case 'k':
			suppress_keywords = 1;
			break;

		case 'p':
			use_stdout = 1;
			got_gname = 0;
			break;

		case 's':
			silent = 1;
			break;

		case 'm':
			show_module = 1;
			break;

		case 'n':
			show_sid = 1;
			break;

		case 'g':
			no_output = 1;
			break;

		case 'w':
			wstring = opts.getarg();
			break;

#if 0		       
		case 'a':
			int i = atoi(opts.getarg());
			if (i < 1) {
				quit(-2, "Invalid sequence number: '%s'",
				     optarg);
			}
			seq_no = i;
			break;
#endif

		case 'G':
		  	got_gname = 1;
			use_stdout = 0;
			gname = opts.getarg();
			break;
			
		case 'D':
			debug = 1;
			break;
			
		case 'V':
			version();
			break;
		}
	}

	FILE *out = stdin;	/* The output file.  It's initialized with
				   stdin so if it's accidently used before
				   being set it will quickly cause an error. */

	if (use_stdout) {
		gname = "-";
		out = stdout_to_stderr();
	}

	if (silent) {
		stdout_to_null();
	}

	if (no_output) {
		if (use_stdout) {
			fclose(out);
		}
		got_gname = 0;
		gname = "null";
		out = open_null();
	}

	sccs_file_iterator iter(argc, argv, opts.get_index());

	while(iter.next()) {
		sccs_name &name = iter.get_name();

		sccs_pfile *pfile = NULL;
		if (for_edit) {
			pfile = new sccs_pfile(name, sccs_pfile::APPEND);
		}

		sccs_file file(name, sccs_file::READ);
		sid new_delta;
		
		sid retrieve;

		if (!file.find_requested_sid(rid, retrieve)) {
		  quit(-1, "%s: Requested SID not found.",
		       (const char *) name);
		}
		
		if (for_edit) {
			file.test_locks(retrieve, *pfile);
			new_delta = file.find_next_sid(rid, retrieve,
						       branch, *pfile);
		}

		if (!use_stdout && !no_output) {
			assert(name.valid());

			/* got_gname is specified if we had -G g-file
			 * on the command line.   This only works for the
			 * first file on the command line (or else we'd
			 * be overwriting easrlier data.
			 */
			if (!got_gname)
			  gname = name.gfile();
			got_gname = 0;
			
			int mode = CREATE_AS_REAL_USER | CREATE_FOR_GET;
			if (!suppress_keywords) {
				mode |= CREATE_READ_ONLY;
			}

			out = fcreate(gname, mode);

			if (out == NULL) {
				quit(errno, "%s: Can't open file for"
					    " writing",
				     (const char *)gname);
			}
		}
		const int keywords = !suppress_keywords;
		struct sccs_file::get_status status
			= file.get(out, gname, retrieve, cutoff,
				   include, exclude, keywords, wstring,
				   show_sid, show_module, debug);

		if (!use_stdout && !no_output) {
			fclose(out);
#ifdef CONFIG_USE_ARCHIVE_BIT
			if (!keywords) {
				clear_archive_bit(gname);
			}
#endif
		}
		
		// Print the name of the SCCS file unless exactly one was specified.
		//const int ix = opts.get_index();
		if ((argc-1) != opts.get_index())
		  {
		    fprintf(stdout, "\n%s:\n", (const char*)name);
		  }
		
		print_id_list("Included", status.included);
		print_id_list("Excluded", status.excluded);
		retrieve.print(stdout);
		putchar('\n');

		if (for_edit) {
			printf("new delta ");
			new_delta.print(stdout);
			putchar('\n');

			pfile->add_lock(retrieve, new_delta, include, exclude);
			delete pfile;
		}

		if (!no_output) {
			printf("%d lines\n", status.lines);
		}
	}

	return 0;
}


// Explicit template instantiations.
template class range_list<sid>;
template class list<sid>;
template class list<mystring>;
template class list<seq_no>;
template class list<sccs_file::delta>;
template class list<sccs_pfile::edit_lock>;

#include "stack.h"
template class stack<unsigned short>;

#include "sid_list.h"
template class range_list<release>;

/* Local variables: */
/* mode: c++ */
/* End: */
