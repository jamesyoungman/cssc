/*
 * sf-admin.c: Part of GNU CSSC.
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
 * Members of the class sccs_file for performing creation and
 * adminstration operations on the SCCS file.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "sl-merge.h"

// We use @LIBOBJS@ instead now.
// #ifndef HAVE_STRSTR
// #include "strstr.cc"
// #endif

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-admin.cc,v 1.13 1997/11/11 23:23:58 james Exp $";
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
		  if (strlen(s))
		    quit(-1, "Flag 'i' does not take an argument.");
		  else
		    flags.no_id_keywords_is_fatal = 1;
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

		case 'e':
		  	quit(-1, "The encoding flag must be set "
			     "with the -b option");
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
			flags.no_id_keywords_is_fatal = 0;
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

		case 'e':
		  	quit(-1, "Deletion of the binary-encoding flag "
			     "is not supported.");
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
		  list<mystring> mrs, list<mystring> comments,
		  int suppress_comments)
{

  sccs_date now = sccs_date::now();
  if (!suppress_comments && comments.length() == 0)
    {
      // The two casts to (char*) on the following line are required
      // by GCC 2.6.3, according to Dave Bodenstab <imdave@mcs.net>.
      mystring one("date and time created ", (char*)now.as_string()),
	two(" by ", (char*)get_user_name());
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

    if (strcmp(iname, "-") == 0)
      {
	in = stdin;
      }
    else
      {
      in = fopen(iname, "r");
      if (in == NULL)
	{
	  quit(errno, "%s: Can't open file for reading.",
	       iname);
	}
      }

    int found_id = 0;

		
    while(!read_line_param(in))
      {
	new_delta.inserted++;
	if (fputs(linebuf, out) == EOF || putc('\n', out) == EOF)
	  {
	    mystring zname = name.zfile();
	    quit(errno, "%s: Write error.",
		 (const char *) zname);
	  }
	if (!found_id)
	  {
	    if (check_id_keywords(linebuf))
	      {
		found_id = 1;
	      }
	  }
      }

    if (ferror(in))
      {
	quit(errno, "%s: Read error.", iname);
      }
    if (in != stdin)
      {
	fclose(in);
      }
    
    if (!found_id)
      {
	no_id_keywords(name);
      }
  }
	
  fprintf(out, "\001E 1\n");
  
  end_update(out, new_delta);
}

/* Local variables: */
/* mode: c++ */
/* End: */
