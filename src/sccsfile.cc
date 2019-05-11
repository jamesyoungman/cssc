/*
 * sccsfile.cc: Part of GNU CSSC.
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
 *
 *
 * Common members of the class sccs_file and its subclasses.  Most of
 * the members in this file are used to read from the SCCS file.
 *
 */
#include <config.h>
#include <errno.h>
#include <string>
#include <sstream>

#include "cssc.h"
#include "sccsfile.h"
#include "delta-table.h"
#include "delta-iterator.h"
#include "linebuf.h"
#include "quit.h"
#include "ioerr.h"
#include "file.h"

#include <ctype.h>
#include <unistd.h>             // SEEK_SET on SunOS.
#include <sys/stat.h>           /* fstat(), struct stat */


string sccs_file_location::as_string() const
{
  std::stringstream s;
  s << lineno_;
  return s.str();
}

#if defined HAVE_FILENO && defined HAVE_FSTAT
/* If an SCCS file has a link count greater than one, then the normal
 * process of updating the file will break the link.  We try to detect this
 * even if the file is being opened to reading only, to give an early
 * warning (and because SCCS does so).
 */
static int just_one_link(FILE *f)
{
  int fd = fileno(f);
  if (fd >= 0)
    {
      struct stat st;
      if (0 != fstat(fd, &st))
        {
          /* We cannot stat the file descriptor.  Perhaps there is a
           * file system functionality issue.   If that's the case then we
           * will give it the benefit of the doubt on the link coutn front.
           */
          return 1;  /* We're happy with the file */
        }
      if (st.st_nlink > 1)
        return 0;               /* We don't like it. */
    }
  return 1;                     /* OK. */
}
#else
static int just_one_link(FILE *f)
{
  /* Without fileno(), we have no way of checking. */
  return 1;
}
#endif


/* Static member for opening a SCCS file and then calculating its checksum. */
FILE *
sccs_file::open_sccs_file(const char *name,
                          enum _mode mode,
                          int *sump,
                          bool* isBKFile)
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
      return NULL;
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
      return NULL;
    }

  bool badMagic = false;
  if (getc(f_local) != '\001')
    {
      badMagic = true;
    }
  else
    {
      char magicMarker = getc(f_local);
      if (magicMarker == 'H')
        {
          if (READ == mode)
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
              return NULL;
            }
          // Inform the caller that this is a BK file.
          // NB: this is the parameter, not member variable
          *isBKFile = true;
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
      return NULL;
    }


  int c;
  errno = 0;
  while ( (c=getc(f_local)) != CONFIG_EOL_CHARACTER)
    {
      if (EOF == c)
        {
          const int saved_errno = errno;
          (void)fclose(f_local);
          errno = saved_errno;
          if (errno)
            {
              perror(name);
            }
          else
            {
              s_corrupt_quit("%s: Unexpected EOF.", name);
              /*NOTEACHED*/
            }
          return NULL;
        }
    }

  int sum = 0u;

  while ((c=getc(f_local)) != EOF)
    sum += (char) c;    // Yes, I mean plain char, not signed, not unsigned.

  if (ferror(f_local))
    {
      perror(name);
      (void)fclose(f_local);
      return NULL;
    }


  *sump = sum & 0xFFFFu;

#ifdef CONFIG_OPEN_SCCS_FILES_IN_BINARY_MODE
  fclose(f_local);
  if (mode == UPDATE)
    f_local = fopen(name, "r+");
  else
    f_local = fopen(name, "r");

  if (NULL == f_local)
    {
      perror(name);
      return NULL;
    }

#else
  rewind(f_local);
  if (ferror(f_local))
    {
      perror(name);
      (void)fclose(f_local);
      return NULL;
    }
#endif
  return f_local;
}



/*
 * Reads a line from the SCCS file.
 * Result:
 *   true if we read a line.   false for EOF or failure.
 * Output params:
 *   control_char: 0 if this is not a control (^A) line, otherwise the line type.
 */
bool
sccs_file::read_line(char* line_type)
{
  if (read_line_param(f))
    {
      if (ferror(f))
	{
	  errormsg_with_errno("%s: Read error.", name.c_str());
	}
      return false;
    }

  here_.advance_line();
  if ( bufchar(0) == '\001')
    *line_type = bufchar(1);
  else
    *line_type = char(0);
  return true;
}


/* Quits with a message saying that SCCS file is corrupt. */

NORETURN
sccs_file::corrupt(const sccs_file_location& loc, const char *fmt, ...) const {
  char buf[80];
  const char *p;

  va_list ap;
  va_start(ap, fmt);
  if (-1 == vsnprintf(buf, sizeof(buf), fmt, ap))
    {
      warning("%s: error message too long for buffer, so the "
              "next message will lack some relevant detail",
              name.c_str());
      p = fmt;                  // best effort
    }
  else
    {
      p = buf;
    }
  s_corrupt_quit("%s: line %d: Corrupted SCCS file. (%s)",
                 name.c_str(), loc.line_number(), p);
}

NORETURN
sccs_file::corrupt_file(const char *fmt, ...) const {
  char buf[80];
  const char *p;

  va_list ap;
  va_start(ap, fmt);
  if (-1 == vsnprintf(buf, sizeof(buf), fmt, ap))
    {
      warning("%s: error message too long for buffer, so the "
              "next message will lack some relevant detail",
              name.c_str());
      p = fmt;                  // best effort
    }
  else
    {
      p = buf;
    }
  s_corrupt_quit("%s: Corrupted SCCS file. (%s)",
                 name.c_str(), p);
}


/* Checks that a control line has at least one argument. */

void
sccs_file::check_arg() const {
        if (bufchar(2) != ' ') {
	    corrupt(here(), "Missing arg");
        }
}


/* Checks the a control line has no arguments. */

void
sccs_file::check_noarg() const {
        if (bufchar(2) != '\0') {
	    corrupt(here(), "Unexpected arg");
        }
}


/* Converts an ASCII string to an unsigned short, quiting if the
   string isn't a valid number. */

unsigned short
sccs_file::strict_atous(const sccs_file_location& loc, const char *s) const
{
  long n = 0;

  char c;
  while ( 0 != (c=*s++) )
    {
      if (!isdigit((unsigned char)c))
        {
	  corrupt(loc, "Invalid number");
        }
      n = n * 10 + (c - '0');
      if (n > 65535L)
        {
	  corrupt(loc, "Number too big");
        }
    }

  return (unsigned short) n;
}

// Convert a number field in an SCCS file to a
// number.  Fields representing numbers in
// SCCS files should top out at 9999.

unsigned long
sccs_file::strict_atoul_idu(const sccs_file_location& loc, const char *s) const
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
              name.c_str());
      if ((UPDATE == mode) || (FIX_CHECKSUM == mode))
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
              name.c_str(), loc.as_string().c_str(), limit);
    }

  return n;
}

/* Reads a delta from the SCCS file's delta table and adds it to the
   delta table. */

void
sccs_file::read_delta() {

        /* The current line should be an 's' control line */

        ASSERT(bufchar(1) == 's');
        check_arg();

        char *args[7];          /* Stores the result of spliting a line */

        if (plinebuf->split(3, args, 3, '/') != 3)
          {
            corrupt(here(), "Two /'s expected");
          }

        // TODO: use constructor here?
        delta tmp;      /* The new delta */
        tmp.set_idu(strict_atoul_idu(here(), args[0]),
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
            tmp.set_type(args[0][0]);
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
	tmp.set_id(newid);

        auto newdate = sccs_date(args[2], args[3]);
        if (!newdate.valid())
	  {
	    corrupt(here(), "Bad Date/Time %s %s", args[2], args[3]);
	  }
        tmp.set_date(newdate);

        tmp.set_user(args[4]);
        tmp.set_seq(strict_atous(here(), args[5]));
        tmp.set_prev_seq(strict_atous(here(), args[6]));

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
                      tmp.set_has_includes(true);
                      break;

                    case 'x':
                      {
                        warning("feature not fully tested: "
                                "excluded delta in SID %s ",
                                tmp.id().as_string().c_str());
                        tmp.set_has_excludes(true);
                      }
                      break;

                    case 'g':
                      tmp.set_has_ignores(true);
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

                                        tmp.add_include(seq);
                                        break;

                                case 'x':
                                  if (bDebug)
                                    {
                                      fprintf(stderr, "Excluding seq %lu\n",
                                              (unsigned long)seq);
                                    }
                                        tmp.add_exclude(seq);
                                        break;

                                case 'g':
                                        tmp.add_ignore(seq);
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
                    tmp.add_mr(plinebuf->c_str() + 3);
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
                if (is_bk_file)
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
                tmp.add_comment(plinebuf->c_str() + 3);
              }

            read_line(&c);	// FIXME: check for EOF
          }


        if (c != 'e') {
	  corrupt(here(), "Expected '@e'");
        }

        check_noarg();

        ASSERT(0 != delta_table);
        delta_table->add(tmp);
}


/* Check for BK flags */
void
sccs_file::check_bk_flag(const sccs_file_location& loc, char flagchar) const
{
  switch (flagchar)
    {
    case 'x':
      return;

    default:
      corrupt(loc, "Unknown flag '%c'.", flagchar);
    }
}


/* Find out if it is OK to change the file - called by cdc, rmdel, get -e
 */
bool
sccs_file::edit_mode_ok(bool editing) const
{
  if (editing && is_bk_file)
    {
      errormsg("%s: This is a BitKeeper file.  Checking BitKeeper files out "
               "for editing (or otherwise modifying them) is not supported "
               "at the moment, sorry.\n",
               name.c_str());
      return false;
    }
  else
    {
      return true;
    }
}


/* Check for BK comments */
void
sccs_file::check_bk_comment(char ch, char arg) const
{
  ASSERT(is_bk_file);

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


/* Seeks on the SCCS file to the start of the body.  This function
   may be rewritten as fseek() doesn't always work too well on
   text files. */
// JAY: use fgetpos()/fsetpos() instead?
bool
sccs_file::seek_to_body()
{
  if (fseek(f, body_offset, SEEK_SET) != 0)
    {
      // this quit should NOT be fatal; we should proceed
      // to the next file if we can.
      errormsg("%s: fseek() failed!", name.c_str());
      return false;
    }
  here_ = body_location;
  return true;
}

bool sccs_file::checksum_ok() const
{
  return checksum_valid;
}


/* Returns the module name of the SCCS file. */

std::string
sccs_file::get_module_name() const
{
  if (flags.module)
    return *flags.module;
  else
    return name.gfile();
}

/* Constructor for the class sccs_file.  Unless the SCCS file is being
   created it reads in the all but the body of the file.  The file is
   locked if it isn't only being read.  */

sccs_file::sccs_file(sccs_name &n, enum _mode m)
  : name(n), f(0), mode(m), xfile_created(false), is_bk_file(false)
{
  delta_table = new cssc_delta_table;
  plinebuf     = new cssc_linebuf;
  ASSERT(0 != delta_table);

  if (!name.valid())
    {
      ctor_fail(-1,
                "%s: Not an SCCS file.  Did you specify the right file?",
                name.c_str());
    }

  flags.no_id_keywords_is_fatal = 0;
  flags.branch = 0;
  flags.floor = (short)0;
  flags.ceiling = (short)0;
  flags.default_sid = sid::null_sid();
  flags.null_deltas = 0;
  flags.joint_edit = 0;
  flags.all_locked = 0;
  flags.encoded = 0;
  flags.executable = 0;
  flags.mr_checker = 0;
  flags.module = 0;
  flags.type = 0;
  flags.reserved = 0;
  flags.user_def = 0;

  ASSERT(!flags.default_sid.valid());

  if (mode != READ)
    {
      if (name.lock())
        {
          ctor_fail(-1, "%s: SCCS file is locked.  Try again later.",
               name.c_str());
        }
    }

  if (mode == CREATE)
    {
      /* f is NULL in this case. */
      return;
    }

  // Even if we are going to change the s-file, we do it by writing
  // a new x-file and then renaming it.   This means that we open
  // the s-file read-only.
  signed int sum = 0;
  f = open_sccs_file(name.c_str(), READ, &sum, &is_bk_file);

  if (mode != READ)
    {
      if (!edit_mode_ok(true))
        {
          ctor_fail(-1,
                    "%s: Editing is not supported for BitKeeper files.\n",
                    name.c_str());
        }
    }

  /* open_sccs_file() returns normally if everything went OK, or if
   * there was an IO error on an apparently valid file.  If this is
   * the case, perror() will already have been called.
   */
  if (NULL == f)
    {
      ctor_fail(-1, "%s: Cannot open SCCS file.\n", name.c_str());
    }

  // If the history file is executable, remember this fact.
  bool is_executable;
  if (get_open_file_xbits(f, &is_executable))
    {
      set_sfile_executable (is_executable);
    }

  char c;
  read_line(&c);		// FIXME: check for EOF

  // open_sccs_file() should have already checked that the first line
  // is ^Ah or ^Ah, so this assertion is really just checking that
  // open_sccs_file() did the right thing.
  if (is_bk_file)
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
		  (is_bk_file ? "%*cH%d" : "%*ch%d"),
		  &given_sum))
    {
      errormsg("Expected checksum line, found line beginning '%.3s'\n",
               plinebuf->c_str());
      checksum_valid = false;
    }
  else
    {
      given_sum &= 0xFFFFu;
      checksum_valid = (given_sum == sum);

      if (false == checksum_valid)
        {
          if (FIX_CHECKSUM == mode)
            {
              // This supports the -z option of admin.
              checksum_valid = true;
            }
          else
            {
              warning("%s: bad checksum "
                      "(expected=%d, calculated %d).\n",
                      name.c_str(), given_sum, sum);
            }
        }
    }
  if (!checksum_valid)
    {
      // Todo: throw exception here?
    }

  read_line(&c);		// FIXME: detect eof
  while (c == 's')
    {
      read_delta();
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
      users.push_back(plinebuf->c_str());
      read_line(&c);		// FIXME: detect eof
    }

  /* Sun's Code Manager sometimes emits lines of the form "^AU 0" and
   * so these lines fail the "no argument" check.  So we no longer do
   * that check for "^AU" lines.  A file including lines of this type
   * was provided by Marko Rauhamaa <marko@tekelec.com>.
   */
  /* check_noarg(); */

  read_line(&c);		// FIXME: detect eof
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
      const char *arg = 0;
      bool got_arg = false;
      if (bufchar(4) == ' ')
        {
          arg = plinebuf->c_str() + 5;
          got_arg = true;
        }
      else
        {
          arg = "";
        }

      switch (bufchar(3)) {
      case 't':
        set_type_flag(arg);
        break;

      case 'v':
        set_mr_checker_flag(arg);
        break;

      case 'i':
        flags.no_id_keywords_is_fatal = 1;
        break;

      case 'b':
        flags.branch = 1;
        break;

      case 'm':
        set_module_flag(arg);
        break;

      case 'f':
        flags.floor = release(arg);
        if (!flags.floor.valid())
          {
            corrupt(here(), "Bad 'f' flag argument %s", arg);
          }
        break;

      case 'c':
        flags.ceiling = release(arg);
        if (!flags.ceiling.valid())
          {
            corrupt(here(), "Bad 'c' flag argument %s", arg);
          }
        break;

      case 'd':
        flags.default_sid = sid(arg);
        if (!flags.default_sid.valid())
          {
            corrupt(here(), "Bad 'd' flag argument %s", arg);
          }
        break;

      case 'n':
        flags.null_deltas = 1;
        break;

      case 'j':
        flags.joint_edit = 1;
        break;

      case 'l':
        if (got_arg && strcmp(arg, "a") == 0)
          {
            flags.all_locked = 1;
          }
        else
          {
            flags.locked = release_list(arg);
          }
        break;

      case 'q':
        set_user_flag(arg);
        break;

      case 'z':
        set_reserved_flag(arg);
        break;

      case 'x':
        // The 'x' flag is supported by SCO's version of SCCS.
        // When this flag is set, the g-file is marked executable.
        // The g-file is also executable when the s-file is executable
        // (to follow the example of Solaris).
        flags.executable = 1;
        break;

      case 'y':
        // The 'y' flag is supported by Solaris 8 and above.
        // It controls the expansion of '%' keywords.  If the
        // y flag is set, its value is a list of keywords that will
        // be expanded.  Otherwise, all known keywords will be expanded.
        set_expanded_keyword_flag(arg);
        break;

      case 'e':
        if (got_arg && '1' == *arg)
          flags.encoded = 1;
        else if (got_arg && '0' == *arg)
          flags.encoded = 0;
        else
          corrupt(here(), "Bad value '%c' for 'e' flag.", arg[0]);
        break;

      default:
        if (is_bk_file)
          {
            check_bk_flag(here(), bufchar(3));
          }
        else
          {
            corrupt(here(), "Unknown flag '%c'.", bufchar(3));
          }
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
      comments.push_back(plinebuf->c_str());
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

  body_offset = ftell(f);
  if (body_offset == -1L)
    {
      ctor_fail(errno, "ftell() failed.");
    }

  body_location = here_;
}


/* Find the SID of the most recently created delta with the same release
   and level as the requested SID. */

sid
sccs_file::find_most_recent_sid(sid id) const {
        sccs_date newest;
        sid found;

        ASSERT(0 != delta_table);
        const_delta_iterator iter(delta_table);

        while (iter.next()) {
          if (id.trunk_match(iter->id())) {
                        if (found.is_null() || newest < iter->date()) {
                                newest = iter->date();
                                found = iter->id();
                        }
                }
        }
        return found;
}

bool
sccs_file::find_most_recent_sid(sid& s, sccs_date& d) const
{
  s = sid();
  d = sccs_date();
  bool found = false;

  ASSERT(0 != delta_table);

  const_delta_iterator iter(delta_table);
  while (iter.next())
    {
      if (!found || iter->date() > d)
        {
          d = iter->date();
          s = iter->id();
          found = true;
        }
    }
  return found;
}

void sccs_file::
set_mr_checker_flag(const char *s)
{
  if (flags.mr_checker)
    delete flags.mr_checker;

  flags.mr_checker = new std::string(s);
}

void sccs_file::
set_module_flag(const char *s)
{
  if (flags.module)
    delete flags.module;

  flags.module = new std::string(s);
}

void  sccs_file::
set_user_flag(const char *s)
{
  if (flags.user_def)
    delete flags.user_def;

  flags.user_def = new std::string(s);
}

void sccs_file::
set_type_flag(const char *s)
{
  if (flags.type)
    delete flags.type;

  flags.type = new std::string(s);
}

void sccs_file::
set_reserved_flag(const char *s)
{
  if (flags.reserved)
    delete flags.reserved;

  flags.reserved = new std::string(s);
}


void sccs_file::
set_expanded_keyword_flag(const char *s)
{
  const size_t len = strlen(s);
  std::set<char> tmp_letters(s, s + len);
  std::swap(tmp_letters, flags.substitued_flag_letters);
}

int
sccs_file::read_line_param(FILE *fp)
{
  if (plinebuf->read_line(fp))
    {
      return 1;
    }
  // chomp the newline from the end of the line.
  // TODO: make me 8-bit clean!
  (*plinebuf)[strlen(plinebuf->c_str()) - 1] = '\0';
  return 0;
}

int
sccs_file::is_delta_creator(const char *user, sid id) const
{
  const delta *d = find_delta(id);
  return (d != 0) && (strcmp(d->user().c_str(), user) == 0);
}


const delta * sccs_file::find_delta(sid id) const
{
  ASSERT(0 != delta_table);
  return delta_table->find(id);
}

const delta * sccs_file::find_any_delta(sid id) const
{
  ASSERT(0 != delta_table);
  return delta_table->find_any(id);
}

delta * sccs_file::find_delta(sid id)
{
  ASSERT(0 != delta_table);
  return delta_table->find(id);
}

seq_no sccs_file::highest_delta_seqno() const
{
  ASSERT(0 != delta_table);
  return delta_table->highest_seqno();
}

sid sccs_file::highest_delta_release() const
{
  ASSERT(0 != delta_table);
  return delta_table->highest_release();
}

sid sccs_file::seq_to_sid(seq_no seq) const
{
  ASSERT(0 != delta_table);
  return delta_table->delta_at_seq(seq).id();
}


/* Destructor for class sccs_file. */

sccs_file::~sccs_file()
{
  if (mode != READ)
    {
      name.unlock();
    }

  if (mode != CREATE)
    {
      ASSERT(0 != f);           // catch multiple destruction.
      fclose(f);
      f = 0;
    }

  if (xfile_created)
    {
      remove(name.xfile().c_str());
    }

  ASSERT(0 != delta_table);     // catch multiple destruction.
  delete delta_table;
  delta_table = 0;

  ASSERT(0 != plinebuf);        // catch multiple destruction.
  delete plinebuf;
  plinebuf = 0;
}


char sccs_file::bufchar(int pos) const
{
  return (*plinebuf)[pos];
}


bool sccs_file::branches_allowed() const
{
  return 0 != flags.branch;
}

/* There are some features that we don't properly understand.
 * If we see them, we should abandon any attempt to modify the
 * file.   We call saw_unknown_feature() when we see one.  It
 * checks what mode we're using the SCCS file in, and reacts
 * accordingly.
 */
void sccs_file::saw_unknown_feature(const char *fmt, ...) const
{
  va_list ap;

  va_start(ap, fmt);

  /* If we are not modifying the file, just issue a warning.  Otherwise,
   * abandon the attempt to edit it.
   */
  switch (mode)
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

bool sccs_file::
print_subsituted_flags_list(FILE *out, const char* separator) const
{
  bool first = true;
  for (auto flagletter : flags.substitued_flag_letters)
    {
      if (!first)
        {
	  // A space separator is required.
          if (printf_failed(fprintf(out, "%s", separator)))
            return false;
        }
      first = false;

      // print the keyword letter.
      if (printf_failed(fprintf(out, "%c", flagletter)))
        return false;
    }
  return true;
}

bool sccs_file::
is_known_keyword_char(char c)
{
  return strchr("MIRLBSDHTEGUYFPQCZWA", c) != NULL;
}

void sccs_file::
set_sfile_executable(bool state)
{
  sfile_executable = state;
}

bool sccs_file::
gfile_should_be_executable() const
{
  return sfile_executable || flags.executable;
}


bool sccs_file::
sfile_should_be_executable() const
{
  return sfile_executable;
}



/* Local variables: */
/* mode: c++ */
/* End: */
