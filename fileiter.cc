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

#include <cstring>

#ifdef __GNUC__
#pragma implementation "fileiter.h"
#endif

#include "cssc.h"
#include "sccsname.h"
#include "fileiter.h"
#include "linebuf.h"
#include "my-getopt.h"
#include "file.h"
#include "quit.h"


#define CONFIG_NO_DIRECTORY

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
# undef CONFIG_NO_DIRECTORY
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# undef CONFIG_NO_DIRECTORY
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# undef CONFIG_NO_DIRECTORY
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# undef CONFIG_NO_DIRECTORY
# endif
#endif



// This function is only defined in this module in order to 
// take advantage of the macros above.
// FIXME: when/if we start to use gnulib, move it to file.c
int 
is_directory(const char *name) 
{
  bool retval = false;
  DIR *p = opendir(name);
  if (p)
    {
      retval = true;
      closedir(p);
    }
  return retval;
}





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
			} else {
			  /* SourceForge bug 121605: Unix coredump in
			   * mystring::mystring() if the directory
			   * name contains a trailing slash.  Solution
			   * is to make sure that "slash" is not NULL.
			   */
			  slash = ""; 
			}
			
#endif
			
			mystring dirname(mystring(first) + mystring(slash));

			struct dirent *dent = readdir(dir);
			while (dent != NULL) {
				mystring name = mystring(dirname) + mystring(dent->d_name, NAMLEN(dent));
				
				if (sccs_name::valid_filename(name.c_str())
				    && is_readable(name.c_str()))
				  {
				    if (is_directory(name.c_str()))
				      {
					warning("Ignoring subdirectory %s",
						name.c_str());
				      }
				    else
				      {
					files.add(name);
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
#endif /* CONFIG_NO_DIRECTORY */

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

// Explicit template instantiations
template void mylist<mystring>::add(mystring const&);
					
/* Local variables: */
/* mode: c++ */
/* End: */
