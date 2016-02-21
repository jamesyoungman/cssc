/*
 * sf-cdc.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,1998,1999,2007 Free Software Foundation, Inc.
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
 * Members of the class sccs_file used for change the comments and
 * MRs of a delta.
 *
 */

#include <config.h>
#include <string>
#include <unordered_set>
#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"
#include "file.h"


/* Adds new MRs and comments to the specified delta. */

static bool
inlist(mylist<std::string> l, const std::string& find)
{
  for (const auto& mr : l)
    {
      if (mr == find)
	return true;
    }
  return false;
}

// Do the MR addition and deletion; if any have been deleted,
// set deleted to true.   Return the updated set of MRs.
static std::vector<std::string>
process_mrs(const std::vector<std::string>& old_mrs,
	    const std::vector<std::string>& to_add,
	    const std::unordered_set<std::string>& to_delete,
	    mylist<std::string>& comments,
	    bool& deleted)
{
  std::vector<std::string> current(to_add);
  deleted = false;
  for (const auto& mr : old_mrs)
    {
      if (to_delete.find(mr) != to_delete.end())
	{
	  if (!deleted)
	    comments.push_back(std::string("*** LIST OF DELETED MRS ***"));
	  deleted = true;
	  comments.push_back(mr);
	}
      else
	{
	  current.push_back(mr);
	}
    }
  return current;
}


bool
sccs_file::cdc(sid id, const std::vector<std::string>& mr_updates,
	       const mylist<std::string>& comment_updates)
{
  if (!edit_mode_ok(true))
    return false;

  delta *p = find_delta(id);
  if (!p)
    {
      errormsg("%s: Requested SID doesn't exist.", name.c_str());
      return false;
    }


  delta &d = *p;

  mylist<std::string> deletion_comment;
  bool mrs_deleted = false;
  if (!mr_updates.empty())
    {
      std::vector<std::string> yes_mrs;
      std::unordered_set<std::string> not_mrs;

      for (const auto& mr_spec : mr_updates)
	{
	  if (mr_spec[0] == '!')
	    not_mrs.insert(std::string(mr_spec, 1));
	  else
	    yes_mrs.push_back(mr_spec);
	}

      d.set_mrs(process_mrs(d.mrs(), yes_mrs, not_mrs, deletion_comment,
			    mrs_deleted));
    }

  if (mrs_deleted || !comment_updates.empty())	// Prepend the new comments.
    {
      mylist<std::string> newcomments;

      // If there are comments to be added, add them.
      if (!comment_updates.empty())
	newcomments += comment_updates;

      // If we had deleted any MRs, indicate that.
      if (mrs_deleted)
	newcomments += deletion_comment;

      // If we had changed the revision commentary,
      // add a footer indicating when (if we only changed the
      // MRs, this doesn't happen)
      if (!comment_updates.empty())
	{
	  std::string changeline
	    = std::string("*** CHANGED *** ")
	    + sccs_date::now().as_string()
	    + std::string(" ")
	    + std::string(get_user_name());

	  newcomments.push_back(changeline);
	}

      newcomments += d.comments();
      d.set_comments(newcomments);
    }

  return true;
}

/* Local variables: */
/* mode: c++ */
/* End: */
