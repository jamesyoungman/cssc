/*
 * delta-iterator.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1999, 2007, 2008, 2009, 2010, 2011, 2014, 2019
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
 * Members of class delta_iterator.
 *
 */

#include <config.h>

#include "cssc.h"
#include "delta-iterator.h"
#include "delta-table.h"
#include "cssc-assert.h"

delta_iterator::delta_iterator(cssc_delta_table *d, delta_selector selector)
  : dtbl_(d), pos_(0), first_(true), selector_(selector)
{
}

bool
delta_iterator::next()
{
  ASSERT(nullptr != dtbl_);

  while ((pos_=advance()) < dtbl_->size())
    {
      if (all() || !dtbl_->at(pos_).removed())
	{
	  return 1;
	}
    }
  return 0;
}

cssc_delta_table::size_type delta_iterator::index() const
{
  return pos_;
}

delta const *
delta_iterator::operator ->() const
{
  ASSERT(nullptr != dtbl_);
  return &dtbl_->at(pos_);
}

delta *
delta_iterator::operator ->()
{
  ASSERT(nullptr != dtbl_);
  return &dtbl_->at(pos_);
}


////////////////////////////////////////////////////////////


const_delta_iterator::const_delta_iterator(cssc_delta_table const *d, delta_selector selector)
  : dtbl_(d), pos_(0), first_(true), selector_(selector)
{
}

bool
const_delta_iterator::next()
{
  ASSERT(nullptr != dtbl_);
  while ((pos_=advance()) < dtbl_->size())
    {
      if (all() || !dtbl_->at(pos_).removed())
	{
	  return true;
	}
    }
  return false;
}

cssc_delta_table::size_type const_delta_iterator::index() const
{
  return pos_;
}

delta const *
const_delta_iterator::operator ->() const
{
  ASSERT(nullptr != dtbl_);
  return &dtbl_->at(pos_);
}



/* Local variables: */
/* mode: c++ */
/* End: */
