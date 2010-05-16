/*
 * fileiter.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,2007,2008 Free Software Foundation, Inc. 
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Members of the class fileiter.
 *
 */
#include <config.h>

#include <cstdio>
#include <cstring>

#include "cssc.h"
#include "sccsname.h"
#include "fileiter.h"
#include "linebuf.h"
#include "my-getopt.h"
#include "file.h"
#include "quit.h"
#include "dirent-safer.h"


sccs_file_iterator::sccs_file_iterator(const CSSC_Options &opts)
	: argv(opts.get_argv() + opts.get_index()),
	  argc(opts.get_argc() - opts.get_index()),
	  is_unique(0)
{

	if (argc < 1) {
		source = NONE;
		return;
	}

	char *first = argv[0];

	if (strcmp(first, "-") == 0) {
		source = STDIN;
		return;
	}

	if (first[0] != '\0') {
		DIR *dir = opendir(first);
		if (dir != NULL) {
			const char *slash = NULL;
			const size_t len = strlen(first);

			if (first[len - 1] != '/') {
			  slash = "/";
			} else {
			  /* SourceForge bug 121605: Unix coredump in
			   * mystring::mystring() if the directory
			   * name contains a trailing slash.  Solution
			   * is to make sure that "slash" is not NULL.
			   */
			  slash = ""; 
			}
			
			mystring dirname(mystring(first) + mystring(slash));

			struct dirent *dent = readdir(dir);
			while (dent != NULL) {
				mystring directory_entry = mystring(dirname) + mystring(dent->d_name);
				
				if (sccs_name::valid_filename(directory_entry.c_str())
				    && is_readable(directory_entry.c_str()))
				  {
				    if (is_directory(directory_entry.c_str()))
				      {
					warning("Ignoring subdirectory %s",
						directory_entry.c_str());
				      }
				    else
				      {
					files.add(directory_entry);
				      }
				  }
				dent = readdir(dir);
			}

			closedir(dir);

			source = DIRECTORY;
			pos = 0;
			return;
		}
	}

	source = ARGS;
	is_unique = (1 == argc);
}


int
sccs_file_iterator::unique() const
{
  return is_unique;
}


int
sccs_file_iterator::next() {
	switch (source) {
	case STDIN:
	  {
	    cssc_linebuf linebuf;

	    if (linebuf.read_line(stdin)) {
	      return 0;
	    }
	    mystring s (linebuf.c_str());
	    name = s.substr(0, s.length()-1u); // chop off the newline.
	    return 1;
	  }
	
	case DIRECTORY:
		if (pos < files.length()) {
			name = files[pos++];
			return 1;
		}
		return 0;

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
