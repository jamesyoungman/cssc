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
	list<mystring> files;
	int pos;
#endif
	sccs_name name;

public:
	sccs_file_iterator(int ac, char **av, int ind = 1);
	
	int next();

	sccs_name &get_name() { return name; }
  // JAY mod: using is now a keyword; change the function name to 
  // using_source().
	enum sources using_source() { return source; }	
};

#endif /* __FILEITER_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
