/*
 * writesubst.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 2001, 2004, 2007, 2008, 2009, 2010, 2011, 2014, 2019
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
 *
 * sccsfile::write_subst()
 *
 */

#include <config.h>
#include <string>

#include "cssc.h"
#include "failure.h"
#include "sccsfile.h"
#include "delta.h"
#include "ioerr.h"
#include "subst-parms.h"

// #include "pfile.h"
// #include "seqstate.h"
// #include "delta-iterator.h"
// #include "delta-table.h"


#include <ctype.h>

using std::string;

/* Return TRUE if the specified keyword letter should be
 * expanded in the gotten file.  If the y flag is set, it controls the
 * keyletters which are expanded.  If the y flag is not present, all
 * key letters are expanded.  The y flag is a Solaris 8 extension.
 */
static bool expand_keyletter(char which, const std::set<char>& expanded)
{
  if (expanded.empty())
    return true;
  else
    return expanded.find(which) != expanded.end();
}


cssc::FailureOr<bool>
sccs_file::emit_keyletter_expansion(FILE *out, struct subst_parms *parms, const delta& d, char c) const
{
  // We need to expand the keyletter.
#define RETURN_IF_ERROR(expression) do { cssc::Failure f = (expression); if (!f.ok()) { return f;  } } while(0)
#define ERRNO_ERROR_IF_NONZERO(expression) do { int x = (expression); if (x) { return cssc::make_failure_builder_from_errno(errno); } } while(0)
  switch (c)
    {
    case 'M':
      {
	const char *mod = get_module_name().c_str();
	ERRNO_ERROR_IF_NONZERO(fputs_failed(fputs(mod, out)));
      }
      return false;

    case 'I':
      RETURN_IF_ERROR(d.id().print(out));
      return false;

    case 'R':
      RETURN_IF_ERROR(d.id().printf(out, 'R', 1));
      return false;

    case 'L':
      RETURN_IF_ERROR(d.id().printf(out, 'L', 1));
      return false;

    case 'B':
      RETURN_IF_ERROR(d.id().printf(out, 'B', 1));
      return false;

    case 'S':
      RETURN_IF_ERROR(d.id().printf(out, 'S', 1));
      return false;

    case 'D':
      RETURN_IF_ERROR(parms->now.printf(out, 'D'));
      return false;

    case 'H':
      RETURN_IF_ERROR(parms->now.printf(out, 'H'));
      return false;

    case 'T':
      RETURN_IF_ERROR(parms->now.printf(out, 'T'));
      return false;

    case 'E':
      RETURN_IF_ERROR(d.date().printf(out, 'D'));
      return false;

    case 'G':
      RETURN_IF_ERROR(d.date().printf(out, 'H'));
      return false;

    case 'U':
      RETURN_IF_ERROR(d.date().printf(out, 'T'));
      return false;

    case 'Y':
      if (flags.type)
	{
	  ERRNO_ERROR_IF_NONZERO(fputs_failed(fputs(flags.type->c_str(), out)));
	}
      else
	{
	  // Expands to nothing.
	}
      return false;

    case 'F':
      ERRNO_ERROR_IF_NONZERO(fputs_failed(fputs(base_part(name.sfile()).c_str(),
						out)));
      return false;

    case 'P':
      if (1) // introduce new scope...
	{
	  cssc::FailureOr<string> canon = canonify_filename(name.c_str());
	  if (!canon.ok())
	    return canon.fail();
	  string path(*canon);
	  ERRNO_ERROR_IF_NONZERO(fputs_failed(fputs(path.c_str(), out)));
	}
      return false;

    case 'Q':
      if (flags.user_def)
	{
	  ERRNO_ERROR_IF_NONZERO(fputs_failed(fputs(flags.user_def->c_str(), out)));
	}
      else
	{
	  // Expands to nothing.
	}
      return false;

    case 'C':
      ERRNO_ERROR_IF_NONZERO(printf_failed(fprintf(out, "%u",
						   parms->out_lineno)));
      return false;

    case 'Z':
      {
	const int fail = fputc_failed(fputc('@', out))
	  || fputs_failed(fputs("(#)", out));
	ERRNO_ERROR_IF_NONZERO(fail);
      }
      return false;

    case 'W':
      {
	cssc::optional<std::string> saved_wstring = parms->wstring;
	if (!saved_wstring.has_value())
	  {
	    /* At some point I had been told that SunOS 4.1.4
	     * apparently uses a space rather than a tab here.
	     * However, a test on 4.1.4 shows otherwise.
	     *
	     * From: "Carl D. Speare" <carlds@attglobal.net>
	     * Subject: RE: SunOS 4.1.4
	     * To: 'James Youngman' <jay@gnu.org>,
	     *         "cssc-users@gnu.org" <cssc-users@gnu.org>
	     * Date: Wed, 11 Jul 2001 01:07:36 -0400
	     *
	     * Ok, here's what I got:
	     *
	     * %W% in a file called test.c expanded to:
	     *
	     * @(#)test.c<TAB>1.1
	     *
	     * Sorry, but my SunOS machine is lacking a network
	     * connection, so I can't bring it over into
	     * mail-land. But, there you are, for what it's
	     * worth.
	     *
	     * --Carl
	     *
	     */
	    saved_wstring = std::string("%Z" "%%M" "%\t%" "I%");
	    /* NB: strange formatting of the string above is
	     * to preserve it unchanged even if this source code does
	     * itself get checked into SCCS or CSSC.
	     */
	  }
	else
	  {
	    /* protect against recursion */
	    parms->wstring = cssc::optional<std::string>();
	  }
	ASSERT(saved_wstring.has_value());
	cssc::Failure recursed = write_subst(saved_wstring.value().c_str(),
					     parms, d, true);
	if (!recursed.ok())
	  return recursed;
	if (!parms->wstring.has_value())
	  {
	    parms->wstring = saved_wstring;
	  }
      }
      return false;

    case 'A':
      {
	cssc::Failure recursed = write_subst("%Z""%%Y""% %M""% %I"
					     "%%Z""%",
					     parms, d, true);
	if (!recursed.ok())
	  return recursed;
	return false;
      }

    default:
      return true;
    }
}



/* Write a line of a file after substituting any id keywords in it.
   Returns true if an error occurs. */
cssc::Failure
sccs_file::write_subst(const char *start,
                       struct subst_parms *parms,
                       const delta& d,
		       bool force_expansion) const
{
  FILE *out = parms->out;

  const char *percent = strchr(start, '%');
  while (percent != NULL)
    {
      char c = percent[1];
      if (c != '\0' && percent[2] == '%')
	{
	  if (start != percent
	      && fwrite(start, percent - start, 1, out) != 1)
	    {
	      return cssc::make_failure_builder_from_errno(errno)
		<< "write failed";
	    }

	  if (!force_expansion
	      && false == expand_keyletter(c, flags.substitued_flag_letters))
	    {
	      // We do not expand this key letter.   Just emit the raw
	      // characters.
	      if (fputc_failed(fputc('%', out))
		  ||  fputc_failed(fputc(c,   out))
		  ||  fputc_failed(fputc('%', out)))
		{
		  return cssc::make_failure_builder_from_errno(errno)
		    << "failed to write unexpanded keyletter "
		    << "'%" << c << "%' to output file";
		}
	      else
		{
		  start = percent+3;
		  percent = strchr(start, '%');
		  continue;
		}
	    }
	  percent += 3;

	  cssc::FailureOr<bool> done = emit_keyletter_expansion(out, parms, d, c);
	  if (!done.ok())
	    return done.fail();

	  if (*done)
	    {
	      start = percent - 3;
	      percent = percent - 1;
	      continue;
	    }
	  else
	    {
	      parms->found_id = 1;
	    }
	  start = percent;
	}
      else
	{
	  percent++;
	}
      percent = strchr(percent, '%');
    }

  if (fputs_failed(fputs(start, out)))
    {
      return cssc::make_failure_builder_from_errno(errno) << "write failed";
    }
  return cssc::Failure::Ok();
}
