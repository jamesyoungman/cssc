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

#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"
#include "file.h"


/* Adds new MRs and comments to the specified delta. */

static bool
inlist(mylist<mystring> l, const mystring& find)
{
  const mylist<mystring>::size_type len = l.length();
  for (mylist<mystring>::size_type i=0; i<len; ++i)
    {
      if (find == l[i])
	return true;
    }
  return false;
}

// Do the MR addition and deletion; if any have been deleted,
// set deleted to true.   Return the updated set of MRs.
static mylist<mystring>
process_mrs(const mylist<mystring>& old_mrs,
	    mylist<mystring> to_add,
	    mylist<mystring> to_delete,
	    mylist<mystring>& comments,
	    bool& deleted)
{
  mylist<mystring> current(to_add);

  const mylist<mystring>::size_type len = old_mrs.length();
  deleted = false;
  
  for (mylist<mystring>::size_type i=0; i<len; ++i)
    {
      mystring const& mr(old_mrs[i]);
      
      if (inlist(to_delete, mr))
	{
	  if (!deleted)
	    comments.add(mystring("*** LIST OF DELETED MRS ***"));
	  deleted = true;
	  comments.add(mr);
	}
      else
	{
	  current.add(mr);
	}
    }
  return current;
}


bool
sccs_file::cdc(sid id, mylist<mystring> mr_updates, mylist<mystring> comment_updates)
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
  
  mylist<mystring> not_mrs;
  mylist<mystring> deletion_comment;
  bool mrs_deleted = false;
  const mylist<mystring>::size_type len = mr_updates.length();
  if (0 != len)
    {
      mylist<mystring> yes_mrs;

      for (mylist<mystring>::size_type i = 0; i < len; i++)
	{
	  const char *s = mr_updates[i].c_str();
	  if (s[0] == '!')
	    not_mrs.add(s + 1);
	  else
	    yes_mrs.add(s);
	}

      mylist<mystring> new_mrs;
      new_mrs = process_mrs(d.mrs(), yes_mrs, not_mrs, deletion_comment, mrs_deleted);
      d.set_mrs(new_mrs);
    }
  
  if (mrs_deleted || comment_updates.length())	// Prepend the new comments.
    {
      mylist<mystring> newcomments;
      
      // If there are comments to be added, add them.
      if (comment_updates.length())
	newcomments += comment_updates;

      // If we had deleted any MRs, indicate that.
      if (mrs_deleted)
	newcomments += deletion_comment;

      // If we had changed the revision commentary,
      // add a footer indicating when (if we only changed the
      // MRs, this doesn't happen)
      if (comment_updates.length())
	{
	  mystring changeline
	    = mystring("*** CHANGED *** ")
	    + sccs_date::now().as_string()
	    + mystring(" ")
	    + mystring(get_user_name());

	  newcomments.add(changeline);
	}
      
      newcomments += d.comments();
      d.set_comments(newcomments);
    }

  return true;
}

/* Local variables: */
/* mode: c++ */
/* End: */
