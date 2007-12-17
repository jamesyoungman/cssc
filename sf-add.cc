/*
 * sf-add.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,2001,2007 Free Software Foundation, Inc. 
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
static const char rcs_id[] = "CSSC $Id: sf-add.cc,v 1.12 2007/12/17 21:59:51 jay Exp $";
#endif

/* Starts an update of an SCCS file that includes a new entry to be
   prepended to delta table. */

FILE *
sccs_file::start_update(const delta &new_delta)
{
  FILE *out = start_update();
  if (out)
    {
      if (write_delta(out, new_delta) || write(out))
	{
	  xfile_error("Write error.");
	}
    }
  return out;
}


/* Set the line counts in the prepended delta and then end the update. */

bool
sccs_file::end_update(FILE **pout, const delta &d)
{
  if (fflush_failed(fflush(*pout)))
    {
      fclose(*pout);
      *pout = NULL;
      xfile_error("Write error.");
    }

  rewind(*pout);

  if (printf_failed(fprintf(*pout, "\001h-----\n\001s %05lu/%05lu/%05lu",
			    cap5(d.inserted),
			    cap5(d.deleted),
			    cap5(d.unchanged))))
    {
      fclose(*pout);
      *pout = NULL;
      xfile_error("Write error.");
    }
  
  bool retval = end_update(pout);
  
  delta_table->prepend(d);
  return retval;
}

/* Local variables: */
/* mode: c++ */
/* End: */
