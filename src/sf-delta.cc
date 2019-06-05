/*
 * sf-delta.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2002, 2007, 2008, 2009, 2010,
 *  2011, 2014, 2019 Free Software Foundation, Inc.
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
 * CSSC was originally based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Members of sccs_file for adding a delta to the SCCS file.
 *
 */
#include <config.h>
#include <string>

#include <errno.h>
#include <unistd.h>

#include "cssc.h"
#include "failure.h"
#include "sccsfile.h"
#include "pfile.h"
#include "seqstate.h"
#include "run.h"
#include "linebuf.h"
#include "delta.h"
#include "delta-table.h"
#include "delta-iterator.h"
#include "bodyio.h"
#include "file.h"
#include "subst-parms.h"

#undef JAY_DEBUG


using cssc::Failure;

class FileDeleter
{
  std::string name;
  bool as_real_user;
  bool armed;

public:
  FileDeleter(const std::string& s, bool realuser)
    : name(s), as_real_user(realuser), armed(true) { }

  ~FileDeleter()
    {
      if (armed)
        {
          const char *s = name.c_str();
          if (as_real_user)
	    {
	      cssc::Failure deleted = unlink_file_as_real_user(s);
	      if (!deleted.ok())
		{
		  errormsg("%s", deleted.to_string().c_str());
		}
	    }
          else
	    {
	      if (remove(s) != 0)
		{
		  errormsg("%s", strerror(errno));
		}
	    }
        }
    }
  void disarm() { armed = false; }
};



/* Adds a new delta to the SCCS file.  It doesn't add the delta to the
   delta list in sccs_file object, so this should be the last operation
   performed before the object is destroyed. */

bool
sccs_file::add_delta(const std::string& gname,
		     sccs_pfile& pfile,
		     sccs_pfile::iterator it,
                     const std::vector<std::string>& new_mrs,
		     const std::vector<std::string>& new_comments,
                     bool display_diff_output)
{
  ASSERT(mode == UPDATE);

  cssc::Failure checked = check_keywords_in_file(gname.c_str());
  if (!checked.ok())
    {
      // This error message concerns any I/O error on the file.  The
      // "No id keywords" warning, if it is needed, will already have
      // been issued.
      errormsg("%s", checked.to_string().c_str());
      return false;
    }

  /*
   * At this point, encode the contents of "gname", and pass
   * the name of this encoded file to diff, instead of the
   * name of the binary file itself.
   */
  std::string file_to_diff;
  bool bFileIsInWorkingDir;

  if (flags.encoded)
    {
      cssc::Failure encoded = encode_file(gname.c_str(), name.ufile().c_str());
      if (!encoded.ok())
        {
          return false;
        }

      file_to_diff = name.ufile();
      bFileIsInWorkingDir = false;
    }
  else
    {
      file_to_diff = gname;
      bFileIsInWorkingDir = true;
    }


  /* When this function exits, delete the temporary file.
   */
  FileDeleter the_cleaner(file_to_diff, bFileIsInWorkingDir);
  if (!flags.encoded)
    the_cleaner.disarm();


#ifdef CONFIG_UIDS
  // To reproduce this error you can do something like this:
  // admin -n -b s.foo && get -e s.foo && cp /bin/sh foo && ( umask 0444; delta s.foo )
  if (!is_readable(file_to_diff.c_str()))
    {
      // This is the file we just wrote.
      errormsg("File %s (which we just created) is not readable by user %d! "
	       "Please check your umask.",
	       file_to_diff.c_str(),
	       // TODO: do something better here; uid_t may not be
	       // signed.
	       static_cast<int>(geteuid()));
      return false;
    }
#endif

  const std::string sid_name = it->got.as_string();
  const delta *got_delta = find_delta(it->got);
  if (got_delta == NULL)
    {
        // The delta listed in the p-file as gotten does not exist in
        // the history file.
        errormsg("Locked delta %s doesn't exist!", sid_name.c_str());
        return false;
    }

  // Remember seq number that will be the predecessor of the
  // one for the delta.
  seq_no predecessor_seq = got_delta->seq();


  seq_state sstate(highest_delta_seqno());
  prepare_seqstate(sstate, predecessor_seq,
		   it->include, it->exclude,
		   sccs_date());

  /* We used to use fcreate here but as shown by the tests in
   * tests/delta/errorcase.sh, the prior existence of the
   * d-file doesn't cause an error.
   *
   * XXX: slight departure from SCCS behaviour here.  The real thing
   * appears to issue an unlink(2) followed by a create(2) to create
   * the file.  If there is a setuid wrapper, but some ordinary user
   * has sufficient priveleges to create a symlink in the project
   * directory, it should be possible to exploit that race condition
   * to create a file of their choice with contents of their choice,
   * as the user to which the wrapper program is set-user or set-group
   * ID.  I believe that using the flag O_EXCL as fcreate() does resolves
   * that problem.
   */
  const int xmode = gfile_should_be_executable() ? CREATE_EXECUTABLE : 0;
  FILE *get_out;
  if (1)
    {
      cssc::FailureOr<FILE*> fof = fcreate(name.dfile(), CREATE_EXCLUSIVE | xmode);
      if (fof.ok())
	{
	  get_out = *fof;
	}
      else
	{
	  remove(name.dfile().c_str());
	  auto fof2 = fcreate(name.dfile(), CREATE_EXCLUSIVE | xmode);
	  if (fof2.ok())
	    {
	      get_out = *fof2;
	    }
	  else
	    {
	      const Failure f = cssc::FailureBuilder(fof2.fail())
		.diagnose() << "cannot create file " << name.dfile();
	      errormsg("%s", f.to_string().c_str());
	      return false;
	    }
	}
    }
  FileDeleter another_cleaner(name.dfile(), false);

  auto w = cssc::optional<std::string>();
  const struct delta blankdelta;
  struct subst_parms parms(name.dfile(), get_module_name(), get_out,
			   w, blankdelta,
                           0, sccs_date());
  seq_state gsstate(sstate);
  cssc::Failure got = do_get(name.dfile(), gsstate, parms, /*do_kw_subst=*/0,
			     /*show_sid=*/0, /*show_module=*/0, /*debug=*/0,
			     GET_NO_DECODE, /*for_edit=*/false);
  if (!got.ok())
    {
      cssc::Failure f = cssc::make_failure_builder(got)
	.diagnose()
	<< "failed to get " << name.sfile() << " into " << name.dfile();
      warning("%s", f.to_string().c_str());
      return false;
    }

  if (fclose_failed(fclose(get_out)))
    {
      errormsg_with_errno("Failed to close temporary file");
      return false;
    }

  // The delta operation consists of:-
  // 1. Writing out the information for the new delta.
  // 2. Writing out any automatic null deltas.
  // 3. Copying the body of the s-file to the output file,
  //    modifying it as indicated by the output of the diff
  //    program.

  // The new delta header includes info about what deltas
  // are included, excluded, ignored.   Compute that now.
  std::set<seq_no> included, excluded;
  for (seq_no iseq = 1; iseq < highest_delta_seqno(); iseq++)
    {
    if (sstate.is_explicitly_tagged(iseq))
      {
      if (sstate.is_included(iseq))
        {
	  included.insert(iseq);
        }
      else if (sstate.is_excluded(iseq))
        {
          excluded.insert(iseq);
        }
      }
    }

  // Create any required null deltas if we need to.
  if (flags.null_deltas)
    {
      // figure out how many null deltas to make to fill the gap
      // between the highest current trunk release and the one
      // belonging to the new delta.

      // use our own comment.
      std::vector<std::string> auto_comment;
      auto_comment.push_back(std::string("AUTO NULL DELTA"));

      release last_auto_rel = release(it->delta);
      // --last_auto_rel;

      sid id(got_delta->id());
      release null_rel = release(id);
      ++null_rel;

      ASSERT(id.valid());

      int infinite_loop_escape = 10000;

      while (null_rel < last_auto_rel)
        {
          ASSERT(id.valid());
          // add a new automatic "null" release.  Use the same
          // MRs as for the actual delta (is that right?) but
          seq_no new_seq = delta_table->next_seqno();

          // Set up for adding the next release.
          id = null_rel;
          id.next_level();

          delta null_delta('D', id, sccs_date::now(),
                           get_user_name(), new_seq, predecessor_seq,
                           new_mrs, auto_comment);
	  ASSERT (null_delta.inserted() == 0);
	  ASSERT (null_delta.deleted() == 0);
	  ASSERT (null_delta.unchanged() == 0);

          delta_table->prepend(null_delta);

          predecessor_seq = new_seq;

          ++null_rel;

          --infinite_loop_escape;
          ASSERT(infinite_loop_escape > 0);
        }
    }
  // assign a sequence number.
  seq_no new_seq = delta_table->next_seqno();

#if 1
  /* 2002-03-21: James Youngman: we already did this, above */

  // copy the list of excluded and included deltas from the p-file
  // into the delta.  it->include is a range_list<sid>,
  // but what we actually want to create is a list of seq_no.
  if (!it->include.empty())
    {
      const_delta_iterator iter(delta_table.get(), delta_selector::current);
      while (iter.next())
        {
          if (it->include.member(iter->id()))
            {
              included.insert(iter->seq());
            }
        }
    }
  if (!it->exclude.empty())
    {
      const_delta_iterator iter(delta_table.get(), delta_selector::current);
      while (iter.next())
        {
          if (it->exclude.member(iter->id()))
            {
              excluded.insert(iter->seq());
            }
        }
    }
#endif

  // Construct the delta information for the new delta.
  delta new_delta('D', it->delta, sccs_date::now(),
                  get_user_name(), new_seq, predecessor_seq,
                  included, excluded, new_mrs, new_comments);

  // We don't know how many lines will be added/changed yet.
  // end_update() fixes that.
  ASSERT (new_delta.inserted() == 0);
  ASSERT (new_delta.deleted() == 0);
  ASSERT (new_delta.unchanged() == 0);

  new_delta.id().print(stdout);
  printf("\n");

  // Begin the update by writing out the new delta.
  // This also writes out the information for all the
  // earlier deltas.
  FILE *out;
  if (1)
    {
      cssc::FailureOr<FILE*> fof = start_update(new_delta);
      if (!fof.ok())
	return false;
      out = *fof;
    }

  delta_result result =
  body_scanner_->delta(name.dfile(), file_to_diff, highest_delta_seqno(), new_delta.seq(),
		       &sstate, out, display_diff_output);

  // The order of things that we do at this point is quite
  // important; we want only to update the s- and p- files if
  // everything worked.

  // It would be nice to know if the diff failed, but actually
  // diff's return value indicates if there was a difference
  // or not.

  pfile.delete_lock(it);

  if (!result.success)
    return false;		// the delta operation failed.
  new_delta.set_idu(result.inserted, result.deleted, result.unchanged);

  cssc::Failure updated = end_update(&out, new_delta);
  if (!updated.ok())
    {
      errormsg("failed to complete update of %s", updated.to_string().c_str());
      return false;
    }

  printf("%lu inserted\n%lu deleted\n%lu unchanged\n",
         new_delta.inserted(), new_delta.deleted(), new_delta.unchanged());

  if (pfile.update(true).ok())
    return true;
  else
    return false;
}

/* Local variables: */
/* mode: c++ */
/* End: */
