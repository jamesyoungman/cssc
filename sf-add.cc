/*
 * sf-add.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
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
 * Members of the class sccs_file used adding a new delta to the SCCS
 * file.
 * 
 */

#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"
#include "delta-table.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-add.cc,v 1.7 1997/11/30 21:05:51 james Exp $";
#endif

/* Starts an update of an SCCS file that includes a new entry to be
   prepended to delta table. */

FILE *
sccs_file::start_update(const delta &new_delta) {
	FILE *out = start_update();

	if (write_delta(out, new_delta) || write(out)) {
		xfile_error("Write error.");
	}
	return out;
}


/* Set the line counts in the prepended delta and then end the update. */

void
sccs_file::end_update(FILE *out, const delta &d)
{
  if (fflush_failed(fflush(out)))
    {
      xfile_error("Write error.");
    }

  rewind(out);

  if (printf_failed(fprintf(out, "\001h-----\n\001s %05lu/%05lu/%05lu",
			    cap5(d.inserted),
			    cap5(d.deleted),
			    cap5(d.unchanged))))
    {
      xfile_error("Write error.");
    }
  
  end_update(out);
  
  delta_table->prepend(d);
}

/* Local variables: */
/* mode: c++ */
/* End: */
