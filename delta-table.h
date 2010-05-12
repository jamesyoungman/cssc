/*
 * delta-table.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999,2007,2008 Free Software Foundation, Inc. 
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
 *
 * Definition of the classes cssc_delta_table and delta_iterator.
 */


#ifndef CSSC_DELTA_TABLE_H
#define CSSC_DELTA_TABLE_H 1

#include <deque>
#include <vector>
#include <map>

#include "delta.h"

class abstract_delta_list
{
  seq_no high_seqno;
  sid high_release;

protected:
  void update_highest(const delta& d)
  {
    if (d.seq() > high_seqno)
      high_seqno = d.seq();

    if (!d.removed()) 
      {
	if (high_release.is_null())
	  {
	    if (d.id().on_trunk())
	      high_release = d.id();
	  }
	else if (d.id() > high_release)
	  {
	    high_release = d.id();
	  }
      }
  }
  
public:
  abstract_delta_list()
    : high_seqno(0),
      high_release(sid::null_sid())
  {
  }
  
  virtual seq_no get_high_seqno() const
  {
    return high_seqno;
  }

  virtual const sid& get_high_release() const
  {
    return high_release;
  }
  
  virtual int length() const = 0;
  virtual const delta& select(int i) const = 0;
  virtual delta& select(int i) = 0;
  virtual ~abstract_delta_list();
};

class mylist_delta_list : public abstract_delta_list
{
  mylist<struct delta> l;
  int *seq_table;

  void
  build_seq_table()
  {
    ASSERT(0 != this);
    const seq_no highseq = get_high_seqno();

    seq_table = new int[highseq + 1];

    int i;
    for(i = 0; i < highseq + 1; i++)
      {
	seq_table[i] = -1;
      }
    for (int i=0; i<l.length(); ++i)
      {
	const seq_no seq = l.select(i).seq();
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
	seq_table[seq] = i;
      }
  }

public:
  mylist_delta_list()
    : seq_table(0)
  {
  }
  
  virtual int length() const
  {
    return l.length();
  }

  virtual const delta& select(int i) const
  {
    return l[i];
  }
  
  virtual delta& select(int i)
  {
    return l.select(i);
  }
  
  virtual void add(const delta& d)
  {
    l.add(d);
    update_highest(d);
  }

  virtual abstract_delta_list& operator += (const abstract_delta_list& other)
  {
    for (int i=0; i<other.length(); ++i)
      {
	add(other.select(i));
      }
    return *this;
  }
  
  virtual ~mylist_delta_list()
  {
    delete[] seq_table;
  }
  
  virtual bool delta_at_seq_exists(seq_no seq)
  {
    if (seq_table == NULL)
      {
	build_seq_table();
      }
    return seq_table[seq] != -1;
  }

  virtual const delta& delta_at_seq(seq_no seq)
  {
    if (seq_table == NULL)
      {
	build_seq_table();
      }
    return select(seq_table[seq]);
  }

private:
};


class stl_delta_list : public abstract_delta_list
{
  std::vector<struct delta> items;
  std::map<seq_no, size_t> seq_table;

public:
  virtual int length() const
  {
    return items.size();
  }

  virtual const delta& select(int i) const
  {
    return items[i];
  }
  
  virtual delta& select(int i)
  {
    return items[i];
  }
  
  virtual void add(const delta& d)
  {
    size_t pos = items.size();
    items.push_back(d);
    seq_table[d.seq()] = pos;
    update_highest(d);
  }
  
  virtual abstract_delta_list& operator += (const abstract_delta_list& other)
  {
    for (int i=0; i<other.length(); ++i)
      {
	add(other.select(i));
      }
    return *this;
  }

  virtual bool delta_at_seq_exists(seq_no seq)
  {
    std::map<seq_no, size_t>::const_iterator i = seq_table.find(seq);
    return i != seq_table.end();
  }
  
  virtual const delta& delta_at_seq(seq_no seq)
  {
    std::map<seq_no, size_t>::const_iterator i = seq_table.find(seq);
    ASSERT (i != seq_table.end());
    return items[i->second];
  }
};


class cssc_delta_table
{
  //typedef mylist_delta_list delta_list;
  typedef stl_delta_list delta_list;
  delta_list l;

  cssc_delta_table &operator =(cssc_delta_table const &); /* undefined */
  cssc_delta_table(cssc_delta_table const &); /* undefined */

public:
  cssc_delta_table()
  {
  }

  void add(const delta &d);		
  void prepend(const delta &); /* sf-add.c */

  // These two methods should b const, but are not because they 
  // call build_seq_table().
  const bool delta_at_seq_exists(seq_no seq);
  const delta & delta_at_seq(seq_no seq);

  const delta *find(sid id) const; 
  const delta *find_any(sid id) const; // includes removed deltas.
  delta *find(sid id); 

  seq_no highest_seqno() const { return l.get_high_seqno(); }
  seq_no next_seqno()    const;
  sid highest_release() const { return l.get_high_release(); }

  int length() const { return l.length(); }

  const delta& select(int pos) const { return l.select(pos); }
  delta& select(int pos) { return l.select(pos); }
  
  ~cssc_delta_table();
};


#endif /* CSSC_DELTA_TABLE_H */

/* Local variables: */
/* mode: c++ */
/* End: */
