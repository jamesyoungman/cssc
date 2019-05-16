/*
 * pf-del.cc: Part of GNU CSSC.
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
 * Members of the class sccs_pfile used when removing an edit lock from
 * a p-file.
 *
 */
#include "config.h"

#include <string>
#include <utility>

#include "cssc.h"
#include "pfile.h"
#include "except.h"
#include "file.h"


std::pair<sccs_pfile::find_status, sccs_pfile::iterator>
sccs_pfile::find_sid(const sid& id)
{
  const char *username = get_user_name();
  iterator found = end();

  for (iterator it = begin(); it != end(); ++it)
    {
      if (strcmp(username, it->user.c_str()) == 0
	  && (id.is_null() || id == it->got || id == it->delta))
	{
	  if (found != end())
	    {
	      return make_pair(find_status::AMBIGUOUS, end());
	    }
	  found = it;
	}
    }

  if (found == end())
    {
      return make_pair(find_status::NOT_FOUND, end());
    }
  return make_pair(find_status::FOUND, found);
}

bool
sccs_pfile::update(bool pfile_already_exists) const
{
  const std::string q_name(name.qfile());
  const char* qname = q_name.c_str();

  FILE *pf = fcreate(qname, CREATE_EXCLUSIVE);
  if (pf == NULL)
    {
      errormsg_with_errno("%s: Can't create temporary file.", qname);
      return false;
    }

  int count = 0;

  try
    {
      for (const_iterator it = begin(); it != end(); ++it)
	{
	  if (write_edit_lock(pf, *it))
	    {
	      errormsg_with_errno("%s: Write error.", qname);
	      remove(qname);
	      return false;
	    }
	  count++;
	}

      if (fclose_failed(fclose(pf)))
	{
	  errormsg_with_errno("%s: Write error.", qname);
	  remove(qname);
	  return false;
	}
    }
  catch (CsscException)
    {
      // got an exception; delete the temporary file and re-throw the exception
      remove(qname);
      throw;
    }

  if (pfile_already_exists)
    {
      if (remove(pname.c_str()) != 0)
	{
	  errormsg_with_errno("%s: Can't remove old p-file.", pname.c_str());
	  return false;
	}
    }

  if (count == 0)		// no locks left -> no pfile rewuired.
    {
      if (remove(qname) != 0)
	{
	  errormsg_with_errno("%s: Can't remove temporary file.",
			      pname.c_str());
	  return false;
	}
    }
  else
    {
      if (rename(qname, pname.c_str()) != 0)
	{
	  // this is really bad; we have already deleted the old p-file!
	  errormsg_with_errno("%s: Can't rename new p-file.",
	       qname);
	  return false;
	}
    }
  return true;
}

/* Local variables: */
/* mode: c++ */
/* End: */
