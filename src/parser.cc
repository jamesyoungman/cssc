/*
 * parser.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2003, 2004, 2007, 2008, 2009,
 *  2010, 2011, 2014, 2019 Free Software Foundation, Inc.
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
 */
#include "config.h"

#include <sys/stat.h>           /* fstat(), struct stat */

#include "cssc.h"
// TODO: eliminate the need to #include "defaults.h" directly.
#include "defaults.h"
#include "parser.h"

#include "delta.h"
#include "delta-table.h"
#include "file.h"
#include "linebuf.h"
#include "quit.h"

namespace
{

#if defined HAVE_FILENO && defined HAVE_FSTAT
  /* If an SCCS file has a link count greater than one, then the normal
   * process of updating the file will break the link.  We try to detect this
   * even if the file is being opened to reading only, to give an early
   * warning (and because SCCS does so).
   */
  int just_one_link(FILE *f)
  {
    int fd = fileno(f);
    if (fd >= 0)
      {
	struct stat st;
	if (0 != fstat(fd, &st))
	  {
	    /* We cannot stat the file descriptor.  Perhaps there is a
	     * file system functionality issue.   If that's the case then we
	     * will give it the benefit of the doubt on the link count front.
	     */
	    return 1;  /* We're happy with the file */
	  }
	if (st.st_nlink > 1)
	  return 0;               /* We don't like it. */
      }
    return 1;                     /* OK. */
  }
#else
  int just_one_link(FILE *f)
  {
    /* Without fileno(), we have no way of checking. */
    return 1;
  }
#endif


  FILE * do_open_sccs_file(const char *name, sccs_file_open_mode mode,
			   const ParserOptions&)
  {
    FILE *f_local;

#ifdef CONFIG_OPEN_SCCS_FILES_IN_BINARY_MODE
    f_local = fopen(name, "rb");
#else
    if (mode == UPDATE)
      f_local = fopen(name, "r+");
    else
      f_local = fopen(name, "r");
#endif

    if (f_local == NULL)
      {
	const char *purpose = (mode == UPDATE) ? "update" : "reading";
	s_missing_quit("Cannot open SCCS file %s for %s", name, purpose);
	/*NOTEACHED*/
	return nullptr;
      }

    if (!just_one_link(f_local))
      {
	// xx: issue error message here
	errormsg("%s had a hard link count which is greater than one.\n"
		 "This means that the normal process of updating the file\n"
		 "would break the hard link.  This error is therefore fatal,\n"
		 "please fix the problem.\n",
		 name);
	(void)fclose(f_local);
	return nullptr;
      }
    return f_local;
  }
}  // namespace


// Factory function for creating a parser.
std::unique_ptr<sccs_file_parser::open_result>
sccs_file_parser::open_sccs_file(const std::string& name,
				 sccs_file_open_mode mode,
				 ParserOptions opts)
{
  FILE * f = do_open_sccs_file(name.c_str(), mode, opts);
  if (nullptr == f)
    {
      return nullptr;		// we already issued an error message.
    }

  auto p = std::make_unique<sccs_file_parser>(name, mode, f, constructor_cookie{});
  // TODO: having an f_ member in a base class and passing in the same
  // FILE* as a function parameter is a bit of a code smell.
  auto open_result = p->parse_header(f, opts);
  if (open_result)
    {
      open_result->parser = std::move(p);
    }
  return open_result;
}


sccs_file_parser::sccs_file_parser(const string& n, sccs_file_open_mode m,
				   FILE *f,
				   sccs_file_parser::constructor_cookie)
  : sccs_file_reader_base(n, f, sccs_file_location(n, 0)),
    mode_(m), is_bk_file_(false)
{
}

/* Reads a delta from the SCCS file's delta table and adds it to the
   delta table. */

std::unique_ptr<delta>
sccs_file_parser::read_delta() {
        /* The current line should be an 's' control line */

        ASSERT(bufchar(1) == 's');
        check_arg();

        char *args[7];          /* Stores the result of spliting a line */

        if (plinebuf->split(3, args, 3, '/') != 3)
          {
            corrupt(here(), "Two /'s expected");
          }

	std::unique_ptr<delta> tmp = std::make_unique<delta>();
        tmp->set_idu(strict_atoul_idu(here(), args[0]),
		     strict_atoul_idu(here(), args[1]),
		     strict_atoul_idu(here(), args[2]));

	char line_type;
        if (!read_line(&line_type) || (line_type != 'd'))
	  {
	    corrupt(here(), "Expected '@d'");
	  }

        check_arg();

        plinebuf->split(3, args, 7, ' ');

        if (delta::is_valid_delta_type(args[0][0])
            && (args[0][1] == 0))
          {
            tmp->set_type(args[0][0]);
          }
        else
          {
            corrupt(here(), "Bad delta type %s", args[0]);
          }

	auto newid = sid(args[1]);
	if (!newid.valid())
	  {
	    corrupt(here(), "Bad SID %s", args[1]);
	  }
	tmp->set_id(newid);

        auto newdate = sccs_date(args[2], args[3]);
        if (!newdate.valid())
	  {
	    corrupt(here(), "Bad Date/Time %s %s", args[2], args[3]);
	  }
        tmp->set_date(newdate);

        tmp->set_user(args[4]);
        tmp->set_seq(strict_atous(here(), args[5]));
        tmp->set_prev_seq(strict_atous(here(), args[6]));

        /* Read in any lists of included, excluded or ignored seq. no's. */

        char c;
	if (!read_line(&c))
	  {
	    corrupt(here(), "Unexpected end-of-file");
	  }

        int i;
        const char *start;
        bool bDebug = getenv("CSSC_SHOW_SEQSTATE") ? true : false;
        for(i = 0; i < 3; i++) {
                if (c == "ixg"[i]) {

                  switch (c)
                    {
                    case 'i':
                      tmp->set_has_includes(true);
                      break;

                    case 'x':
                      {
                        warning("feature not fully tested: "
                                "excluded delta in SID %s ",
                                tmp->id().as_string().c_str());
                        tmp->set_has_excludes(true);
                      }
                      break;

                    case 'g':
                      tmp->set_has_ignores(true);
                      break;
                    }

                  if (bufchar(2) != ' ')
                    {
		      // throw line away.
                      read_line(&c);  // FIXME: missing EOF check here.
                      continue;
                    }

                        check_arg();

                        start = plinebuf->c_str() + 3;
                        do {
                                // In C++, strchr() is overloaded so that
                                // it returns const char* if the first
                                // argument is const char*, and char* only if
                                // the first argument is char*.
                                const char *end = strchr(start, ' ');
                                if (end != NULL) {
                                  //*end++ = '\0';
                                  const char *p = plinebuf->c_str();
                                  plinebuf->set_char(end-p, 0);
                                  ASSERT(*end == 0);
                                  ++end;
                                }
                                seq_no seq = strict_atous(here(), start);
                                switch (c) {
                                case 'i':
                                  if (bDebug)
                                    {
                                      fprintf(stderr, "Including seq %lu\n",
                                              (unsigned long)seq);
                                    }

                                        tmp->add_include(seq);
                                        break;

                                case 'x':
                                  if (bDebug)
                                    {
                                      fprintf(stderr, "Excluding seq %lu\n",
                                              (unsigned long)seq);
                                    }
                                        tmp->add_exclude(seq);
                                        break;

                                case 'g':
                                        tmp->add_ignore(seq);
                                        break;
                                }
                                start = end;
                        } while (start != NULL);

                        read_line(&c); // FIXME: unchecked EOF.
                }
        }

        // According to Hyman Rosen <hymie@jyacc.com>, it is possible
        // to have a ^A m line which has no argument.  Therefore we don't
        // use check_arg().

        // According to Hyman Rosen <hymie@jyacc.com>, it is sometimes
        // possible to have ^Am lines after ^Ac lines, as well as the
        // more usual before.  Hence we now cope with both.

        while (c == 'm' || c == 'c')
          {
            if (c == 'm')
              {
                if (bufchar(2) == ' ')
                  {
                    tmp->add_mr(plinebuf->c_str() + 3);
                  }
              }
            else if (c == 'c')
              {
                /* Larry McVoy's extensions for BitKeeper and BitSCCS
                 * add in extra stuff like "^AcSyadayada".  Real SCCS
                 * doesn't mind about that, so at Larry's request, we
                 * tolerate it too.   No idea what these lines mean though.
                 * Ask <lm@bitmover.com> for more information.  Anyway,
                 * normal comment lines look like "^Ac yadayada" instead,
                 * and check_arg() exists to check for the space.   Hence,
                 * to support Larry's extensions, we don't call check_arg()
                 * here.
                 */
                if (is_bk_file_)
                  {
                    check_bk_comment(c, bufchar(2));
                  }
                else
                  {
		    if (bufchar(2) == '\0')
		      {
			/* Some historic versions of SCCS emit totally empty
			 * comment lines.  We accept those.
			 */
		      }
                    else if (bufchar(2) != ' ')
                      {
                        saw_unknown_feature("Unknown special comment "
                                            "intro '%c%c'",
                                            c, bufchar(2));
                      }
                  }
                tmp->add_comment(plinebuf->c_str() + 3);
              }

            read_line(&c);	// FIXME: check for EOF
          }


        if (c != 'e') {
	  corrupt(here(), "Expected '@e'");
        }

        check_noarg();

        return tmp;
}

// Convert a number field in an SCCS file to a
// number.  Fields representing numbers in
// SCCS files should top out at 9999.

unsigned long
sccs_file_parser::strict_atoul_idu(const sccs_file_location& loc, const char *s) const
{
  unsigned long n = 0;
  bool found_ws = false;
  const unsigned long limit = 99999uL;

  /* Unix System III pads to the left with spaces in the
   * numbers, while more modern versions of SCCS pad to
   * the left with zeroes.   We don't allow left-pad with
   * whitespace characters other than an actual space.
   */
  while (' ' == *s)
    {
      ++s;
      found_ws = true;
    }

  if (found_ws)
    {
      warning("%s contains spaces in the line counts in its delta table.",
              name().c_str());
      if ((UPDATE == mode_) || (FIX_CHECKSUM == mode_))
        {
          warning("These leading spaces will be converted to leading zeroes.");
        }
    }

  if ('-' == *s)
    {
      corrupt (loc, "Line counts should be positive");
    }
  else
    {
      char *end;
      n = strtoul (s, &end, 10);
      if (*end && (*end) != ' ')
        {
          corrupt (loc, "Unexpected suffix %s on line number count", end);
        }
    }

  if (n > limit)
    {
      warning("%s: %s: number field exceeds %lu.",
              name().c_str(), loc.name().c_str(), limit);
    }

  return n;
}


static bool eat_rest_of_line(FILE* f_local, const std::string& name)
{
  int c;
  errno = 0;
  while ( (c=getc(f_local)) != '\n')
    {
      if (EOF == c)
        {
          const int saved_errno = errno;
          (void)fclose(f_local);
          errno = saved_errno;
          if (errno)
            {
              perror(name.c_str());
            }
          else
            {
              s_corrupt_quit("%s: Unexpected EOF.", name.c_str());
              /*NOTEACHED*/
            }
          return false;
        }
    }
  return true;
}


/* Parse the header of an SCCS file. */
std::unique_ptr<sccs_file_parser::open_result>
sccs_file_parser::parse_header(FILE *f_local, ParserOptions opts)
{
  const char* name = this->name().c_str();

  bool badMagic = false;
  bool is_bk = false;
  if (getc(f_local) != '\001')
    {
      badMagic = true;
    }
  else
    {
      char magicMarker = getc(f_local);
      if (magicMarker == 'H')
        {
          if (READ == mode_)
            {
              /* We support read-only access to BK files. */
              warning("%s is a BitKeeper file.", name);
            }
          else
            {
              errormsg("%s: This is a BitKeeper file, and only read-only "
                       "access to BitKeeper files is supported at the moment, "
                       "sorry.\n",
                       name);
              (void)fclose(f_local);
              return nullptr;
            }
          // Inform the caller that this is a BK file.
          is_bk = true;
        }
      else if (magicMarker != 'h')
        {
          badMagic = true;
        }
    }

  if (badMagic)
    {
      (void)fclose(f_local);
      s_corrupt_quit("%s: No SCCS-file magic number.  "
                     "Did you specify the right file?", name);
      /*NOTEACHED*/
      return nullptr;
    }

  if (!eat_rest_of_line(f_local, this->name()))
    {
      return nullptr;
    }

  int sum = 0u;
  /* Read the whole file and compute the checksum. */
  {
    int c;
    while ((c=getc(f_local)) != EOF)
      sum += (char) c;    // Yes, I mean plain char, not signed, not unsigned.

    if (ferror(f_local))
      {
	perror(name);
	(void)fclose(f_local);
	return nullptr;
      }
  }

#ifdef CONFIG_OPEN_SCCS_FILES_IN_BINARY_MODE
  fclose(f_local);
  if (mode == UPDATE)
    f_local = fopen(name, "r+");
  else
    f_local = fopen(name, "r");

  if (NULL == f_local)
    {
      perror(name);
      return nullptr;
    }

#else
  rewind(f_local);
  if (ferror(f_local))
    {
      perror(name);
      (void)fclose(f_local);
      return nullptr;
    }
#endif

  std::unique_ptr<open_result> result = std::make_unique<open_result>();
  result->computed_sum = sum & 0xFFFFu;
  result->is_bk = is_bk;

  // If the history file is executable, remember this fact.
  result->is_executable = false;
  bool executable;
  if (get_open_file_xbits(f_local, &executable))
    {
      result->is_executable = executable;
    }

  char c;
  read_line(&c);		// FIXME: check for EOF

  // open_sccs_file() should have already checked that the first line
  // is ^Ah or ^Ah, so this assertion is really just checking that
  // open_sccs_file() did the right thing.
  if (is_bk)
    {
      ASSERT(c == 'H');
    }
  else
    {
      ASSERT(c == 'h');
    }

  /* the checksum is represented in the file as decimal.
   */
  signed int given_sum = 0;
  if (1 != sscanf(plinebuf->c_str(),
		  (is_bk ? "%*cH%d" : "%*ch%d"),
		  &result->stored_sum))
    {
      if (!opts.silent_checksum_error())
	{
	  errormsg("Expected checksum line, found line beginning '%.3s'\n",
		   plinebuf->c_str());
	}
      result->checksum_valid_ = false;
    }
  else
    {
      given_sum &= 0xFFFFu;
      result->checksum_valid_ = (result->stored_sum == result->computed_sum);
      if (!result->checksum_valid_ && !opts.silent_checksum_error())
	{
	  warning("%s: bad checksum "
		  "(expected=%d, calculated %d).\n",
		  name, result->stored_sum, result->computed_sum);
	}
    }

  /* Read the delta table. */
  read_line(&c);		// FIXME: detect eof
  while (c == 's')
    {
      if (!result->delta_table)
	{
	  result->delta_table = std::make_unique<cssc_delta_table>();
	}
      std::unique_ptr<delta> d = read_delta();
      result->delta_table->add(*d); // FIXME: memory allocation churn, excess copying
      read_line(&c);		// FIXME: detect eof
    }

  if (c != 'u')
    {
      corrupt(here(), "Expected '@u'");
    }

  check_noarg();

  read_line(&c);		// FIXME: detect eof
  while (c != 'U')
    {
      if (c != 0)
        {
          corrupt(here(), "User name expected.");
        }
      result->users.push_back(plinebuf->c_str());
      read_line(&c);		// FIXME: detect eof
    }

  /* Sun's Code Manager sometimes emits lines of the form "^AU 0" and
   * so these lines fail the "no argument" check.  So we no longer do
   * that check for "^AU" lines.  A file including lines of this type
   * was provided by Marko Rauhamaa <marko@tekelec.com>.
   */
  /* check_noarg(); */


  read_line(&c); // FIXME: detect eof
  while (c == 'f')
    {
      check_arg();

      if (bufchar(3) == '\0'
          || (bufchar(4) != '\0' && bufchar(4) != ' '))
        {
          corrupt(here(), "Bad flag arg");
        }

      // We have to be careful to not crash on input lines like
      // "^Af v".  That is, bufchar[4] may well be zero!
      // Thanks to William W. Austin <bill@baustin.alph.att.com>
      // for this diagnosis.
      if (bufchar(4) == ' ')
        {
	  const char *arg = plinebuf->c_str() + 5;
	  result->flags.push_back(parsed_flag(here(), bufchar(3), string(arg)));
        }
      else
        {
	  result->flags.push_back(parsed_flag(here(), bufchar(3)));
        }
      read_line(&c);		// FIXME: eof detection
    }

  if (c != 't')
    {
      corrupt(here(), "Expected '@t'");
    }

  /* Sun's Code Manager sometimes emits lines of the form "^At 0" and
   * so these lines fail the "no argument" check.  So we no longer do
   * that check for "^At" lines.  A file including lines of this type
   * was provided by Marko Rauhamaa <marko@tekelec.com>.
   */
  /*check_noarg();*/

  read_line(&c);		// FIXME: eof detection
  while (c == 0)
    {
      result->comments.push_back(plinebuf->c_str());
      read_line(&c);		// FIXME: eof detection
    }
  if (c != 'T')
    {
      corrupt(here(), "Expected '@T'");
    }
  /* Sun's Code Manager sometimes emits lines of the form "^AT 0" and
   * so these lines fail the "no argument" check.  So we no longer do
   * that check for "^AT" lines.  A file including lines of this type
   * was provided by Marko Rauhamaa <marko@tekelec.com>.
   */
  /*check_noarg();*/

  auto body_offset = ftell(f_local);
  if (body_offset == -1L)
    {
      errormsg_with_errno("ftell() failed.");
      return nullptr;
    }
  // The body scanner takes ownership of f_local.
  result->body_scanner =
    std::make_unique<sccs_file_body_scanner>(this->name(), f_local,
					     body_offset, here().line_number());
  return result;
}

/* Check for BK comments */
void
sccs_file_parser::check_bk_comment(char ch, char arg) const
{
  ASSERT(is_bk_file_);

  switch (arg)
    {
    case 'B':
    case 'C':
    case 'F':
    case 'H':
    case 'K':
    case 'M':
    case 'O':
    case 'P':
    case 'R':
    case 'S':
    case 'T':
    case 'V':
    case 'X':
    case 'Z':
      return;

    case ' ': // this is the normal SCCS case.
      return;

    default:
      saw_unknown_feature("Unknown special comment intro '%c%c' "
                          "in BitKeeper file\n",
                          ch, arg);
    }
}


/* There are some features that we don't properly understand.
 * If we see them, we should abandon any attempt to modify the
 * file.   We call saw_unknown_feature() when we see one.  It
 * checks what mode we're using the SCCS file in, and reacts
 * accordingly.
 */
void sccs_file_parser::saw_unknown_feature(const char *fmt, ...) const
{
  va_list ap;

  va_start(ap, fmt);

  /* If we are not modifying the file, just issue a warning.  Otherwise,
   * abandon the attempt to edit it.
   */
  switch (mode_)
    {
    case READ:
    case FIX_CHECKSUM:
      v_unknown_feature_warning(fmt, ap);
      break;

    case UPDATE:
    case CREATE:
      s_unrecognised_feature_quit(fmt, ap);
      /*NOTREACHED*/
      break;
    }
}
