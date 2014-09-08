/*
 * sccs-delta.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,2007 Free Software Foundation, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Members of class delta.
 *
 */

#include <config.h>
#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"

delta &
delta::operator =(delta const &it)
{
  inserted_ = it.inserted_;
  deleted_ = it.deleted_;
  unchanged_ = it.unchanged_;
  set_type(it.delta_type_);
  id_ = it.id_;
  date_ = it.date_;
  user_ = it.user_;
  seq_ = it.seq_;
  prev_seq_ = it.prev_seq_;

  included_ = it.included_;
  excluded_ = it.excluded_;
  ignored_ = it.ignored_;
  have_includes_ = it.have_includes_;
  have_excludes_ = it.have_excludes_;
  have_ignores_  = it.have_ignores_;

  mrs_ = it.mrs_;
  comments_ = it.comments_;
  return *this;
}

/* Local variables: */
/* mode: c++ */
/* End: */
