/*
 * delta-table.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999 Free Software Foundation, Inc. 
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
 */

#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"
#include "delta-table.h"
#include "delta-iterator.h"

const bool
cssc_delta_table::delta_at_seq_exists(seq_no seq)
{
  ASSERT(0 != this);
  ASSERT(seq > 0 && seq <= high_seqno);
  if (seq_table == NULL)
    {
      build_seq_table();
    }
  return seq_table[seq] != -1;
}

const delta &
cssc_delta_table::delta_at_seq(seq_no seq)
{
  ASSERT(0 != this);
  ASSERT(seq > 0 && seq <= high_seqno);
  if (seq_table == NULL)
    {
      build_seq_table();
    }
  return l.select(seq_table[seq]);
}



cssc_delta_table::~cssc_delta_table()
{
  ASSERT(0 != this);
  if (seq_table)
    delete [] seq_table;
  seq_table = 0;
}



seq_no cssc_delta_table::next_seqno() const
{
  ASSERT(0 != this);
  return highest_seqno() + 1u;
}


/* Builds the seq_no to delta table index table. */

void
cssc_delta_table::build_seq_table()
{
  ASSERT(0 != this);

  seq_table = new int[high_seqno + 1];

  int i;
  for(i = 0; i < high_seqno + 1; i++)
    {
      seq_table[i] = -1;
    }
  
  delta_iterator iter(this);
  while (iter.next(1))
    {
      seq_no seq = iter->seq;
      if (seq_table[seq] != -1)
	{
          /* ignore duplicate sequence number: some old sccs files
           * contain removed deltas with the same sequence number as
           * existing delta
           */
          continue;
	  s_corrupt_quit("Sequence number %u is duplicated"
	       " in delta table [build].", seq);
	}
      seq_table[seq] = iter.index();
    }
}

void
cssc_delta_table::update_highest(const delta &it) 
{
  ASSERT(0 != this);
  seq_no seq = it.seq;
  
  if (seq > high_seqno) 
    {
      if (seq_table != NULL) 
	{
//
// Create a temporary array to hold seq_table
//
	  int *temp_seq_table = new int[seq + 1];
	  memcpy( temp_seq_table, seq_table, (high_seqno+1)*sizeof( int));
	  delete [] seq_table;
	  seq_table = temp_seq_table;
	  for(int i = high_seqno + 1; i < seq + 1; i++) 
	    {
	      seq_table[i] = -1;
	    }
	}
      high_seqno = it.seq;
    }

  /* high_release is initialised to {0} so 
   * any greater-than comparison always fails since 
   * operator> decides it's not comparable with it.id.
   */
  if (high_release.is_null())
    {
      if (it.id.on_trunk())
	high_release = it.id;
    }
  else if (it.id > high_release)
    {
      high_release = it.id;
    }

  if (seq_table) 
    {
      if (seq_table[seq] != -1) 
	{
	  sid sid_using_this_seqno = delta_at_seq(seq).id;

	  sid_using_this_seqno.print(stderr);
	  it.id.print(stderr);

	  if (it.id != sid_using_this_seqno)
	    {
	      // diagnose...
	      for (int i = 1; i < high_seqno + 1; i++) 
		fprintf(stderr, "%u ", (unsigned)seq_table[i]);

	      fprintf(stderr,
		      "Sequence number %u is duplicated"
		      " in delta table [update] "
		      "(already used by entry %u: ", seq, seq_table[seq]);
	      sid_using_this_seqno.print(stderr);
	      fprintf(stderr, ")\n");
	  
	      s_corrupt_quit("Sequence number %u is duplicated"
			     " in delta table [update]", seq);
	    }
	}
      seq_table[seq] = l.length() - 1;
    }
}



/* Adds a delta to the end of the delta_table. */

void
cssc_delta_table::add(const delta &it)
{
  ASSERT(0 != this);
  
  l.add(it);
  update_highest(it);
}

/* for the prepend() operation, see sf-add.cc. */


/* Finds a delta in the delta table by its SID. */

delta const * cssc_delta_table::
find(sid id) const
{
  ASSERT(0 != this);
  const_delta_iterator iter(this);
  
  while (iter.next())
    {
      if (iter->id == id)
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
  ASSERT(0 != this);
  const_delta_iterator iter(this);
  
  while (iter.next(1))
    {
      if (iter->id == id)
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
  ASSERT(0 != this);
  delta_iterator iter(this);
  
  while (iter.next())
    {
      if (iter->id == id)
	{
	  return iter.operator->();
	}
    }
  return NULL;
}


