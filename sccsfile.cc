/*
 * sccsfile.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999,2001 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Common members of the class sccs_file and its subclasses.  Most of
 * the members in this file are used to read from the SCCS file.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "delta-table.h"
#include "delta-iterator.h"
#include "linebuf.h"
#include "quit.h"
#include "err_no.h"

#ifdef STDC_HEADERS
#include <ctype.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>		// SEEK_SET on SunOS.
#endif

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sccsfile.cc,v 1.45 2001/09/29 19:39:41 james_youngman Exp $";
#endif



/* Static member for opening a SCCS file and then calculating its checksum. */
#define f f_is_also_a_class_member_variable
FILE *
sccs_file::open_sccs_file(const char *name, enum _mode mode, int *sump)
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
  
  if (getc(f_local) != '\001' || getc(f_local) != 'h')
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
    sum += (char) c;	// Yes, I mean plain char, not signed, not unsigned.
  
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
#undef f


/* Reads a line from the SCCS file and increments the current line number.
   If the end of file is reached it returns -1.  If it's a control line
   (it starts with ^A) then the control character (the second character)
   is returned.  Otherwise 0 is returned. */

int
sccs_file::read_line() {
	if (read_line_param(f)) {
		if (ferror(f)) {
			errormsg_with_errno("%s: Read error.", name.c_str());
		}
		return -1;
	} 

	lineno++;
	if ( bufchar(0) == '\001')
	  {
	    return bufchar(1);
	  }
	return 0;
}


/* Quits with a message saying that SCCS file is corrupt. */

NORETURN
sccs_file::corrupt(const char *why) const {
	s_corrupt_quit("%s: line %d: Corrupted SCCS file. (%s)",
		       name.c_str(), lineno, why);
}


/* Checks that a control line has at least one argument. */

void
sccs_file::check_arg() const {
	if (bufchar(2) != ' ') {
		corrupt("Missing arg");
	}
}


/* Checks the a control line has no arguments. */

void
sccs_file::check_noarg() const {
	if (bufchar(2) != '\0') {
		corrupt("Unexpected arg");
	}
}


/* Converts an ASCII string to an unsigned short, quiting if the
   string isn't a valid number. */

unsigned short
sccs_file::strict_atous(const char *s) const
{
  long n = 0;
  
  char c;
  while ( 0 != (c=*s++) )
    {
      if (!isdigit((unsigned char)c))
	{
	  corrupt("Invalid number");
	}
      n = n * 10 + (c - '0');
      if (n > 65535L)
	{
	  corrupt("Number too big");
	}
    }
  
  return (unsigned short) n;
}

// Convert a number field in an SCCS file to a 
// number.  Fields representing numbers in 
// SCCS files should top out at 9999.

unsigned long
sccs_file::strict_atoul(const char *s) const
{
  unsigned long n = 0;
  const unsigned long limit = 99999uL;
  char c;
  while ( 0 != (c=*s++) )
    {
      if (!isdigit((unsigned char)c))
	{
	  corrupt("Invalid number");
	}
      n = n * 10 + (c - '0');
    }
  if (n > limit)
    {
      errormsg("%s: line %d: Warning: number field exceeds %lu.", 
	       name.c_str(), lineno, limit);
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
	
	char *args[7];		/* Stores the result of spliting a line */

	if (plinebuf->split(3, args, 3, '/') != 3)
	  {
	    corrupt("Two /'s expected");
	  }

	// TODO: use constructor here?
	delta tmp;	/* The new delta */

	tmp.inserted = strict_atoul(args[0]);
	tmp.deleted = strict_atoul(args[1]);
	tmp.unchanged = strict_atoul(args[2]);

	if (read_line() != 'd') {
		corrupt("Expected '@d'");
	}

	check_arg();

	plinebuf->split(3, args, 7, ' ');

	tmp.type = args[0][0];
	tmp.id = sid(args[1]);
	tmp.date = sccs_date(args[2], args[3]);
	tmp.user = args[4];
	tmp.seq = strict_atous(args[5]);
	tmp.prev_seq = strict_atous(args[6]);

	if ((tmp.type != 'R' && tmp.type != 'D') || args[0][1] != '\0') {
		corrupt("Bad delta type");
	}
	if (!tmp.id.valid()) {
		corrupt("Bad SID");
	}
	if (!tmp.date.valid()) {
		corrupt("Bad Date/Time");
	}

	/* Read in any lists of included, excluded or ignored seq. no's. */

	int c = read_line();
	int i;
 	const char *start;
	bool bDebug = getenv("CSSC_SHOW_SEQSTATE") ? true : false;
	for(i = 0; i < 3; i++) {
		if (c == "ixg"[i]) {

		  switch (c)
		    {
		    case 'i':
		      tmp.have_includes = true;
		      break;

		    case 'x':
		      tmp.have_excludes = true;
		      break;

		    case 'g':
		      tmp.have_ignores = true;
		      break;
		    }
		  
		  if (bufchar(2) != ' ')
		    {
		      c = read_line(); // throw line away.
		      continue;
		    }
		  	
			check_arg();

			start = plinebuf->c_str() + 3;
			do {
				char *end = strchr(start, ' ');
				if (end != NULL) {
					*end++ = '\0';
				}
				seq_no seq = strict_atous(start);
				switch (c) {
				case 'i':
				  if (bDebug)
				    {
				      fprintf(stderr, "Including seq %lu\n",
					      (unsigned long)seq);
				    }

					tmp.included.add(seq);
					break;

				case 'x':
				  if (bDebug)
				    {
				      fprintf(stderr, "Excluding seq %lu\n",
					      (unsigned long)seq);
				    }
					tmp.excluded.add(seq);
					break;

				case 'g':
					tmp.ignored.add(seq);
					break;
				}
				start = end;
			} while (start != NULL);

			c = read_line();
		}
	}

	// According to Hyman Rosen <hymie@jyacc.com>, it is possible
	// to have a ^A m line which has no argument.  Therefore we don't
	// use check_arg().  

	// According to Hyman Rosen <hymie@jyacc.com>, it is sometimes
	// possible to have ^Am lines after ^Ac lines, as well as the
	// more usual before.  Hence we now cope with both.
	while (c == 'm')
	  {
	    if (bufchar(2) == ' ')
	      {
		tmp.mrs.add(plinebuf->c_str() + 3);
	      }
		c = read_line();
	  }

	while (c == 'c') {
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
	  /* check_arg(); */
		tmp.comments.add(plinebuf->c_str() + 3);
		c = read_line();
	}

	// According to Hyman Rosen <hymie@jyacc.com>, it is sometimes
	// possible to have ^Am lines after ^Ac lines, as well as the
	// more usual before.  Hence we now cope with both.
	while (c == 'm')
	  {
	    if (bufchar(2) == ' ')
	      {
		tmp.mrs.add(plinebuf->c_str() + 3);
	      }
		c = read_line();
	  }

	if (c != 'e') {
		corrupt("Expected '@e'");
	}

	check_noarg();

	ASSERT(0 != delta_table);
	delta_table->add(tmp);
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
  lineno = body_lineno;
  return true;
}

bool sccs_file::checksum_ok() const
{
  return checksum_valid;
}


/* Returns the module name of the SCCS file. */

mystring
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
  : name(n), mode(m), lineno(0), xfile_created(false)
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
  flags.floor = 0;
  flags.ceiling = 0;
  flags.default_sid = NULL;	// (XXX: correct use of NULL?) 
  flags.null_deltas = 0;
  flags.joint_edit = 0;
  flags.all_locked = 0;
  flags.encoded = 0;
  flags.mr_checker = 0;
  flags.module = 0;
  flags.type = 0;
  flags.reserved = 0;
  flags.user_def = 0;
  
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
      return;
    }

  // Even if we are going to change the s-file, we do it by writing
  // a new x-file and then renaming it.   This means that we open
  // the s-file read-only.
  signed int sum = 0;
  f = open_sccs_file(name.c_str(), READ, &sum);
  
  /* open_sccs_file() returns normally if everything went OK, or if 
   * there was an IO error on an apparently valid file.  If this is 
   * the case, perror() will already have been called.
   */
  if (NULL == f)
    {
      ctor_fail(-1, "%s: Cannot open SCCS file.\n", name.c_str());
    }
  
  int c = read_line();
  ASSERT(c == 'h');

  /* the checksum is represented in the file as decimal.
   */
  signed int given_sum = 0;
  if (1 != sscanf(plinebuf->c_str(), "%*ch%d", &given_sum))
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
	      errormsg("%s: Warning: bad checksum "
		       "(expected=%d, calculated %d).\n",
		       name.c_str(), given_sum, sum);
	    }
	}
    }
  if (!checksum_valid)
    {
      // Todo: throw exception here?
    }
  
  c = read_line();
  while (c == 's')
    {
      read_delta();
      c = read_line();
    }
  
  if (c != 'u')
    {
      corrupt("Expected '@u'");
    }
  
  check_noarg();
  
  c = read_line();
  while (c != 'U')
    {
      if (c != 0)
	{
	  corrupt("User name expected.");
	}
      users.add(plinebuf->c_str());
      c = read_line();
    }

  /* Sun's Code Manager sometimes emits lines of the form "^AU 0" and
   * so these lines fail the "no argument" check.  So we no longer do
   * that check for "^AU" lines.  A file including lines of this type
   * was provided by Marko Rauhamaa <marko@tekelec.com>.
   */
  /* check_noarg(); */
  
  c = read_line();
  while (c == 'f')
    {
      check_arg();

      if (bufchar(3) == '\0'
	  || (bufchar(4) != '\0' && bufchar(4) != ' '))
	{
	  corrupt("Bad flag arg.");
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
	    corrupt("Bad 'f' flag");
	  }
	break;
	
      case 'c':
	flags.ceiling = release(arg);
	if (!flags.ceiling.valid())
	  {
	    corrupt("Bad 'c' flag");
	  }
	break;

      case 'd':
	flags.default_sid = sid(arg);
	if (!flags.default_sid.valid())
	  {
	    corrupt("Bad 'd' flag");
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
      case 'e':
	if (got_arg && '1' == *arg)
	  flags.encoded = 1;
	else if (got_arg && '0' == *arg)
	  flags.encoded = 0;
	else
	  corrupt("Bad value for 'e' flag.");
	break;
	
      default:
	corrupt("Unknown flag.");
      }
      
      c = read_line();
    }
  
  if (c != 't')
    {
      corrupt("Expected '@t'");
    }
  
  /* Sun's Code Manager sometimes emits lines of the form "^At 0" and
   * so these lines fail the "no argument" check.  So we no longer do
   * that check for "^At" lines.  A file including lines of this type
   * was provided by Marko Rauhamaa <marko@tekelec.com>.
   */
  /*check_noarg();*/
  
  c = read_line();
  while (c == 0)
    {
      comments.add(plinebuf->c_str());
      c = read_line();
    }
  
  if (c != 'T')
    {
      corrupt("Expected '@T'");
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
  
  body_lineno = lineno;
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
		if (id.trunk_match(iter->id)) {
			if (found.is_null() || newest < iter->date) {
				newest = iter->date;
				found = iter->id;
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
      if (!found || iter->date > d)
	{
	  d = iter->date;
	  s = iter->id;
	  found = true;
	}
    }
  return found;
}

void sccs_file::
set_mr_checker_flag(const char *s)
{
  if (flags.mr_checker)
    *flags.mr_checker = s;
  else
    flags.mr_checker = new mystring(s);
}

void sccs_file::
set_module_flag(const char *s)
{
  if (flags.module)
    *flags.module = s;
  else
    flags.module = new mystring(s);
}

void  sccs_file::
set_user_flag(const char *s)
{
  if (flags.user_def)
    *flags.user_def = s;
  else
    flags.user_def = new mystring(s);
}

void sccs_file::
set_type_flag(const char *s)
{
  if (flags.type)
    *flags.type = s;
  else
    flags.type = new mystring(s);
}

void sccs_file::
set_reserved_flag(const char *s)
{
  if (flags.reserved)
    *flags.reserved = s;
  else
    flags.reserved = new mystring(s);
}



int
sccs_file::read_line_param(FILE *f)
{
  if (plinebuf->read_line(f))
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
  return (d != 0) && (strcmp(d->user.c_str(), user) == 0);
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
  return delta_table->delta_at_seq(seq).id;
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
      ASSERT(0 != f);		// catch multiple destruction.
      fclose(f);
      f = 0;
    }

  if (xfile_created)
    {
      remove(name.xfile().c_str());
    }
  
  ASSERT(0 != delta_table); 	// catch multiple destruction.
  delete delta_table;
  delta_table = 0;
  
  ASSERT(0 != plinebuf); 	// catch multiple destruction.
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

  
/* Local variables: */
/* mode: c++ */
/* End: */
