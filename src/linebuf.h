/*
 * linebuf.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 2007, 2008, 2009, 2010, 2011, 2014, 2019,
 *  2024 Free Software Foundation, Inc.
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
 * Defines the class cssc_linebuf.
 *
 */

#ifndef CSSC__LINEBUF_H__
#define CSSC__LINEBUF_H__

#include <memory>

#include "failure.h"
#include "location.h"

/* This class is used to read lines of unlimited length from a file. */

class cssc_linebuf
{
  // TODO: use some STL data structure, or a Cord, to hold the data.
  char *buf_;
  size_t buflen_;

public:
  cssc_linebuf();

  // This class has a pointer member.  So that we don't have to
  // override the copy constructor and assignment operator, we just
  // delete them.
  cssc_linebuf& operator=(const cssc_linebuf&) = delete;
  cssc_linebuf(const cssc_linebuf&) = delete;

  cssc::Failure read_line(FILE *f);

  // TODO: Reduce the use of c_str() in favour of operations that more
  // directly reflect what the program actually needs (perhaps for
  // example a string_view).
  const char *c_str() const { return buf_; }
  const char *c_str() { return buf_; }

  // TODO: set_char is only used in sccs_file_parser::read_delta, to
  // nul-terminate the string.  If we move to string_view, we can
  // probably achieve the same thing by truncating the string_view
  // (and hence no need to modify the buffer at all).
  void set_char(unsigned offset, char value);

  // TODO: split into a vector of std::string, perhaps.
  int split(int offset, char **args, int len, char c);

  // Returns true if the buffer contains a valid SCCS id keyword.
  bool check_id_keywords() const;

  cssc::Failure write(FILE*) const;

  char &operator [](int index) const { return buf_[index]; }

#ifdef __GNUC__
  char *operator +(int index) const { return buf_ + index; }
#endif

  ~cssc_linebuf() { delete[] buf_; buf_ = nullptr; }
};

std::unique_ptr<cssc_linebuf> make_unique_linebuf();

#endif /* __LINEBUF_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
