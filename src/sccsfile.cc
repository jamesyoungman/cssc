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


/* Find out if it is OK to change the file - called by cdc, rmdel, get -e
 */
cssc::Failure
sccs_file::edit_mode_permitted(bool editing) const
{
  if (editing && !edit_mode_ok_)
    {
      return cssc::make_failure_builder(cssc::errorcode::CannotEditBitkeeperFile)
	<< name.c_str() << " is a BitKeeper file.  Checking BitKeeper files out "
	<< "for editing (or otherwise modifying them) is not supported "
	<< "at the moment, sorry.";
    }
  return cssc::Failure::Ok();
}



bool sccs_file::checksum_ok() const
{
  return checksum_valid_;
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

sccs_file::sccs_file(sccs_name &n, sccs_file_open_mode m,
		     ParserOptions opts)
  : flags(),
    name(n), checksum_valid_(false), mode(m), xfile_created(false), edit_mode_ok_(true),
    sfile_executable(false),
    delta_table(std::make_unique<cssc_delta_table>()),
    body_scanner_(), users(), comments()
{
  if (!name.valid())
    {
      ctor_fail(-1,
                "%s: Not an SCCS file.  Did you specify the right file?",
                name.c_str());
    }

  flags.no_id_keywords_is_fatal = 0;
  flags.branch = 0;
  flags.floor = release();
  flags.ceiling = release();
  flags.default_sid = sid::null_sid();
  flags.null_deltas = 0;
  flags.joint_edit = 0;
  flags.all_locked = 0;
  flags.encoded = 0;
  flags.executable = 0;
  flags.mr_checker = nullptr;
  flags.module = nullptr;
  flags.type = nullptr;
  flags.reserved = nullptr;
  flags.user_def = nullptr;

  ASSERT(!flags.default_sid.valid());

  if (mode != READ)
    {
      auto locked = name.lock();
      if (!locked.ok())
        {
          ctor_fail(-1, "%s: SCCS file is locked (%s).  Try again later.",
		    name.c_str(), locked.to_string().c_str());
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
  if (mode == FIX_CHECKSUM)
    {
      ParserOptions new_opts;
      new_opts = opts;
      new_opts.set_silent_checksum_error(true);
      opts = new_opts;
    }
  auto failure_or_opened = sccs_file_parser::open_sccs_file(name.sfile(), READ, opts);
  if (!failure_or_opened.ok())
    {
      ctor_fail(-1, "%s: Cannot open SCCS file.\n", name.c_str());
    }
  auto opened = std::move(*failure_or_opened);
  // We do not support edit operations on Bitkeeper files.
  edit_mode_ok_ = !opened->is_bk;
  set_sfile_executable (opened->is_executable);

  body_scanner_ = std::move(opened->body_scanner);

  if (mode != READ)
    {
      cssc::Failure edit_permitted = edit_mode_permitted(true);
      if (!edit_permitted.ok())
        {
          ctor_fail(-1, "%s", edit_permitted.to_string().c_str());
        }
    }
  if (FIX_CHECKSUM == mode)
    {
      // This supports the -z option of admin.
      checksum_valid_ = true;
    }
  else
    {
      checksum_valid_ = opened->checksum_valid_;
    }
  delta_table = std::move(opened->delta_table);
  std::swap(opened->users, users);

  for (const auto& setting : opened->flags)
    {
      const bool got_arg = setting.value.has_value();
      const char *arg = nullptr;
      if (got_arg)
	{
	  arg = setting.value.value().c_str();
	}
      switch (setting.letter) {
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
	    corrupt(setting.where, "Bad 'f' flag argument %s", arg);
	  }
	break;

      case 'c':
	flags.ceiling = release(arg);
	if (!flags.ceiling.valid())
	  {
	    corrupt(setting.where, "Bad 'c' flag argument %s", arg);
	  }
	break;

      case 'd':
	flags.default_sid = sid(arg);
	if (!flags.default_sid.valid())
	  {
	    corrupt(setting.where, "Bad 'd' flag argument %s", arg);
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
	//
	// Bitkeeper also uses the 'x' flag but since we only access
	// Bitkeeper files read-only, not doing anything about that
	// should be benign (except that our gotten files will be
	// executable).
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
	  corrupt(setting.where, "Bad value '%c' for 'e' flag.", arg[0]);
	break;

      default:
	corrupt(setting.where, "Unknown flag '%c'.", setting.letter);
	break;
      }
    }
  std::swap(comments, opened->comments);
}


/* Find the SID of the most recently created delta with the same release
   and level as the requested SID. */

sid
sccs_file::find_most_recent_sid(sid id) const {
        sccs_date newest;
        sid found;

        ASSERT(nullptr != delta_table);
        const_delta_iterator iter(delta_table.get(), delta_selector::current);

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

  ASSERT(nullptr != delta_table);

  const_delta_iterator iter(delta_table.get(), delta_selector::current);
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

void
sccs_file::set_mr_checker_flag(const char *s)
{
  if (flags.mr_checker)
    delete flags.mr_checker;

  flags.mr_checker = new std::string(s);
}

void
sccs_file::set_module_flag(const char *s)
{
  if (flags.module)
    delete flags.module;

  flags.module = new std::string(s);
}

void
sccs_file::set_user_flag(const char *s)
{
  if (flags.user_def)
    delete flags.user_def;

  flags.user_def = new std::string(s);
}

void
sccs_file::set_type_flag(const char *s)
{
  if (flags.type)
    delete flags.type;

  flags.type = new std::string(s);
}

void
sccs_file::set_reserved_flag(const char *s)
{
  if (flags.reserved)
    delete flags.reserved;

  flags.reserved = new std::string(s);
}


void sccs_file::set_expanded_keyword_flag(const char *s)
{
  const size_t len = strlen(s);
  std::set<char> tmp_letters(s, s + len);
  std::swap(tmp_letters, flags.substitued_flag_letters);
}

bool
sccs_file::is_delta_creator(const char *user, sid id) const
{
  const delta *d = find_delta(id);
  return (d != nullptr) && (strcmp(d->user().c_str(), user) == 0);
}


const delta * sccs_file::find_delta(sid id) const
{
  ASSERT(nullptr != delta_table);
  return delta_table->find(id);
}

const delta * sccs_file::find_any_delta(sid id) const
{
  ASSERT(nullptr != delta_table);
  return delta_table->find_any(id);
}

delta * sccs_file::find_delta(sid id)
{
  ASSERT(nullptr != delta_table);
  return delta_table->find(id);
}

seq_no sccs_file::highest_delta_seqno() const
{
  ASSERT(nullptr != delta_table);
  return delta_table->highest_seqno();
}

sid sccs_file::highest_delta_release() const
{
  ASSERT(nullptr != delta_table);
  return delta_table->highest_release();
}

sid sccs_file::seq_to_sid(seq_no seq) const
{
  ASSERT(nullptr != delta_table);
  return delta_table->delta_at_seq(seq).id();
}


/* Destructor for class sccs_file. */

sccs_file::~sccs_file()
{
  if (mode != READ)
    {
      name.unlock();
    }

  if (xfile_created)
    {
      remove(name.xfile().c_str());
    }
}


bool sccs_file::branches_allowed() const
{
  return 0 != flags.branch;
}

cssc::Failure
sccs_file::print_subsituted_flags_list(FILE *out, const char* separator) const
{
  bool first = true;
  for (auto flagletter : flags.substitued_flag_letters)
    {
      if (!first)
        {
	  // A space separator is required.
          if (printf_failed(fprintf(out, "%s", separator)))
            return cssc::make_failure_from_errno(errno);
        }
      first = false;

      // print the keyword letter.
      if (printf_failed(fprintf(out, "%c", flagletter)))
	return cssc::make_failure_from_errno(errno);
    }
  return cssc::Failure::Ok();
}

bool
sccs_file::is_known_keyword_char(char c)
{
  return strchr("MIRLBSDHTEGUYFPQCZWA", c) != NULL;
}

void
sccs_file::set_sfile_executable(bool state)
{
  sfile_executable = state;
}

bool
sccs_file::gfile_should_be_executable() const
{
  return sfile_executable || flags.executable;
}


bool
sccs_file::sfile_should_be_executable() const
{
  return sfile_executable;
}

sccs_file::sccs_file_flags::sccs_file_flags()
  : type(nullptr), mr_checker(nullptr), no_id_keywords_is_fatal(false),
    branch(0), module(nullptr), floor(), ceiling(), default_sid(),
    null_deltas(), joint_edit(), locked(), all_locked(),
    user_def(nullptr), reserved(nullptr),
    encoded(0), executable(0), substitued_flag_letters()
{
}

/* Local variables: */
/* mode: c++ */
/* End: */
