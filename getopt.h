/*
 * getopt.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class getopt.
 *
 * @(#) CSSC getopt.h 1.1 93/11/09 17:17:47
 *
 */

#ifndef __GETOPT_H__
#define __GETOPT_H__

#ifdef __GNUC__
#pragma interface
#endif

class getopt {
public:
	enum { 
		END_OF_ARGUMENTS = 0,
		UNRECOGNIZED_OPTION = -1,
		MISSING_ARGUMENT = -2
	};

	int argc;
	char **argv;

	int index;
	char *cindex;
	const char *opts;
	int opterr;
	char *arg;

public:
	getopt(int ac, char **av, const char *s, int err = 1)
		: argc(ac), argv(av), index(1),
		  cindex(NULL), opts(s), opterr(err) {}
	int next();

	int get_index() { return index; }
	char *getarg() { return arg; }
};

#endif /* __GETOPT_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
