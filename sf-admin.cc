/*
 * sf-admin.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_file for performing creation and
 * adminstration operations on the SCCS file.
 *
 */

#include "mysc.h"
#include "sccsfile.h"
#include "sl-merge.c"

#ifdef CONFIG_NO_STRSTR
#include "strstr.c"
#endif

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC sf-admin.c 1.1 93/11/09 17:18:00";
#endif

/* Changes the file comment, flags, and/or the user authorization list
   of the SCCS file. */

void
sccs_file::admin(const char *file_comment,
		 list<mystring> set_flags, list<mystring> unset_flags,
		 list<mystring> add_users, list<mystring> erase_users) {
	int i;
	int len;

	if (file_comment != NULL) {
		comments = NULL;
		if (file_comment[0] != '\0') {
			FILE *fc = fopen(file_comment, "r");
			if (fc == NULL) {
				quit(errno, "%s: Can't open comment file.",
				     file_comment);
			}

			while(!read_line_param(fc)) {
				comments.add((const char *)linebuf);
			}

			if (ferror(fc)) {
				quit(errno, "%s: Read error.", file_comment);
			}
			fclose(fc);
		}
	}

	len = set_flags.length();
	for(i = 0; i < len; i++) {
		const char *s = set_flags[i];

		switch(*s++) {

		case 'b':
			flags.branch = 1;
			break;

		case 'c':
			flags.ceiling = release(s);
			if (!flags.ceiling.valid()) {
				quit(-1, "Invalid release ceiling: '%s'", s);
			}
			break;

		case 'f':
			flags.floor = release(s);
			if (!flags.floor.valid()) {
				quit(-1, "Invalid release floor: '%s'", s);
			}
			break;


		case 'd':
			flags.default_sid = sid(s);
			if (!flags.default_sid.valid()) {
				quit(-1, "Invalid default SID: '%s'", s);
			}
			break;

		case 'i':
			flags.id_keywords = s;
			break;


		case 'j':
			flags.joint_edit = 1;
			break;

		case 'l':
			if (strcmp(s, "a") == 0) {
				flags.all_locked = 1;
				flags.locked = NULL;
			} else {
				flags.locked.merge(release_list(s));
			}
			break;

		case 'n':
			flags.null_deltas = 1;
			break;


		case 'q':
			flags.user_def = s;
			break;

		case 't':
			flags.type = s;
			break;

		case 'v':
			flags.mr_checker = s;
			break;

		default:
			quit(-1, "Unrecognized flag '%c'", s[-1]);
		}
	}
	      
	len = unset_flags.length();
	for(i = 0; i < len; i++) {
		const char *s = unset_flags[i];

		switch(*s++) {

		case 'b':
			flags.branch = 0;
			break;

		case 'c':
			flags.ceiling = NULL;
			break;

		case 'f':
			flags.floor = NULL;
			break;


		case 'd':
			flags.default_sid = NULL;
			break;

		case 'i':
			flags.id_keywords = NULL;
			break;

		case 'j':
			flags.joint_edit = 0;
			break;

		case 'l':
			if (strcmp(s, "a") == 0) {
				flags.all_locked = 0;
				flags.locked = NULL;
			} else {
				flags.locked.remove(release_list(s));
			}
			break;

		case 'n':
			flags.null_deltas = 0;
			break;


		case 'q':
			flags.user_def = NULL;
			break;

		case 't':
			flags.type = NULL;
			break;

		case 'v':
			flags.mr_checker = NULL;
			break;

		default:
			quit(-1, "Unrecognized flag '%c'", s[-1]);
		}
	}
	      
	users += add_users;
	users -= erase_users;
}


/* Creates a new SCCS file. */

void
sccs_file::create(release first_release, const char *iname,
		  list<mystring> mrs, list<mystring> comments) {

	sccs_date now = sccs_date::now();
	if (comments.length() == 0) {
		mystring one("date and time created ", now.as_string()),
		       two(" by ", get_user_name());
		mystring it(one, two);
		comments.add(it);
	}

	sid id = sid(first_release).successor();

	struct delta new_delta('D', id, now, get_user_name(), 1, 0,
			       mrs, comments);
	new_delta.inserted = 0;
	new_delta.deleted = 0;
	new_delta.unchanged = 0;

	FILE *out = start_update(new_delta);

	fprintf(out, "\001I 1\n");

	if (iname != NULL) {
		FILE *in;

		if (strcmp(iname, "-") == 0) {
			in = stdin;
		} else {
			in = fopen(iname, "r");
			if (in == NULL) {
				quit(errno, "%s: Can't open file for reading.",
				     iname);
			}
		}

		int found_id = 0;
		const char *req_id = flags.id_keywords;
		if (req_id != NULL && req_id[0] == '\0') {
			req_id = NULL;
		}

		while(!read_line_param(in)) {
			new_delta.inserted++;
			if (fputs(linebuf, out) == EOF
			    || putc('\n', out) == EOF) {
				mystring zname = name.zfile();
				quit(errno, "%s: Write error.",
				     (const char *) zname);
			}
			if (!found_id) {
				if (req_id == NULL) {
					if (check_id_keywords(linebuf)) {
						found_id = 1;
					}
				} else if (strstr(linebuf, req_id) != NULL) {
					found_id = 1;
				}
			}
		}
		if (ferror(in)) {
			quit(errno, "%s: Read error.", iname);
		}
		if (in != stdin) {
			fclose(in);
		}

		if (!found_id) {
			if (req_id != NULL) {
				quit(-1, "%s: Required keywords \"%s\""
				         " missing.",
				     (const char *) name, req_id);
			}
			fprintf(stderr, "%s: Warning: No id keywords.\n",
				(const char *) name);
		}
	}

	fprintf(out, "\001E 1\n");

	end_update(out, new_delta);
}

/* Local variables: */
/* mode: c++ */
/* End: */
