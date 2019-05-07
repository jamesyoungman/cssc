/*
 * sf_chkmr.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1999, 2007, 2008, 2009, 2010, 2011, 2014 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Defines the check_mrs member function of the class sccs_file.
 *
 * @(#) CSSC sf-chkmr.h 1.1 93/11/09 17:17:51
 *
 */

#ifndef CSSC__SF_CHKMR_H__
#define CSSC__SF_CHKMR_H__

#include "run.h"

/* This function is defined here instead of in sccsfile.h so
   that not every programme that includes sccsfile.h needs to
   have the run_mr_checker function defined. */

inline int
sccs_file::check_mrs(const std::vector<string>& mrs)
{
  ASSERT(0 != flags.mr_checker);
  return 0 != run_mr_checker(flags.mr_checker->c_str(),
			     name.gfile().c_str(), mrs);
}

#endif /* __SF_CHKMR_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
