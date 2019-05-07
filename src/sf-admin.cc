/*
 * sf-admin.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2004, 2007, 2008, 2009, 2010, 2011, 2014 Free Software Foundation, Inc.
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
 * Members of the class sccs_file for performing creation and
 * adminstration operations on the SCCS file.
 *
 */

#include <config.h>
#include <string>
#include <vector>
#include "cssc.h"
#include "sccsfile.h"
#include "sl-merge.h"
#include "delta.h"
#include "linebuf.h"
#include "bodyio.h"
#include "file.h"


/* #define ADMIN_MERGE_LOCKED_RELEASES if you want
 * admin -fl1 s.foo ;  admin -fl2 s.foo
 * to result in both releases 1 and 2 being locked;
 * if you do not deifne this, the -fl option will
 * implicitly clear any previous list of locked releases.
 */
#undef ADMIN_MERGE_LOCKED_RELEASES



/* Changes the file comment, flags, and/or the user authorization list
   of the SCCS file. */

bool
sccs_file::admin(const char *file_comment,
		 bool force_binary,
		 const std::vector<std::string>& set_flags,
		 const std::vector<std::string>& unset_flags,
		 const std::vector<std::string>& add_users,
		 const std::unordered_set<std::string>& erase_users)
{
  if (force_binary)
    flags.encoded = 1;

  if (file_comment != NULL)
    {
      comments.clear();
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
	      comments.push_back(plinebuf->c_str());
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

  for (const auto& set_flag : set_flags)
    {
      const char flag_letter = set_flag[0];
      const char* flag_value = set_flag.c_str() + 1;

      switch (flag_letter)
	{
	case 'b':
	  flags.branch = 1;
	  break;

	case 'c':
	  flags.ceiling = release(flag_value);
	  if (!flags.ceiling.valid())
	    {
	      errormsg("Invalid release ceiling: '%s'", flag_value);
	      return false;
	    }
	  break;

	case 'f':
	  flags.floor = release(flag_value);
	  if (!flags.floor.valid())
	    {
	      errormsg("Invalid release floor: '%s'", flag_value);
	      return false;
	    }
	  break;


	case 'd':
	  flags.default_sid = sid(flag_value);
	  if (!flags.default_sid.valid())
	    {
	      errormsg("Invalid default SID: '%s'", flag_value);
	      return false;
	    }
	  break;

	case 'i':
	  if (strlen(flag_value) != 0)
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
	  if (strcmp(flag_value, "a") == 0)
	    {
	      flags.all_locked = 1;
	      flags.locked = release_list(); // empty list
	    }
	  else
	    {
#ifdef ADMIN_MERGE_LOCKED_RELEASES
	      flags.locked.merge(release_list(flag_value));
#else
	      /* "admin -fl" clears any previously locked releases.
	       */
	      flags.locked = release_list(); // empty list
	      flags.locked.merge(release_list(flag_value));
#endif
	    }
	  break;

	case 'm':
	  set_module_flag(flag_value);
	  break;

	case 'n':
	  flags.null_deltas = 1;
	  break;


	case 'q':
	  set_user_flag(flag_value);
	  break;

	case 'e':
	  errormsg("The encoding flag must be set with the -b option");
	  return false;


	case 't':
	  set_type_flag(flag_value);
	  break;

	case 'v':
	  set_mr_checker_flag(flag_value);
	  break;

	case 'x':
	  warning("The 'x' (executable) flag is a SCO extension and is not supported by other versions of SCCS.");
	  flags.executable = 1;
	  break;

	case 'y':
	  // Argument is a comma-separated list of keyword letters to expand.
	  warning("The 'y' (expanded keywords) flag is a Sun extension present only in Solaris 8 and later, and is not supported by other versions of SCCS.");
	  set_expanded_keyword_flag(""); // delete any existing ones.
	  for (const char c : string(flag_value))
	    {
	      if (',' != c)
		{
		  if (isalpha((unsigned char)c))
		    {
		      if (!is_known_keyword_char(c))
			{
			  warning("'%%%c%%' is not a recognised SCCS keyword, "
				  "but remembering that we want to expand it "
				  "anyway, for the future.", c);
			}

		      flags.substitued_flag_letters.insert(c);
		    }
		  else
		    {
		      errormsg("Unexpected character '%c' in argument to option '-fy'.", c);
		      return false;
		    }
		}
	    }
	  break;

	default:
	  // TODO: this will fail for every file, so should probably
	  // be a "hard" error.
	  errormsg("Unrecognized flag '%c'", flag_letter);
	  return false;
	}
    }

  for (const auto& unset_flag : unset_flags)
    {
      const char flag_letter = unset_flag[0];
      const string flag_value = string(unset_flag, 1);
      // TODO: diagnose !flag_value.empty() for cases other than flag_letter=='l'
      switch (flag_letter)
	{
	case 'b':
	  flags.branch = 0;
	  break;

	case 'c':
	  flags.ceiling = static_cast<short>(0);
	  break;

	case 'f':
	  flags.floor = static_cast<short>(0);
	  break;


	case 'd':
	  flags.default_sid = sid::null_sid();
	  ASSERT(!flags.default_sid.valid());
	  break;

	case 'i':
	  flags.no_id_keywords_is_fatal = 0;
	  break;

	case 'j':
	  flags.joint_edit = 0;
	  break;

	case 'l':
	  if (flag_value == "a")
	    {
	      flags.all_locked = 0;
	      flags.locked = release_list();
	    }
	  else
	    {
	      if (flags.all_locked)
		{
		  errormsg("Unlocking just release %s of %s is not possible, "
			   "since all releases are locked.  "
			   "Use admin -dla to unlock all releases.",
			   flag_value.c_str(), name.c_str());
		  return false;
		}
	      else
		{
		  // TODO: consider adding release_list::release_list(const string&)
		  flags.locked.remove(release_list(flag_value.c_str()));
		}
	    }
	  break;

	case 'm':
	  delete flags.module;
	  flags.module = 0;
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

	case 'x':
	  flags.executable = 0;
	  break;

	case 'y':
	  // Set the expanded-keyword flag to the empty string, which
	  // means 'all' rather than none.
	  set_expanded_keyword_flag("");
	  break;

	default:
	  // TODO: this will fail for every file, so should probably
	  // be a "hard" error.
	  errormsg("Unrecognized flag '%c'", flag_letter);
	  return false;
	}
    }

  // Erase any required users from the list.
  std::vector<std::string> newusers = add_users;
  for (const auto& user : users)
    {
      if (erase_users.find(user) == erase_users.end())
	{
	  newusers.push_back(user);
	}
    }
  std::swap(users, newusers);

  return true;
}


/* Creates a new SCCS file. */

bool
sccs_file::create(const sid &id,
		  const char *iname,
		  const std::vector<std::string>& mrs,
		  std::vector<std::string>* starting_comments,
		  int suppress_comments, bool force_binary)
{

  sccs_date now = sccs_date::now();
  if (!suppress_comments && starting_comments->empty())
    {
      starting_comments->push_back(std::string("date and time created ")
				   + now.as_string()
				   + std::string(" by ")
				   + get_user_name());
    }


  delta new_delta('D', id, now, get_user_name(), 1, 0,
		  mrs, *starting_comments);
  ASSERT (new_delta.inserted() == 0);
  ASSERT (new_delta.deleted() == 0);
  ASSERT (new_delta.unchanged() == 0);

  bool ret = true;

  FILE *in = NULL;
  if (iname != NULL)
    {
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
	      return false;
	    }
	}

      bool file_is_executable = false;
      if (get_open_file_xbits(in, &file_is_executable))
	set_sfile_executable(file_is_executable);
    }

  FILE *out = start_update(new_delta);
  if (NULL == out)
    {
      fclose(in);
      return false;
    }

  if (fprintf_failed(fprintf(out, "\001I 1\n")))
    return false;


  if (NULL != in)
    {
      bool found_id = false;
      unsigned long int lines = 0uL;

      // Insert the body...
      if (body_insert(&force_binary,
		      iname,		// input file name
		      name.xfile().c_str(), // output file name
		      in, out,
		      &lines, &found_id))
	{
	  new_delta.set_inserted(lines);

	  if (force_binary)
	    flags.encoded = true;	// fixup file in sccs_file::end_update()
	}
      else
	{
	  ret = false;
	}

      if (in != stdin)
	fclose(in);

      // TODO: what if no id keywords is fatal?  Delete current s-file?
      // If so, do we continue with the next?
      if (!found_id)
	{
	  no_id_keywords(name.c_str()); // this function normally returns.
	}
    }

  if (fprintf_failed(fprintf(out, "\001E 1\n")))
    return false;

  // if the "encoded" flag needs to be changed,
  // end_update() will change it.
  if (!end_update(&out, new_delta))
    return false;

  return ret;
}

/* Local variables: */
/* mode: c++ */
/* End: */
