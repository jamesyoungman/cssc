/*
 * sf-cdc.c: Part of GNU CSSC.
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
 * Members of the class sccs_file used for change the comments and
 * MRs of a delta. 
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"


#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-cdc.cc,v 1.7 1998/01/15 22:47:20 james Exp $";
#endif

/* Adds new MRs and comments to the specified delta. */

static bool
inlist(list<mystring> l, const mystring& find)
{
  int len = l.length();
  for (int i=0; i<len; ++i)
    {
      if (find == l[i])
	return true;
    }
  return false;
}

// Do the MR addition and deletion; if any have been deleted,
// return true.
static bool
process_mrs(list<mystring>& current,
	   list<mystring> to_add,
	   list<mystring> to_delete,
	   list<mystring>& comments)
{
  list<mystring> old_mrs = current;

  current = to_add;

  const int len = old_mrs.length();
  bool deleted = false;
  
  for (int i=0; i<len; ++i)
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
  return deleted;
}


void
sccs_file::cdc(sid id, list<mystring> mrs, list<mystring> comments)
{
  delta *p = find_delta(id);
  if (!p)
    quit(-1, "%s: Requested SID doesn't exist.", name.c_str());
  
  delta &d = *p;
  
  list<mystring> not_mrs;
  list<mystring> deletion_comment;
  int mrs_deleted = 0;
  int len = mrs.length();
  if (0 != len)
    {
      list<mystring> yes_mrs;

      for (int i = 0; i < len; i++)
	{
	  const char *s = mrs[i].c_str();
	  if (s[0] == '!')
	    not_mrs.add(s + 1);
	  else
	    yes_mrs.add(s);
	}

      if (process_mrs(d.mrs, yes_mrs, not_mrs, deletion_comment))
	mrs_deleted = 1;
    }
  
  if (mrs_deleted || comments.length())	// Prepend the new comments.
    {
      list<mystring> newcomments;
      
      // If there are comments to be added, add them.
      if (comments.length())
	newcomments += comments;

      // If we had deleted any MRs, indicate that.
      if (mrs_deleted)
	newcomments += deletion_comment;

      // If we had changed the revision commentary,
      // add a footer indicating when (if we only changed the
      // MRs, this doesn't happen)
      if (comments.length())
	{
	  mystring changeline
	    = mystring("*** CHANGED *** ")
	    + sccs_date::now().as_string()
	    + mystring(" ")
	    + mystring(get_user_name());

	  newcomments.add(changeline);
	}
      
      newcomments += d.comments;
      d.comments = newcomments;
    }
}

/* Local variables: */
/* mode: c++ */
/* End: */
