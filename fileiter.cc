/*
 * fileiter.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class fileiter.
 *
 */

#ifdef __GNUC__
#pragma implementation "fileiter.h"
#endif

#include "mysc.h"
#include "sccsname.h"
#include "fileiter.h"
#include "linebuf.h"

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC fileiter.c 1.1 93/11/09 17:17:54";
#endif

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
			mystring dirname(first, slash);

			struct dirent *dent = readdir(dir);
			while(dent != NULL) {
				mystring name = mystring(dirname) + mystring(dent->d_name, NAMLEN(dent));
				
				if (sccs_name::valid_filename(name)
				    && is_readable(name)) {
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
}

int
sccs_file_iterator::next() {
	switch(source) {
	case STDIN:
	  {
	    static class _linebuf linebuf;

	    if (linebuf.read_line(stdin)) {
	      return 0;
	    }

	    name = (const char *) linebuf;
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
		if (!name.valid()) {
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
