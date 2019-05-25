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

cssc::FailureOr<sccs_pfile::lock_count_type>
sccs_pfile::write_edit_locks(FILE *out, const std::string& file_name) const
{
  auto diagnose = [file_name](cssc::Failure orig) -> cssc::Failure
    {
      return cssc::FailureBuilder(orig)
      .diagnose() << "failed to write to " << file_name;
    };

  auto write_error = [diagnose]() -> cssc::Failure
    {
      return diagnose(cssc::make_failure_builder_from_errno(errno));
    };

  auto write_edit_lock = [write_error, out, diagnose](struct edit_lock const &it)
    {
      auto written = it.got.print(out);
      if (!written.ok())
	return diagnose(written);

      if (putc_failed(putc(' ', out)))
	{
	  return write_error();
	}
      if (!(written = it.delta.print(out)).ok())
	return diagnose(written);

      if (putc_failed(putc(' ', out))
	  || fputs_failed(fputs(it.user.c_str(), out))
	  || putc_failed(putc(' ', out))
	  || it.date.print(out))
	{
	  return write_error();
	}

      if (!it.include.empty()
	  && ((fputs(" -i", out) == EOF || it.include.print(out))))
	{
	  return write_error();
	}

      if (!it.exclude.empty()
	  && ((fputs(" -x", out) == EOF || it.exclude.print(out))))
	{
	  return write_error();
	}

      if (putc('\n', out) == EOF) {
	return write_error();
      }
      return cssc::Failure::Ok();
    };

  lock_count_type locks_remaining = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      auto written = write_edit_lock(*it);
      if (!written.ok())
	return written;
      locks_remaining++;
    }
  return locks_remaining;
}

cssc::Failure
sccs_pfile::update(bool pfile_already_exists) const
{
  const std::string q_name(name.qfile());

  FILE *pf = fcreate(q_name.c_str(), CREATE_EXCLUSIVE);
  if (pf == NULL)
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << q_name << ": Can't create temporary file";
    }
  cssc::Failure qfile_deletion_result = cssc::Failure::Ok();

  ResourceCleanup qfile_deleter([q_name, &pf, &qfile_deletion_result]() {
      if (pf != NULL)
	{
	  if (fclose_failed(fclose(pf)))
	    {
	      qfile_deletion_result = cssc::make_failure_builder_from_errno(errno)
		.diagnose() << "failed to close " << q_name;
	    }
	  pf = NULL;
	}
      if (0 != remove(q_name.c_str()))
	{
	  qfile_deletion_result = cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "failed to remove " << q_name;
	}
    });

  auto rewrite = [q_name, &qfile_deleter, &pf, this, pfile_already_exists]() -> cssc::Failure
    {
      auto written = write_edit_locks(pf, q_name);
      if (!written.ok())
	return written.fail();
      const lock_count_type locks_remaining = *written;
      if (fclose_failed(fclose(pf)))
	{
	  return cssc::make_failure_builder_from_errno(errno)
	  .diagnose() << q_name << ": write error";
	}
      pf = NULL;

      if (pfile_already_exists)
	{
	  if (remove(pname.c_str()) != 0)
	    {
	      return cssc::make_failure_builder_from_errno(errno)
	      .diagnose() << pname << ": can't remove old p-file";
	    }
	}

      if (locks_remaining)	// pfile is still required
	{
	  if (rename(q_name.c_str(), pname.c_str()) != 0)
	    {
	      // this is really bad; we have already deleted the old p-file!
	      return cssc::make_failure_builder_from_errno(errno)
	      .diagnose() << "failed to rename " << q_name << " to " << pname;
	    }
	  qfile_deleter.disarm();
	}

      return cssc::Failure::Ok();
    };

  auto rewrite_result = rewrite();
  if (!rewrite_result.ok())
    return rewrite_result;
  else
    return qfile_deletion_result;
}

/* Local variables: */
/* mode: c++ */
/* End: */
