/*
 * location.h: Part of GNU CSSC.
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
#include "config.h"

#include <sstream>

#include "location.h"

std::string sccs_file_location::as_string() const
{
  std::stringstream s;
  s << name() << " line " << lineno_;
  return s.str();
}

sccs_file_location&
sccs_file_location::operator=(const sccs_file_location& other)
{
  name_ = other.name_;
  lineno_ = other.lineno_;
  return *this;
}
