/*
 * pfile.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998 Free Software Foundation, Inc. 
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
 * Common members of the class sccs_pfile.
 *
 */

#include "cssc.h"
#include "linebuf.h"
#include "pfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: pfile.cc,v 1.12 1998/09/02 21:03:26 james Exp $";
#endif

NORETURN
sccs_pfile::corrupt(int lineno, const char *msg) const {
	p_corrupt_quit("%s: line %d: p-file is corrupt.  (%s)",
		       pname.c_str(), lineno, msg);
}

sccs_pfile::sccs_pfile(sccs_name &n, enum _mode m)
		: name(n), mode(m), pos(-1) {

        if (!name.valid()) {
		ctor_fail(-1, "%s: Not a SCCS file.", name.c_str());
	}

	pname = name.pfile();

	if (mode != READ) {
		if (name.lock()) {
			ctor_fail(-1,
				  "%s: SCCS file is locked.  Try again later.",
				  name.c_str());
		}
	}

	FILE *pf = fopen(pname.c_str(), "r");
	if (pf == NULL) {
		if (errno != ENOENT) {
			ctor_fail(-1, "%s: Can't open p-file for reading.",
			     pname.c_str());
		}
	} else {
		cssc_linebuf linebuf;

		int lineno = 0;
		while (!linebuf.read_line(pf)) {
		  // chomp the newline 
		  // TODO: make me 8-bit clean!
		  linebuf[strlen(linebuf.c_str()) - 1] = '\0';
			lineno++;

			char *args[7];

			int argc = linebuf.split(0, args, 7, ' ');

			if (argc < 5) {
				corrupt(lineno, "Expected 5-7 args");
			}

			char *incl = NULL, *excl = NULL;
			int i;
			for(i = 5; i < argc; i++) {
				if (args[i][0] == '-') {
					if (args[i][1] == 'i') {
						incl = args[i] + 2;
					} else if (args[i][1] == 'x') {
						excl = args[i] + 2;
					}
				}
			}

			if ((argc == 6 && excl == NULL && incl == NULL)
			    || (argc == 7 && (excl == NULL || incl == NULL))) {
				corrupt(lineno, "Bad -i or -x option");
			}
				      
				
			struct edit_lock tmp(args[0], args[1], args[2],
					     args[3], args[4], 
					     incl, excl);

			if (!tmp.got.valid() || !tmp.got.valid()) {
				corrupt(lineno, "Invalid SID");
			}

			if (!tmp.date.valid()) {
				corrupt(lineno, "Invalid date");
			}
					
			if (!tmp.include.valid() || !tmp.exclude.valid()) {
				corrupt(lineno, "Invalid SID list");
			}
			edit_locks.add(tmp);
		}

		if (ferror(pf))
		  {
		    errormsg_with_errno("%s: Read error.", pname.c_str());
		    ctor_fail(-1, NULL);
		  }

		fclose(pf);
	}
}

int
sccs_pfile::is_locked(sid id) {
	rewind();

	sccs_pfile &it = *this;
	while (next()) {
		if (it->got == id) {
			return 1;
		}
	}

	return 0;
}

int
sccs_pfile::is_to_be_created(sid id) {
	rewind();

	sccs_pfile &it = *this;
	while (next()) {
		if (it->delta == id) {
			return 1;
		}
	}

	return 0;
}

int 
sccs_pfile::print_lock_sid(FILE *fp)
{
  const edit_lock& l = edit_locks.select(pos);
  const sid& s       = l.delta;
  return s.print(fp);
}



sccs_pfile::~sccs_pfile() {
	if (mode != READ) {
		name.unlock();
	}
}


/* Local variables: */
/* mode: c++ */
/* End: */
