/*
 * get.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2002, 2007, 2008, 2009, 2010,
 *  2011, 2014, 2019, 2024 Free Software Foundation, Inc.
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
 * Extract a requested delta from a SCCS file.
 *
 */
#include "config.h"

#include <functional>
#include <initializer_list>
#include <string>
#include <errno.h>

#include "cssc.h"
#include "delta-table.h"
#include "failure.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "delta.h"
#include "pfile.h"
#include "my-getopt.h"
#include "version.h"
#include "except.h"
#include "file.h"
#include "privs.h"
#include "subst-parms.h"

#include <limits.h>


/* Prints a list of included or excluded SIDs. */

static cssc::Failure
print_id_list(FILE *fp, const char *s, std::vector<sid> const &list)
{
  cssc::Failure status;
  if (!list.empty())
    {
      if (fprintf_failed(fprintf(fp, "%s:\n", s)))
	status = cssc::Update(status, cssc::make_failure_from_errno(errno));

      for (const auto& sid : list)
        {
	  status = cssc::Update(status, sid.print(fp));
          if (fputc_failed(fputc('\n', fp)))
	    status = cssc::Update(status, cssc::make_failure_from_errno(errno));
        }
    }
  return status;
}

void
usage() {
        fprintf(stderr,
"usage: %s [-begkmnpstLV] [-c date] [-r SID] [-i range] [-w string]\n"
"\t[-x range] [-G gfile] file ...\n",
                prg_name);
}


static void maybe_clear_archive_bit(const std::string &)
{
#ifdef CONFIG_USE_ARCHIVE_BIT
  clear_archive_bit(gname);
#endif
}

#define EXITVAL_INVALID_OPTION (1)

using cssc::Update;
using cssc::Failure;
using cssc::FailureOr;
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
  int send_body_to_stdout = 0;		/* -p */
  int silent = 0;                       /* -s */
  int no_output = 0;                    /* -g */
  cssc::optional<std::string> wstring;	/* -w */
  sid_list include, exclude;            /* -i, -x */
  sccs_date cutoff_date;                /* -c */
  int show_sid = 0;                     /* -m */
  int show_module = 0;                  /* -n */
  int debug = 0;                        /* -D */
  std::string gname;                    /* -G */
  int got_gname = 0;                    /* -G */
  seq_no seq = 0;                       /* -a */
  int get_top_delta = 0;                /* -t */
  bool real_file;
  bool delta_summary = false;	        /* -L, -l */
  bool create_lfile = false;            /* -l */
  FILE *commentary = stdout;

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
          send_body_to_stdout = 1;
	  commentary = stderr;
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
		  commentary = stderr;
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
	  commentary = stderr;
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
	  if (opts.getarg())
	    {
	      wstring = std::string(opts.getarg());
	    }
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
            seq = static_cast<seq_no>(i);
          }
          break;

        case 't':
          get_top_delta = 1;
          break;

        case 'G':
          got_gname = 1;
          send_body_to_stdout = 0;
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

  if (silent)
    {
      FailureOr<FILE*> opened = open_null();
      if (!opened.ok())
	return 1;       // fatal error.
      commentary = *opened;
    }

  if (no_output)
    {
      got_gname = 0;
      gname = "null";
      FailureOr<FILE*> opened = open_null();
      if (!opened.ok())
	return 1;       // fatal error.
      out = *opened;
    }
  else if (send_body_to_stdout)
    {
      gname = "standard output";
      out = stdout;
    }


  sccs_file_iterator iter(opts);
  if (sccs_file_iterator::source::NONE == iter.using_source())
    {
      errormsg("No SCCS file specified");
      return 1;
    }

  while (iter.next())
    {
      try
        {
          sccs_name &name = iter.get_name();

          // Print the name of the SCCS file unless exactly one
          // was specified.
          if (!iter.unique())
            {
              fprintf(commentary, "\n%s:\n", name.c_str());
            }


          sccs_pfile *pfile = NULL;
          if (for_edit)
            {
              pfile = new sccs_pfile(name, sccs_pfile::pfile_mode::PFILE_APPEND);
            }

          sccs_file file(name, READ);
          sid new_delta;
          sid retrieve;

          if (seq)
            {
              if (org_rid.valid())
                {
                  warning("both the -r and the -a "
                          "option have been specified; "
                          "the -r option has been ignored.");
                }

              if (!file.find_requested_seqno(seq, retrieve))
                {
                  errormsg("%s: Requested sequence number %u not found.",
                           name.c_str(), static_cast<unsigned>(seq));
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

              if ( (nullptr==pfile) || !file.test_locks(retrieve, *pfile))
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

          if (!send_body_to_stdout && !no_output)
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

	      if (file.gfile_should_be_executable())
		{
		  mode |= CREATE_EXECUTABLE;
		}

              real_file = true;
	      FailureOr<FILE*> fof = fcreate(gname, mode);
              if (!fof.ok())
                {
		  out = NULL;
		  errormsg("%s", fof.to_string().c_str());
                  retval = 1;
                  continue;     // with next file....
                }
	      out = *fof;
            }

	  FILE *summary_file = NULL;
	  if (delta_summary)
	    {
	      if (create_lfile)
		{
		  std::string lname = name.lfile();
		  FailureOr<FILE*> fof = fcreate(lname, CREATE_READ_ONLY);
		  if (!fof.ok())
		    {
		      // XXX: This would probably be surprising to the
		      // user (compared witht he alternative of faling
		      // with an error message).  Compare what other
		      // SCCS implementation do.
		      summary_file = NULL;
		    }
		  else
		    {
		      summary_file = *fof;
		    }
		}
	      else
		{
		  summary_file = stdout;
		}
	    }

          const int keywords = !suppress_keywords;
	  Failure f;

	  ResourceCleanup gfile_cleaner([out, gname, &f, real_file](){
	      if (!real_file)
		return;
	      if (fclose_failed(fclose(out)))
		{
		  f = Update(f, cssc::make_failure_builder(fclose_failure(out))
			     .diagnose() << "failed to close " << gname);
		}
	      if (0 != remove(gname.c_str()))
		{
		  f = Update(f, cssc::make_failure_builder_from_errno(errno)
			     .diagnose() << "failed to delete " << gname);
		}
	    });
	  cssc::FailureOr<get_status> gotten =
	    file.get(out, gname, summary_file, retrieve, cutoff_date,
		     include, exclude, keywords, wstring,
		     show_sid, show_module, debug, for_edit);
	  if (gotten.ok())
	    {
	      // The "get" operation succeeded, keep the output.
	      gfile_cleaner.disarm();
	    }
	  else
	    {
	      f  = Update(f, gotten.fail());
	    }
	  if (create_lfile)
	    {
              fclose (summary_file);
	    }

          // We delete this file if the "get" failed.  However, this
          // is not conditional on "retval" since if the iteration for
          // the previous history file failed, we would like to
          // succeed on the remaining files.
	  if (real_file)
	    {
	      f = Update(f, fclose_failure(out));
	      /* The g-file was created with the real uid,
	       * and so if we want to change its mode, we
	       * will have to temporarily set EUID=RUID.
	       * set_gfile_writable already does that.
	       */
	      f = Update(f,
			 set_gfile_writable(gname, suppress_keywords,
					    file.gfile_should_be_executable()));
	      if (suppress_keywords)
		{
		  maybe_clear_archive_bit(gname);
		}
	    }

          if (!gotten.ok()) // get failed.
            {
	      // gfile_cleaner should delete the unwanted g-file.
              continue;
            }

	  f = Update(f, print_id_list(commentary, "Included", (*gotten).included));
	  f = Update(f, print_id_list(commentary, "Excluded", (*gotten).excluded));
	  f = Update(f, retrieve.print(commentary));
	  f = Update(f, fputc_failure('\n', commentary));
          if (for_edit)
            {
	      f = Update(f, fprintf_failure(fprintf(commentary, "new delta ")));
	      f = Update(f, new_delta.print(commentary));
	      f = Update(f, fputc_failure('\n', commentary));

	      Failure added = pfile->add_lock(retrieve, new_delta, include, exclude);
              if (!added.ok())
                {
		  f = Update(f, added);
                  // Failed to add the lock to the p-file.
                  if (real_file)
                    {
		      f = Update(f, cssc::make_failure_builder
				 (unlink_file_as_real_user(gname.c_str()))
				 .diagnose() << "Failed to remove file " << gname);
                    }
                }
              delete pfile;
              pfile = NULL;
            }
	  if (!f.ok())
	    {
	      retval = 1;
	    }
          if (!no_output)
            {
	      ASSERT(gotten.ok());
	      // TODO: should an fprintf failure here affect the exit status?
	      fprintf(commentary, "%u lines\n", (*gotten).lines);
            }
        }
      catch (CsscExitvalException e)
        {
          if (e.exitval > retval)
            retval = e.exitval;
        }
    }

  return retval;
}

/* Output the specified version to a file with possible modifications.
   Most of the actual work is done with a seqstate object that
   figures out whether or not given line of the SCCS file body
   should be included in the output file. */
cssc::FailureOr<get_status>
sccs_file::get(FILE *out, const std::string& gname,
	       FILE *summary_file,
	       sid id, sccs_date cutoff_date,
               sid_list include, sid_list exclude,
               bool keywords, cssc::optional<std::string> wstring,
               bool show_sid, bool show_module, bool debug,
	       bool for_edit)
{
  ASSERT(nullptr != delta_table_);

  seq_state state(highest_delta_seqno());
  const delta *d = find_delta(id);
  ASSERT(d != NULL);

  ASSERT(nullptr != delta_table_);

  cssc::Failure edit_allowed = edit_mode_permitted(for_edit);
  if (!edit_allowed.ok())	// "get -e" on BK files is not allowed
    return edit_allowed;

  prepare_seqstate(state, d->seq(), include, exclude, cutoff_date);

  // Fix by Mark Fortescue.
  // Fix Cutoff Date Problem
  const delta *dparm;
  bool set=false;

  for (seq_no s = d->seq(); s>0; s--)
    {
      if (delta_table_->delta_at_seq_exists(s))
	{
	    const struct delta & del = delta_table_->delta_at_seq(s);

	    if (!state.is_excluded(s) && !set)
	      {
		dparm = find_delta(del.id());
		set = true;
	      }
	}
    }
  if ( !set ) dparm = d;
  // End of fix

  if (getenv("CSSC_SHOW_SEQSTATE"))
    {
      for (seq_no s = d->seq(); s>0; s--)
        {
          if (!delta_table_->delta_at_seq_exists(s))
            {
              /* skip non-existent seq number */
              continue;
            }

          fprintf(stderr, "%4d (", s);
          delta_table_->delta_at_seq(s).id().dprint(stderr);
          fprintf(stderr, ") ");

          if (state.is_explicitly_tagged(s))
            {
              fprintf(stderr, "explicitly ");
            }

          if (state.is_ignored(s))
            {
              fprintf(stderr, "ignored\n");
            }
          else if (state.is_included(s))
            {
              fprintf(stderr, "included\n");
            }
          else if (state.is_excluded(s))
            {
              fprintf(stderr, "excluded");
            }
          else
            {
              fprintf(stderr, "irrelevant\n");
            }
        }
    }

  if (summary_file)
    {
      bool first = true;

      for (seq_no s = d->seq(); s>0; s--)
        {
          if (delta_table_->delta_at_seq_exists(s)
	      && state.is_included(s))
	    {
	      const struct delta & it = delta_table_->delta_at_seq(s);

	      fprintf (summary_file, "%s    ",
		       first ? "" : "\n");
	      first = false;
	      it.id().print(summary_file);
	      fprintf (summary_file, "\t");
	      it.date().print(summary_file);
	      fprintf (summary_file, " %s\n", it.user().c_str());

	      for (const std::string& comment : it.comments())
		{
		  fprintf (summary_file, "\t%s\n", comment.c_str());
		}
	    }
	}
      fputc ('\n', summary_file);
    }



  // The subst_parms here may not be the Whole Truth since
  // the cutoff date may affect which version is actually
  // gotten.  That's taken care of; the correct delta is
  // passed as a parameter to the substitution function.
  // (eugh...)
  // Changed to use dparm not d to deal with Cutoff Date (Mark Fortescue)
  struct subst_parms parms(gname, get_module_name(),
			   out, wstring, *dparm,
                           0, sccs_date::now());


  cssc::Failure got = do_get(gname, state, parms, keywords, show_sid, show_module, debug,
			     false, false);
  if (!got.ok())
    {
      // TODO: verify whether or not we need to delete the g-file.
      return got;
    }

  // only issue a warning about there being no keywords
  // substituted, IF keyword substitution was being done.
  if (keywords && !parms.found_id)
    {
      no_id_keywords(name_.c_str());
      // this function normally returns.
    }

  /* Set the return status. */
  struct get_status goodstatus;
  goodstatus.lines = parms.out_lineno;

  seq_no seq;
  for(seq = 1; seq <= highest_delta_seqno(); seq++)
    {
      if (state.is_explicitly_tagged(seq))
        {
          const sid id_of_this_seq = seq_to_sid(seq);

          if (state.is_included(seq))
            goodstatus.included.push_back(id_of_this_seq);
          else if (state.is_excluded(seq))
            goodstatus.excluded.push_back(id_of_this_seq);
        }
    }
  return goodstatus;
}



/* Local variables: */
/* mode: c++ */
/* End: */
