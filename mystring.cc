
/*
 * mystring.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc. 
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
 * Members of the class string.
 *
 */


// We don't implement the standard string class, so we only have
// code here if USE_STANDARD_STRING is not defined.


#ifdef __GNUC__
#pragma implementation "mystring.h"
#endif

#include "cssc.h"
#include "mystring.h"


// If we have a standard <string> header we do not need to define the
// class mystring since it is just a typedef for the standard string
// class.  To find out if we're using that string class, we include
// "mystring.h", which checks the macro HAVE_STRING from config.h,
// which tells us if we have a <string> header.


#ifndef USE_STANDARD_STRING

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: mystring.cc,v 1.13 1999/03/19 23:58:34 james Exp $";
#endif




struct mystring::MyStrRep
{
public:
  charT *data;
  int  refs;
  size_type len;
  
  MyStrRep(const char *s, size_type sl) // length already measured.
  {
    data = new charT[sl+1];
    len = sl;
    memcpy(data, s, len*sizeof(charT));
    data[sl] = 0;		// terminate.
    refs = 1;
  }
  
  ~MyStrRep()
  {
    delete[] data;
    data = 0;
    refs = 0;
    len = 0;
  }

  // We don't want the default copy methods.
private:
  MyStrRep(const MyStrRep&);
  MyStrRep& operator=(const MyStrRep&);
  MyStrRep() : data(0), refs(0) { }
};

mystring::mystring()
{
  rep = new MyStrRep("", 0);
}

mystring::mystring(const charT* s, size_type len)
{
  rep = new MyStrRep(s, len);
}

mystring::mystring(const charT* s)
{
  rep = new MyStrRep(s, strlen(s));
}

mystring::mystring(const mystring& from)
{
  rep = from.rep;
  rep->refs++;
}

mystring& 
mystring::operator=(const mystring& from)
{
  if (&from != this)
    {
      if (1 == rep->refs--)
	delete rep;

      rep = from.rep;
      rep->refs++;
    }
  return *this;
}

mystring&
mystring::assign(const mystring &s, size_type pos)
{
  // We _could_ cope with pos being zero specially,
  // but we do not.
  size_type flen = s.length();
  if (pos < flen)
    {
      MyStrRep* new_rep = new MyStrRep(s.rep->data + pos, flen-pos);
      if (1 == rep->refs--)
	{
	  delete rep;
	}
      rep = new_rep;
    }
  else
    {
      // Tried to copy the bit of string beyond the end...
      if (1 == rep->refs--)
	{
	  delete rep;
	}
      rep = new MyStrRep("", 0);
    }
  return *this;
}

bool mystring::valid() const
{
  return rep != 0;
}

bool mystring::empty() const
{
  return length() == 0;
}

mystring::size_type 
mystring::length() const
{
  return rep->len;
}

mystring::charT
mystring::at(size_type pos) const
{
  ASSERT(pos < length());	// TODO: throw exception??
  return rep->data[pos];
}

mystring::ModifiableReference mystring::at(mystring::size_type pos)
{
  ASSERT(pos < length());	// TODO: throw exception??
  return ModifiableReference(*this, pos);
}

const char* mystring::c_str() const
{
  return rep->data;		// already terminated.
}

mystring
mystring::substr(size_type first, size_type last) const
{
  ASSERT(length() > first);

  if (last > length())
    last = length();
 
  return mystring(rep->data + first, last-first);
}


// Not very efficient but this is a get-you-home implementation 
// anyway; the header <string> should provide the Real Thing anyway.
mystring mystring::operator+(const mystring &s) const
{
  size_type newlen = length() + s.length();
  charT *newdata = new charT[newlen];
  memcpy(newdata, rep->data, rep->len);
  memcpy(newdata+rep->len, s.rep->data, s.rep->len);

  return mystring(newdata, newlen);
}

// TODO: either using memcmp() here is invalid, 
// or we could just use strcmp()...
int
mystring::compare(const mystring &s) const
{
  if (rep == s.rep)
    {
      return 0;			// no difference
    }
  else
    {
      // Compare the two strings, up to the smaller of the two lengths.
      bool me_longer = length() > s.length();
      size_type minlen;
      
      if (me_longer)
	minlen = s.length();
      else
	minlen = length();
      
      const int cmp = memcmp(rep->data, s.rep->data, minlen);
      if (cmp)
	return cmp;

      // If the strings are of different length, and one is
      // a prefix of the other, the longer string is "greater".
      if (me_longer)
	return 1;
      else if (s.length() > length())
	return -1;
      else
	return 0;
    }
}

bool
mystring::operator==(const mystring &s) const
{
  return compare(s) == 0;
}

bool
mystring::operator!=(const mystring &s) const
{
  return compare(s) != 0;
}

void mystring::prepare_for_writing()
{
  if (rep->refs > 1)
    {
      MyStrRep *oldrep = rep;
      rep = new MyStrRep(rep->data, rep->len);
      --oldrep->refs;
    }
}

mystring::charT &
mystring::ModifiableReference::operator=(charT ch)
{
  if (ch != s.rep->data[pos])
    {
      s.prepare_for_writing();
      s.rep->data[pos] = ch;
    }
  return s.rep->data[pos];
}

mystring::ModifiableReference::operator char() const
{
  return s.rep->data[pos];
}

mystring::size_type
mystring::find_last_of(charT needle) const
{
  size_type xpos = length();
  while (xpos-- > 0)
    if (rep->data[xpos] == needle)
      return xpos;
  return npos;
}


#endif /* USE_STANDARD_STRING */

	
/* Local variables: */
/* mode: c++ */
/* End: */

