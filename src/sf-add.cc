/*
 * sf-add.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2001, 2007, 2008, 2009, 2010, 2011, 2014, 2019
 *  Free Software Foundation, Inc.
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
 * Members of the class sccs_file used adding a new delta to the SCCS
 * file.
 *
 */

#include <config.h>
#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"
#include "delta-table.h"
#include "ioerr.h"

/* Starts an update of an SCCS file that includes a new entry to be
   prepended to delta table. */

FILE *
sccs_file::start_update(const delta &new_delta)
{
  cssc::FailureOr<FILE*> fof = start_update();
  if (!fof.ok())
    return NULL;
  FILE *out = *fof;
  cssc::Failure written = write_delta(out, new_delta);
  if (!written.ok() || write(out))
    {
      xfile_error("Write error.");
      fclose(out);
      return NULL;
    }
  return out;
}


/* Set the line counts in the prepended delta and then end the update. */

cssc::Failure
sccs_file::end_update(FILE **pout, const delta &d)
{
  if (fflush_failed(fflush(*pout)))
    {
      const int saved_errno = errno;
      fclose(*pout);
      *pout = NULL;
      return cssc::make_failure_builder_from_errno(saved_errno)
	.diagnose() << "failed to flush " << name.xfile();
    }

  rewind(*pout);

  if (printf_failed(fprintf(*pout, "\001h-----\n\001s %05lu/%05lu/%05lu",
			    cap5(d.inserted()),
			    cap5(d.deleted()),
			    cap5(d.unchanged()))))
    {
      fclose(*pout);
      *pout = NULL;
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << "failed to write to " << name.xfile();
    }

  cssc::Failure updated = end_update(pout);
  delta_table->prepend(d);
  return updated;
}

/* Local variables: */
/* mode: c++ */
/* End: */
