/*
 * get.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998,1999,2001,2002,2007,2008 Free Software Foundation, Inc. 
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
 * Extract a requested delta from a SCCS file.
 * 
 */

#include "cssc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "delta.h"
#include "pfile.h"
#include "my-getopt.h"
#include "version.h"
#include "except.h"
#include "err_no.h"
#include "file.h"

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

const char main_rcs_id[] = "$Id: get.cc,v 1.57 2008/01/06 19:42:23 jay Exp $";

/* Prints a list of included or excluded SIDs. */

static void
print_id_list(const char *s, mylist<sid> const &list)
{
  int i, len;
        
  len = list.length();
  if (len > 0)
    {
      printf("%s:\n", s);
      for(i = 0; i < len; i++)
        {
          list[i].print(stdout);
          putchar('\n');
        }
    }
}

void
usage() {
        fprintf(stderr,
"usage: %s [-begkmnpstLV] [-c date] [-r SID] [-i range] [-w string]\n"
"\t[-x range] [-G gfile] file ...\n",
                prg_name);
}


static void maybe_clear_archive_bit(const mystring &)
{
#ifdef CONFIG_USE_ARCHIVE_BIT
  clear_archive_bit(gname);
#endif
}

#define EXITVAL_INVALID_OPTION (1)


int
main(int argc, char **argv)
{
  Cleaner arbitrary_name;
  int retval = 0;
  int c;
  sid rid(sid::null_sid());
  sid org_rid(sid::null_sid());
  int for_edit = 0;                     /* -e */
  int branch = 0;                       /* -b */
  int suppress_keywords = 0;            /* -k */
  int use_stdout = 0;                   /* -p */
  int silent = 0;                       /* -s */
  int no_output = 0;                    /* -g */
  const char *wstring = NULL;           /* -w */
  sid_list include, exclude;            /* -i, -x */
  sccs_date cutoff_date;                /* -c */
  int show_sid = 0;                     /* -m */
  int show_module = 0;                  /* -n */
  int debug = 0;                        /* -D */
  mystring gname;                       /* -G */
  int got_gname = 0;                    /* -G */
  seq_no seq = 0;                       /* -a */
  int get_top_delta = 0;                /* -t */
  bool real_file;
  bool delta_summary = false;	        /* -L, -l */
  bool create_lfile = false;            /* -l */
  
  if (argc > 0)
      set_prg_name(argv[0]);
  else
    set_prg_name("get");

  ASSERT(!rid.valid());
  ASSERT(!org_rid.valid());

  class CSSC_Options opts(argc, argv, "r!c!i!x!ebkl!psmngtw!a!DVG!L",
                          EXITVAL_INVALID_OPTION);
  for(c = opts.next();
      c != CSSC_Options::END_OF_ARGUMENTS;
      c = opts.next())
    {
      switch (c)
        {
        default:
          errormsg("Unsupported option: '%c'", c);
          return EXITVAL_INVALID_OPTION;
          
        case 'r':
          org_rid = sid(opts.getarg());
          if (!org_rid.valid())
            {
              errormsg("Invalid SID: '%s'", opts.getarg());
              return EXITVAL_INVALID_OPTION;
            }
          break;
          
        case 'c':
          cutoff_date = sccs_date(opts.getarg());
          if (!cutoff_date.valid())
            {
              errormsg("Invalid cutoff date: '%s'",
                       opts.getarg());
              return EXITVAL_INVALID_OPTION;
            }
          break;
          
        case 'i':
                {
                    sid_list include_arg(opts.getarg());
                    if (!include_arg.valid())
                    {
                        errormsg("Invalid inclusion list: '%s'",
                                 opts.getarg());
                        return EXITVAL_INVALID_OPTION;
                    }
                    include = include_arg;
                }
          break;
          
	case 'x':
                {
                    sid_list exclude_arg(opts.getarg());
                    if (!exclude_arg.valid())
                    {
                        errormsg("Invalid exclusion list: '%s'",
                                 opts.getarg());
                        return EXITVAL_INVALID_OPTION;
                    }
                    exclude = exclude_arg;
                }
                
          break;
          
        case 'e':
          for_edit = 1;
          suppress_keywords = 1;
          break;

        case 'b':
          branch = 1;
          break;

        case 'k':
          suppress_keywords = 1;
          break;

        case 'p':
          use_stdout = 1;
          got_gname = 0;
          break;

	case 'l':
	  delta_summary = true;
	  if (opts.getarg() && opts.getarg()[0])
	    {
	      if (0 == strcmp(opts.getarg(), "p"))
		{
		  /* -lp is a traditional synonym for -L */
		  create_lfile = false;
		}
	      else
		{
		  errormsg ("Unsupported -l option: '-l%s'", opts.getarg());
		  return EXITVAL_INVALID_OPTION;
		}
	    }
	  else
	    {
	      create_lfile = true;
	    }
	  break;

	case 'L':
	  delta_summary = true;
	  create_lfile = false;
	  break;
	  
        case 's':
          silent = 1;
          break;

        case 'm':
          show_sid = 1;
          break;

        case 'n':
          show_module = 1;
          break;

        case 'g':
          no_output = 1;
          break;

        case 'w':
          wstring = opts.getarg();
          break;

        case 'a':
          {
            int i;
            i = atoi(opts.getarg());
            if (i < 1 || i > USHRT_MAX)
              {
                errormsg("Invalid sequence number: '%s'", opts.getarg());
                return 2;
              }
            seq = (unsigned short) i;
          }
          break;
          
        case 't':
          get_top_delta = 1;
          break;

        case 'G':
          got_gname = 1;
          use_stdout = 0;
          gname = opts.getarg();
          break;
                        
        case 'D':
          debug = 1;
          break;
          
        case 'V':
          version();
          break;
        }
    }

  if (branch && !for_edit)
    {
      warning("there is not a lot of point in using the "
              "-b option unless you want to check the file out for "
              "editing (using the -e option).\n");
    }
        

  FILE *out = NULL;     /* The output file.  It's initialized
                           with NULL so if it's accidentally
                           used before being set it will
                           quickly cause an error. */
  
  if (use_stdout)
    {
      gname = "-";
      out = stdout_to_stderr();
      if (NULL == out)
        return 1;       // fatal error.
    }
  
  if (silent)
    {
      if (!stdout_to_null())
        return 1;       // fatal error.
    }
  
  if (no_output)
    {
      if (use_stdout)
        {
          fclose(out);
        }
      got_gname = 0;
      gname = "null";
      if (NULL == (out = open_null()))
        return 1;
    }

  sccs_file_iterator iter(opts);
  if (sccs_file_iterator::NONE == iter.using_source())
    {
      errormsg("No SCCS file specified");
      return 1;
    }
  
  while (iter.next())
    {
#ifdef HAVE_EXCEPTIONS
      try
        {
#endif        
          sccs_name &name = iter.get_name();

          // Print the name of the SCCS file unless exactly one
          // was specified.
          if (!iter.unique())
            {
              fprintf(stdout, "\n%s:\n", name.c_str());
            }
          
          
          sccs_pfile *pfile = NULL;
          if (for_edit)
            {
              pfile = new sccs_pfile(name, sccs_pfile::APPEND);
            }
          
          sccs_file file(name, sccs_file::READ);
          sid new_delta;
          sid retrieve;

          if (seq)
            {
              if (org_rid.valid())
                {
                  warning("both the the -r and the -a "
                          "option have been specified; "
                          "the -r option has been ignored.");
                }
              
              if (!file.find_requested_seqno(seq, retrieve))
                {
                  errormsg("%s: Requested sequence number %d not found.",
                           name.c_str(), (int) seq);
                  retval = 1;
                  continue; // with next file....
                }
            }
          else
            {
              rid = org_rid;
              if (!file.find_requested_sid(rid, retrieve, get_top_delta))
                {
                  errormsg("%s: Requested SID not found.", name.c_str());
                  retval = 1;
                  continue; // with next file....
                }
              if (!rid.valid() ||
                  (rid.release_only() && release(rid) == release(retrieve)))
                {
                  rid = retrieve;
                }
            }
          
          
          
          if (for_edit)
            {
              if (branch && !file.branches_allowed())
                {
                  warning("%s: Branch-enable flag not set, "
                          "option -b ignored.\n",
                          name.c_str());
                }
              
              if ( (NULL==pfile) || !file.test_locks(retrieve, *pfile))
                {
                  retval = 1;
                  continue; // continue with next file...
                }

              int failed = 0;
              new_delta = file.find_next_sid(rid, retrieve,
                                             branch, *pfile,
                                             &failed);
              if (failed)
                {
                  /* sccs_file::find_next_sid() has returned NULL.
                   * This is a rare case.
                   */
                  retval = 1;
                  continue; // continue with next file...
                }
            }

          real_file = false;
          
          if (!use_stdout && !no_output)
            {
              ASSERT(name.valid());
              
              /* got_gname is specified if we had -G g-file
               * on the command line.   This only works for the
               * first file on the command line (or else we'd
               * be overwriting earlier data).
               */
              if (!got_gname)
                gname = name.gfile();
              got_gname = 0;
              
              int mode = CREATE_AS_REAL_USER | CREATE_FOR_GET;
              if (!suppress_keywords)
                {
                  mode |= CREATE_READ_ONLY;
                }

	      if (file.executable_flag_set())
		{
		  mode |= CREATE_EXECUTABLE;
		}
	      
              out = fcreate(gname, mode);
              real_file = true;
              
              if (NULL == out)
                {
                  if (errno)
                    perror(gname.c_str());
                  retval = 1;
                  continue;     // with next file....
                }
            }

	  FILE *summary_file = NULL;
	  if (delta_summary)
	    {
	      if (create_lfile)
		{
		  mystring lname = name.lfile();
		  summary_file = fcreate(lname, CREATE_READ_ONLY);
		}
	      else
		{
		  summary_file = stdout;
		}
	    }

          const int keywords = !suppress_keywords;
          struct sccs_file::get_status status;
          
#ifdef HAVE_EXCEPTIONS
          try
            {
#endif    
          status = file.get(out, gname, summary_file, retrieve, cutoff_date,
                            include, exclude, keywords, wstring,
                            show_sid, show_module, debug, for_edit);
#ifdef HAVE_EXCEPTIONS
            }
          catch (CsscException)
            {
              // the get failed.  Delete the g-file and re-throw the exception.
              if (real_file)
                {
                  fclose(out);
                  remove(gname.c_str());
                  throw;
                }
            }
#endif
          
          if (real_file)
            {
              fclose(out);
              if (suppress_keywords)
              {
                  if (!set_gfile_writable(gname, true,
					  file.executable_flag_set()))
                      retval = 1;
		  maybe_clear_archive_bit(gname);
              }
              else
              {
                /* The g-file was created with the real uid,
                 * and so if we want to change its mode, we 
                 * will have to temporarily set EUID=RUID.
                 */
                give_up_privileges();
                if (!set_gfile_writable(gname, false,
					file.executable_flag_set()))
                    status.success = false;
                restore_privileges();
              }
            }

          // We delete this file if the "get" failed.
          // However, this is not conditional on "retval" since 
          // if the previous attempt failed, we would like the others
          // to succeed.

          if (!status.success) // get failed.
            {
              retval = 1;
	      if (!unlink_file_as_real_user(gname.c_str()))
		{
		  errormsg_with_errno("Failed to remove file %s",
				      gname.c_str());
		}
              continue;
            }
          
          print_id_list("Included", status.included);
          print_id_list("Excluded", status.excluded);
          retrieve.print(stdout);
          putchar('\n');
          
          if (for_edit)
            {
              printf("new delta ");
              new_delta.print(stdout);
              putchar('\n');
              
              if (!pfile->add_lock(retrieve, new_delta, include, exclude))
                {
                  // Failed to add the lock to the p-file.
                  if (real_file)
                    {
		      if (!unlink_file_as_real_user(gname.c_str()))
			{
			  errormsg_with_errno("Failed to remove file %s",
					      gname.c_str());
			}
                    }
                  retval = 1;   // remember the failure.
                }
              delete pfile;
              pfile = NULL;
            }
          
          if (!no_output)
            {
              printf("%d lines\n", status.lines);
            }
#ifdef HAVE_EXCEPTIONS
        }
      catch (CsscExitvalException e)
        {
          if (e.exitval > retval)
            retval = e.exitval;
        }
#endif          
    }

  return retval;
}


// Explicit template instantiations.
template class range_list<sid>;
template class mylist<sid>;
template class mylist<mystring>;
template class mylist<seq_no>;
template class mylist<delta>;
template class mylist<sccs_pfile::edit_lock>;

#include "stack.h"
template class stack<unsigned short>;

#include "sid_list.h"
template class range_list<release>;

/* Local variables: */
/* mode: c++ */
/* End: */
