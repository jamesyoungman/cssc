/*
 * sf_chkmr.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the check_mrs member function of the class sccs_file.
 *
 * @(#) MySC sf-chkmr.h 1.1 93/11/09 17:17:51
 *
 */

#ifndef __SF_CHKMR_H__
#define __SF_CHKMR_H__

#include "run.h"

/* This function is defined here instead of in sccsfile.h so
   that not every programme that includes sccsfile.h needs to
   have the run_mr_checker function defined. */

inline int 
sccs_file::check_mrs(list<string> mrs) {
	string gname = name.gfile();
	return run_mr_checker(flags.mr_checker, gname, mrs) != 0;
}

#endif /* __SF_CHKMR_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
