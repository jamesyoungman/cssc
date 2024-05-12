
/*
 * delta-table.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2007, 2008, 2009, 2010, 2011, 2014,
 *  2019, 2024 Free Software Foundation, Inc.
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
 */

#include <config.h>

#include "cssc.h"
#include "delta-table.h"
#include "delta-iterator.h"

bool
cssc_delta_table::delta_at_seq_exists(seq_no seq) const
{
  const seq_no limit = l_.get_high_seqno();
  ASSERT (0 < seq);
  ASSERT (seq <= limit);
  return l_.delta_at_seq_exists(seq);
}

const delta &
cssc_delta_table::delta_at_seq(seq_no seq) const
{
  const seq_no limit = l_.get_high_seqno();
  ASSERT (0 < seq);
  ASSERT (seq <= limit);
  return l_.delta_at_seq(seq);
}



cssc_delta_table::~cssc_delta_table()
{
}



seq_no cssc_delta_table::next_seqno() const
{
  ASSERT(nullptr != this);
  seq_no next = highest_seqno();
  ++next;
  return next;
}



/* Adds a delta to the end of the delta_table. */

void
cssc_delta_table::add(const delta &it)
{
  ASSERT(nullptr != this);

  l_.add(it);
}

/* for the prepend() operation, see dtbl-prepend.cc. */


/* Finds a delta in the delta table by its SID. */

delta const * cssc_delta_table::
find(sid id) const
{
  ASSERT(nullptr != this);
  const_delta_iterator iter(this, delta_selector::current);

  while (iter.next())
    {
      if (iter->id() == id)
	{
	  return iter.operator->();
	}
    }
  return NULL;
}

/* Finds a delta in the delta table by its SID.
 * Removed deltas are counted.
 */

delta const * cssc_delta_table::
find_any(sid id) const
{
  ASSERT(nullptr != this);
  const_delta_iterator iter(this, delta_selector::all);

  while (iter.next())
    {
      if (iter->id() == id)
	{
	  return iter.operator->();
	}
    }
  return NULL;
}

// This non-const variety is used by sf-cdc.cc.
delta * cssc_delta_table::
find(sid id)
{
  ASSERT(nullptr != this);
  delta_iterator iter(this, delta_selector::current);

  while (iter.next())
    {
      if (iter->id() == id)
	{
	  return iter.operator->();
	}
    }
  return NULL;
}


std::unique_ptr<cssc_delta_table> make_unique_cssc_delta_table()
{
#if __cplusplus >= 201402L
  return std::make_unique<cssc_delta_table>();
#else
  return std::unique_ptr<cssc_delta_table>(new cssc_delta_table());
#endif
}
