/*
 * delta-iterator.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
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
 *
 * Definition of the class delta_iterator.
 *
 * $Id: delta-iterator.h,v 1.3 2001/09/29 19:39:41 james_youngman Exp $
 *
 */


#ifndef CSSC_DELTA_ITERATOR_H
#define CSSC_DELTA_ITERATOR_H "$Id: delta-iterator.h,v 1.3 2001/09/29 19:39:41 james_youngman Exp $"

#include "delta.h"

class delta_iterator
{
  cssc_delta_table *dtbl;
  int pos;
  
public:
  delta_iterator(cssc_delta_table *d);
  
  int next(int all = 0);
  int index() const;
  
  delta * operator ->();
  const delta * operator ->() const;

  void rewind();
};

class const_delta_iterator
{
  const cssc_delta_table *dtbl;
  int pos;
  
public:
  const_delta_iterator(cssc_delta_table const *d);
  
  int next(int all = 0);
  int index() const;
  
  delta const * operator ->() const;

  void rewind();
};

#endif /* CSSC_DELTA_TABLE_H */

/* Local variables: */
/* mode: c++ */
/* End: */
