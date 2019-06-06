/*
 * fileiter.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 2007, 2008, 2008, 2009, 2010, 2011, 2014, 2019
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
 * Members of the class fileiter.
 *
 */
#include <config.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "cssc.h"
#include "cleanup.h"
#include "sccsname.h"
#include "fileiter.h"
#include "linebuf.h"
#include "my-getopt.h"
#include "file.h"
#include "quit.h"
#include "dirent-safer.h"

namespace
{

  std::vector<std::string> from_directory(const std::string& passed_dir_name, DIR * dir)
  {
    std::vector<std::string> result;
    const std::string slash((passed_dir_name.back() != '/') ? "/" : "");
    const std::string dirname = passed_dir_name + slash;

    unsigned long entry_error_count = 0;
    int sample_entry_errno = 0;
    for (;;)
      {
	errno = 0;
	struct dirent *dent = readdir(dir);
	if (dent == nullptr)
	  {
	    if (errno == 0)
	      {
		break; // end of directory entries
	      }
	    else
	      {
		// Otherwise it's a directory
		// read error which might be
		// followed by some valid
		// files.
		if (entry_error_count++ == 0)
		  sample_entry_errno = errno;
	      }
	  }
	std::string directory_entry = std::string(dirname) + std::string(dent->d_name);

	if (sccs_name::valid_filename(directory_entry.c_str()).ok()
	    && is_readable(directory_entry.c_str()))
	  {
	    cssc::FailureOr<bool> dircheck = is_directory(directory_entry.c_str());
	    if (dircheck.ok())
	      {
		if (*dircheck)
		  warning("Ignoring subdirectory %s",
			  directory_entry.c_str());
		else
		  result.push_back(directory_entry);
	      }
	    else
	      {
		warning("Don't know if %s is a subdirectory (%s), ignoring it",
			directory_entry.c_str(),
			dircheck.to_string().c_str());
	      }
	  }
      }
    if (entry_error_count)
      {
	warning("%lu errors occurred reading from directory '%s' (example: \"%s\"), "
		"so some directory entries may have been ignored.",
		entry_error_count, passed_dir_name.c_str(), strerror(sample_entry_errno));
      }
    return result;
  }

  std::vector<std::string> from_stdin()
  {
    std::vector<std::string> result;

    cssc_linebuf linebuf;

    while (linebuf.read_line(stdin).ok())
      {
	std::string s (linebuf.c_str());
	auto name = s.substr(0, s.length()-1u); // chop off the newline.
	result.push_back(name);
      }
    return result;
  }


}  // unnamed namespace

sccs_file_iterator::sccs_file_iterator(const CSSC_Options &opts)
  : source_(source::NONE),
    is_unique_(false),
    files_(),
    pos(0),
    name_()
{
  auto argv = opts.get_argv() + opts.get_index();
  auto argc = opts.get_argc() - opts.get_index();

  if (argc < 1)
    {
      source_ = source::NONE;
      return;
    }

  char *first = argv[0];

  if (strcmp(first, "-") == 0)
    {
      source_ = source::STDIN;
      files_ = from_stdin();
      return;
    }

  if (first[0] != '\0')
    {
      DIR *dir = opendir(first);
      if (dir != NULL)
	{
	  source_ = source::DIRECTORY;
	  ResourceCleanup dir_closer([&dir](){
	      closedir(dir);
	    });
	  files_ = from_directory(first, dir);
	  pos = 0;
	  return;
	}
    }

  source_ = source::ARGS;
  is_unique_ = (1 == argc);
  files_.reserve(argc);
  while (argc-- > 0)
    {
      std::string n(*argv++);
      sccs_name sname;
      sname = n;
      if (sname.valid())
	{
	  files_.push_back(n);
	}
      else
	{
	  sname.make_valid();
	  files_.push_back(sname.sfile());
	}
    }
}

bool
sccs_file_iterator::unique() const
{
  return is_unique_;
}


bool sccs_file_iterator::next()
{
  if (pos == files_.size())
    return false;		// end
  name_ = files_[pos++];
  return true;
}


/* Local variables: */
/* mode: c++ */
/* End: */
