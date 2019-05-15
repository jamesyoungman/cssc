/*
 * subst-parms.h: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2007,
 *  2008, 2009, 2010, 2011, 2014, 2019 Free Software Foundation, Inc.
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
 */
#ifndef CSSC__SUBST_PARMS_H__
#define CSSC__SUBST_PARMS_H__

#include <stdio.h>
#include <string>

#include "delta.h"
#include "sccsdate.h"


struct subst_parms
{
  std::string outname;
  std::string module_name;
  const char *wstring;
  FILE *out;
  struct delta const &delta;
  unsigned out_lineno;
  sccs_date now;
  int found_id;
  
subst_parms(const std::string& name, const std::string modname,
	    FILE *o, const char *w, struct delta const &d,
	    unsigned int l, sccs_date n)
: outname(name), module_name(modname), wstring(w), out(o),
    delta(d), out_lineno(l), now(n),
    found_id(0) {}

  const string& get_module_name() const
  {
    return module_name;
  }
};

#endif /* CSSC__SUBST_PARMS_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
