/*
 * fileiter.c: Part of GNU CSSC.
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
 * Members of the class fileiter.
 *
 */

#ifdef __GNUC__
#pragma implementation "fileiter.h"
#endif

#include "cssc.h"
#include "sccsname.h"
#include "fileiter.h"
#include "linebuf.h"
#include "my-getopt.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: fileiter.cc,v 1.11 1998/02/21 14:03:54 james Exp $";
#endif

#if 0
sccs_file_iterator::sccs_file_iterator(int ac, char **av, int ind)
	: argv(av + ind), argc(ac - ind) {

	if (argc < 1) {
		quit(-2, "No SCCS file specified.");
	}

	char *first = argv[0];

	if (strcmp(first, "-") == 0) {
		source = STDIN;
		return;
	}

#ifndef CONFIG_NO_DIRECTORY
	if (first[0] != '\0') {
		DIR *dir = opendir(first);
		if (dir != NULL) {
			const char *slash = NULL;
			int len = strlen(first);

#ifdef CONFIG_MSDOS_FILES
			if (first[len - 1] != '/' && first[len - 1] != '\\') {
				if (strchr(first, '/') == NULL) {
					slash = "\\";
				} else {
					slash = "/";
				}
			}
#else
			if (first[len - 1] != '/') {
				slash = "/";
			}
#endif
			
			mystring dirname(mystring(first) + mystring(slash));

			struct dirent *dent = readdir(dir);
			while(dent != NULL) {
				mystring name = mystring(dirname) + mystring(dent->d_name, NAMLEN(dent));
				
				if (sccs_name::valid_filename(name.c_str())
				    && is_readable(name.c_str())) {
					files.add(name);
				}
				dent = readdir(dir);
			}

			closedir(dir);

			source = DIRECTORY;
			pos = 0;
			return;
		}
	}
#endif /* CONFIG_NO_DIRECTORY */

	source = ARGS;
	is_unique = (1 == argc);
}
#else
sccs_file_iterator::sccs_file_iterator(const getopt &opts)
	: argv(opts.get_argv() + opts.get_index()),
	  argc(opts.get_argc() - opts.get_index()) {

	if (argc < 1) {
		quit(-2, "No SCCS file specified.");
	}

	char *first = argv[0];

	if (strcmp(first, "-") == 0) {
		source = STDIN;
		return;
	}

#ifndef CONFIG_NO_DIRECTORY
	if (first[0] != '\0') {
		DIR *dir = opendir(first);
		if (dir != NULL) {
			const char *slash = NULL;
			int len = strlen(first);

#ifdef CONFIG_MSDOS_FILES
			if (first[len - 1] != '/' && first[len - 1] != '\\') {
				if (strchr(first, '/') == NULL) {
					slash = "\\";
				} else {
					slash = "/";
				}
			}
#else
			if (first[len - 1] != '/') {
				slash = "/";
			}
#endif
			
			mystring dirname(mystring(first) + mystring(slash));

			struct dirent *dent = readdir(dir);
			while(dent != NULL) {
				mystring name = mystring(dirname) + mystring(dent->d_name, NAMLEN(dent));
				
				if (sccs_name::valid_filename(name.c_str())
				    && is_readable(name.c_str())) {
					files.add(name);
				}
				dent = readdir(dir);
			}

			closedir(dir);

			source = DIRECTORY;
			pos = 0;
			return;
		}
	}
#endif /* CONFIG_NO_DIRECTORY */

	source = ARGS;
	is_unique = (1 == argc);
}
#endif


int
sccs_file_iterator::unique() const
{
  return is_unique;
}


int
sccs_file_iterator::next() {
	switch(source) {
	case STDIN:
	  {
	    static cssc_linebuf linebuf; // FIXME: static!

	    if (linebuf.read_line(stdin)) {
	      return 0;
	    }

	    name = linebuf.c_str();
	    return 1;
	  }
	
#ifndef CONFIG_NO_DIRECTORY	
	case DIRECTORY:
		if (pos < files.length()) {
			name = files[pos++];
			return 1;
		}
		return 0;
#endif

	case ARGS:
		if (argc-- <= 0) {
			return 0;
		}
		name = *argv++;

#ifdef CONFIG_FILE_NAME_GUESSING
		if (!name.valid())
		  {
		    name.make_valid();
		  }
#endif
		return 1;

	default:
		abort();
	}

	return 0;
}
					
/* Local variables: */
/* mode: c++ */
/* End: */
