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


/* Write a line of a file after substituting any id keywords in it.
   Returns true if an error occurs. */

cssc::Failure
sccs_file::write_subst(const char *start,
                       struct subst_parms *parms,
                       const delta& d,
		       bool force_expansion) const
{
  int err = [this, &start, parms, d, force_expansion]() -> int
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
		  return 1;
		}

	      int err = 0;
	      if (!force_expansion
		  && false == expand_keyletter(c, flags.substitued_flag_letters))
		{
		  // We do not expand this key letter.   Just emit the raw
		  // characters.
		  err = fputc_failed(fputc('%', out))
		    ||  fputc_failed(fputc(c,   out))
		    ||  fputc_failed(fputc('%', out));

		  if (err)
		    {
		      return 1;
		    }
		  else
		    {
		      start = percent+3;
		      percent = strchr(start, '%');
		      continue;
		    }
		}
	      percent += 3;



	      // We need to expand the keyletter.
	      switch (c)
		{
		case 'M':
		  {
		    const char *mod = get_module_name().c_str();
		    err = fputs_failed(fputs(mod, out));
		  }
		  break;

		case 'I':
		  err = !d.id().print(out).ok();
		  break;

		case 'R':
		  err = !d.id().printf(out, 'R', 1).ok();
		  break;

		case 'L':
		  err = !d.id().printf(out, 'L', 1).ok();
		  break;

		case 'B':
		  err = !d.id().printf(out, 'B', 1).ok();
		  break;

		case 'S':
		  err = !d.id().printf(out, 'S', 1).ok();
		  break;

		case 'D':
		  err = parms->now.printf(out, 'D');
		  break;

		case 'H':
		  err = parms->now.printf(out, 'H');
		  break;

		case 'T':
		  err = parms->now.printf(out, 'T');
		  break;

		case 'E':
		  err = d.date().printf(out, 'D');
		  break;

		case 'G':
		  err = d.date().printf(out, 'H');
		  break;

		case 'U':
		  err = d.date().printf(out, 'T');
		  break;

		case 'Y':
		  if (flags.type)
		    {
		      err = fputs_failed(fputs(flags.type->c_str(), out));
		    }
		  break;

		case 'F':
		  err =
		    fputs_failed(fputs(base_part(name.sfile()).c_str(),
				       out));
		  break;

		case 'P':
		  if (1) // introduce new scope...
		    {
		      cssc::FailureOr<string> canon = canonify_filename(name.c_str());
		      if (!canon.ok())
			{
			  // XXX: probably the resulting error message issued
			  // by the caller will be a bit inaccurate.
			  err = 1;
			}
		      else
			{
			  string path(*canon);
			  err = fputs_failed(fputs(path.c_str(), out));
			}
		    }
		  break;

		case 'Q':
		  if (flags.user_def)
		    {
		      err = fputs_failed(fputs(flags.user_def->c_str(), out));
		    }
		  break;

		case 'C':
		  err = printf_failed(fprintf(out, "%u",
					      parms->out_lineno));
		  break;

		case 'Z':
		  if (fputc_failed(fputc('@', out))
		      || fputs_failed(fputs("(#)", out)))
		    {
		      err = 1;
		    }
		  else
		    {
		      err = 0;
		    }
		  break;

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
		    if (!write_subst(saved_wstring.value().c_str(), parms, d, true).ok())
		      err = 1;
		    if (!parms->wstring.has_value())
		      {
			parms->wstring = saved_wstring;
		      }
		  }
		  break;

		case 'A':
		  if (!write_subst("%Z""%%Y""% %M""% %I"
				   "%%Z""%",
				   parms, d, true).ok())
		    {
		      err = 1;
		    }
		  break;

		default:
		  start = percent - 3;
		  percent = percent - 1;
		  continue;
		}

	      parms->found_id = 1;

	      if (err)
		{
		  return 1;
		}
	      start = percent;
	    }
	  else
	    {
	      percent++;
	    }
	  percent = strchr(percent, '%');
	}

      return fputs_failed(fputs(start, out));
    }();
  if (err )
    {
      // TODO: refactor this funciton so that we can use a more
      // specific error code.
      return cssc::make_failure_builder(cssc::errorcode::GetFileBodyFailed);
    }
  return cssc::Failure::Ok();
}
