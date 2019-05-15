/*
 * parser.h: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2007,
 *  2008, 2009, 2010, 2011, 2014, 2019 Free Software Foundation, Inc.
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
#ifndef CSSC__LOCATION_H__
#define CSSC__LOCATION_H__

#include <string>

class sccs_file_location 
{
public:
  explicit sccs_file_location(const std::string& n)
    : name_(n), lineno_(0) {}
  sccs_file_location(const std::string& n, int line_number)
    : name_(n), lineno_(line_number) {}

  const std::string& name() const
  {
    return name_;
  }
  
  std::string as_string() const;
  
  int line_number() const
  {
    return lineno_;
  }

  void advance_line()
  {
    ++lineno_;
  }

  void set_line_number(int n) 
  {
    lineno_ = n;
  }
  
  sccs_file_location& operator=(const sccs_file_location&);

protected:
  std::string name_;

 private:
  int lineno_;
};

#endif /* CSSC__LOCATION_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
