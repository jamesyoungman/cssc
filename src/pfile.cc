/*
 * pfile.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2007, 2008, 2009, 2010, 2011,
 *  2014, 2019 Free Software Foundation, Inc.
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
 * Common members of the class sccs_pfile.
 *
 */
#include "config.h"

#include <errno.h>

#include "cssc.h"
#include "linebuf.h"
#include "pfile.h"

NORETURN
sccs_pfile::corrupt(int lineno, const char *msg) const {
        p_corrupt_quit("%s: line %d: p-file is corrupt.  (%s)",
                       pname.c_str(), lineno, msg);
}

sccs_pfile::sccs_pfile(sccs_name &n, pfile_mode m)
  : name(n),
    pname(name.pfile()),
    mode(m),
    edit_locks()
{

        if (!name.valid()) {
                ctor_fail(-1, "%s: Not a SCCS file.", name.c_str());
        }

        if (mode != pfile_mode::PFILE_READ)
	  {
	    if (!name.lock().ok())
	      {
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
                while (linebuf.read_line(pf).ok()) {
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

                        if (!tmp.got.valid()) {
                                corrupt(lineno, "Invalid gotten SID");
                        }

                        if (!tmp.delta.valid()) {
                                corrupt(lineno, "Invalid delta SID");
                        }

                        if (!tmp.date.valid()) {
                                corrupt(lineno, "Invalid date/time");
                        }

                        if (!tmp.include.valid() || !tmp.exclude.valid()) {
                                corrupt(lineno, "Invalid SID list");
                        }
                        edit_locks.push_back(tmp);
                }

                if (ferror(pf))
                  {
                    errormsg_with_errno("%s: Read error.", pname.c_str());
                    ctor_fail_nomsg(-1);
                  }

                fclose(pf);
        }
}

sccs_pfile::const_iterator
sccs_pfile::find_locked(const sid& id) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if (it->got == id)
	return it;
    }
  return end();
}

sccs_pfile::const_iterator
sccs_pfile::find_to_be_created(const sid& id) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if (it->delta == id)
	return it;
    }
  return end();
}

cssc::Failure
sccs_pfile::print_lock_sid(FILE *fp, const_iterator pos) const
{
  return pos->delta.print(fp);
}


sccs_pfile::~sccs_pfile() {
  if (mode != pfile_mode::PFILE_READ) {
                name.unlock();
        }
}


/* Local variables: */
/* mode: c++ */
/* End: */
