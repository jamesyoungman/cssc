/*
 * fileiter.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class sccs_file_iter.
 *
 * @(#) MySC fileiter.h 1.1 93/11/09 17:17:46
 *
 */

#ifndef __FILEITER_H__
#define __FILEITER_H__

#include "list.h"
#include "sccsname.h"

#ifndef CONFIG_NO_DIRECTORY
#include <dirent.h>
#undef DIRECTORY
#endif

#ifdef __GNUC__
#pragma interface
#endif

/* This class is used to iterate over the list of SCCS files as
   specified on the command line. */

class sccs_file_iterator {
public:
	enum sources { ARGS, STDIN, DIRECTORY };

private:
	enum sources source;

	char **argv;
	int argc;

#ifndef CONFIG_NO_DIRECTORY
	list<string> files;
	int pos;
#endif
	sccs_name name;

public:
	sccs_file_iterator(int ac, char **av, int ind = 1);
	
	int next();

	sccs_name &get_name() { return name; }
	enum sources using() { return source; }	
};

#endif /* __FILEITER_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
