/*
 * mystring.h: Part of GNU CSSC.
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
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Defines the class mystring.
 *
 * $Id: mystring.h,v 1.11 1998/01/15 22:50:17 james Exp $
 *
 */

#ifndef CSSC__MYSTRING_H__
#define CSSC__MYSTRING_H__

// If we have the header file <string>, then
// use that rather than mystring.
#ifdef HAVE_STRING
#define USE_STANDARD_STRING
#endif


#ifdef USE_STANDARD_STRING

#include <string>
typedef string mystring;


#else /* Use our own "mystring". */

#ifdef __GNUC__
#pragma interface
#endif

#define STR_OFFSET (offsetof(ptr, str))
#define STR_PTR(str) ((ptr *)((str) - STR_OFFSET))

/* This is a very simple string class.  Its purpose is mainly to
   simplify the allocation of strings. */


class mystring
{
  class MyStrRep;		// nested class.
  class ModifiableReference;
  
  MyStrRep *rep;
  
  
public:
  typedef unsigned long size_type;
  typedef char charT;
  
  static const size_type npos = static_cast<size_type>(-1);
  
  
  // Constructors...
  mystring();
  mystring(const charT*);
  mystring(const charT*, size_type); // first N characters.
  mystring(const mystring&);

  // Assignment
  mystring& operator=(const mystring&);
  mystring& assign(const mystring&, size_type pos);
  
  // Observers
  bool valid() const;
  bool empty() const;
  size_type length() const;
  
  // Data access
  charT at(size_type) const;
  ModifiableReference at(size_type);
  
  
  // Casts etc.
  const charT *c_str() const;
  
  // Slicing...
  mystring substr(size_type first, size_type last) const;

  // Operators...
  mystring operator+(const mystring&) const;
  bool operator==(const mystring&) const;


  // Searching..
  size_type find_last_of(charT) const;


private:

  class ModifiableReference
  {
    mystring &s;
    size_type pos;
  public:
    ModifiableReference(mystring& sref, size_type i) : s(sref), pos(i) { }
    charT& operator=(charT ch);
    operator char() const;
  };

  int compare(const mystring &) const;

  // Copy management.
  void prepare_for_writing();
};


#endif /* USE_STANDARD_STRING */


#endif /* __MYSTRING_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
