/*
 * sf-admin.cc: Part of GNU CSSC.
 * 
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
 * Members of the class sccs_file for performing creation and
 * adminstration operations on the SCCS file.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "sl-merge.h"
#include "delta.h"
#include "linebuf.h"
#include "bodyio.h"


#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-admin.cc,v 1.21 1998/06/15 20:50:01 james Exp $";
#endif

/* Changes the file comment, flags, and/or the user authorization list
   of the SCCS file. */

bool
sccs_file::admin(const char *file_comment,
		 bool force_binary,
		 list<mystring> set_flags, list<mystring> unset_flags,
		 list<mystring> add_users, list<mystring> erase_users)
{
	
  if (force_binary)
    flags.encoded = 1;

  if (file_comment != NULL)
    {
      comments = NULL;
      if (file_comment[0] != '\0')
	{
	  FILE *fc = fopen(file_comment, "r");
	  if (NULL == fc)
	    {
	      errormsg_with_errno("%s: Can't open comment file", file_comment);
	      return false;
	    }

	  while (!read_line_param(fc))
	    {
	      comments.add(plinebuf->c_str());
	    }

	  if (ferror(fc))
	    {
	      errormsg_with_errno("%s: Read error", file_comment);
	      fclose(fc);
	      return false;
	    }
	  else
	    {
	      fclose(fc);
	    }
	}
    }

  int i;

  int len;
  len = set_flags.length();
  for(i = 0; i < len; i++)
    {
      const char *s = set_flags[i].c_str();
      
      switch (*s++)
	{
	case 'b':
	  flags.branch = 1;
	  break;
	  
	case 'c':
	  flags.ceiling = release(s);
	  if (!flags.ceiling.valid())
	    {
	      errormsg("Invalid release ceiling: '%s'", s);
	      return false;
	    }
	  break;

	case 'f':
	  flags.floor = release(s);
	  if (!flags.floor.valid())
	    {
	      errormsg("Invalid release floor: '%s'", s);
	      return false;
	    }
	  break;


	case 'd':
	  flags.default_sid = sid(s);
	  if (!flags.default_sid.valid())
	    {
	      errormsg("Invalid default SID: '%s'", s);
	      return false;
	    }
	  break;

	case 'i':
	  if (strlen(s))
	    {
	      errormsg("Flag 'i' does not take an argument.");
	      return false;
	    }
	  else
	    {
	      flags.no_id_keywords_is_fatal = 1;
	    }
	  break;


	case 'j':
	  flags.joint_edit = 1;
	  break;

	case 'l':
	  if (strcmp(s, "a") == 0)
	    {
	      flags.all_locked = 1;
	      flags.locked = NULL;
	    }
	  else
	    {
	      flags.locked.merge(release_list(s));
	    }
	  break;

	case 'm':
	  set_module_flag(s);
	  break;
	  
	case 'n':
	  flags.null_deltas = 1;
	  break;


	case 'q':
	  set_user_flag(s);
	  break;
	  
	case 'e':
	  errormsg("The encoding flag must be set with the -b option");
	  return false;


	case 't':
	  set_type_flag(s);
	  break;

	case 'v':
	  set_mr_checker_flag(s);
	  break;
	  
	default:
	  // TODO: this will fail for every file, so should probably
	  // be a "hard" error.
	  errormsg("Unrecognized flag '%c'", s[-1]);
	  return false;
	}
    }
	      
	
  len = unset_flags.length();
  for(i = 0; i < len; i++)
    {
      const char *s = unset_flags[i].c_str();

      switch (*s++)
	{
	case 'b':
	  flags.branch = 0;
	  break;
	  
	case 'c':
	  flags.ceiling = NULL;
	  break;
	  
	case 'f':
	  flags.floor = NULL;
	  break;
	  
	  
	case 'd':
	  flags.default_sid = NULL;
	  break;
	  
	case 'i':
	  flags.no_id_keywords_is_fatal = 0;
	  break;
	  
	case 'j':
	  flags.joint_edit = 0;
	  break;
	  
	case 'l':
	  if (strcmp(s, "a") == 0)
	    {
	      flags.all_locked = 0;
	      flags.locked = NULL;
	    }
	  else
	    {
	      flags.locked.remove(release_list(s));
	    }
	  break;
	  
	case 'n':
	  flags.null_deltas = 0;
	  break;
	  
	  
	case 'q':
	  delete flags.user_def;
	  flags.user_def = 0;
	  break;
	  
	case 'e':
	  errormsg("Deletion of the binary-encoding flag is not supported.");
	  return false;
			
	case 't':
	  delete flags.type;
	  flags.type = 0;
	  break;
	  
	case 'v':
	  delete flags.mr_checker;
	  flags.mr_checker = 0;
	  break;
	  
	default:
	  // TODO: this will fail for every file, so should probably
	  // be a "hard" error.
	  errormsg("Unrecognized flag '%c'", s[-1]);
	  return false;
	}
    }

  // Erase any required users from the list.
  users -= erase_users;
	
  // Add the specified users to the beginning of the user list.
  list<mystring> newusers = add_users;
  newusers += users;
  users = newusers;

  return true;
}


/* Creates a new SCCS file. */

bool
sccs_file::create(release first_release, const char *iname,
		  list<mystring> mrs, list<mystring> comments,
		  int suppress_comments, bool force_binary)
{

  sccs_date now = sccs_date::now();
  if (!suppress_comments && comments.length() == 0)
    {
      comments.add(mystring("date and time created ")
		   + now.as_string()
		   + mystring(" by ")
		   + get_user_name());
    }

  sid id = sid(first_release).successor();

  delta new_delta('D', id, now, get_user_name(), 1, 0,
		  mrs, comments);
  new_delta.inserted = 0;
  new_delta.deleted = 0;
  new_delta.unchanged = 0;

  FILE *out = start_update(new_delta);
  if (NULL == out)
    return false;
  
  if (fprintf_failed(fprintf(out, "\001I 1\n")))
    return false;

  if (iname != NULL)
    {
    FILE *in;

    if (strcmp(iname, "-") == 0)
      {
	in = stdin;
      }
    else
      {
      in = fopen(iname, "r");
      if (NULL == in)
	{
	  errormsg_with_errno("%s: Can't open file for reading", iname);
	  // TODO: delete output file?
	  fclose(out);
	  return false;
	}
      }

    bool found_id = false;
    unsigned long int lines = 0uL;

    // Insert the body...
    body_insert(&force_binary,
		iname,		// input file name
		name.xfile().c_str(), // output file name
		in, out,
		&lines, &found_id);

    new_delta.inserted = lines;
    
    if (force_binary)
      flags.encoded = true;	// fixup file in sccs_file::end_update()
    
    if (in != stdin)
      fclose(in);

    // TODO: what if no id keywords is fatal?  Delete current s-file?
    // If so, do we continue with the next?
    if (!found_id)
      no_id_keywords(name.c_str());
  }
	
  if (fprintf_failed(fprintf(out, "\001E 1\n")))
    return false;

  // if the "encoded" flag needs to be changed,
  // end_update() will change it.
  end_update(out, new_delta);

  return true;
}

/* Local variables: */
/* mode: c++ */
/* End: */
