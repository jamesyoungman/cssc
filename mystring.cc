/*
 * mystring.c: Part of GNU CSSC.
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
 * Members of the class string.
 *
 */


// We don't implement the standard string class, so we only have
// code here if USE_STANDARD_STRING is not defined.


#include "cssc.h"
#include "mystring.h"

#ifndef USE_STANDARD_STRING

#ifdef __GNUC__
#pragma implementation "mystring.h"
#endif

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: mystring.cc,v 1.8 1997/11/18 23:22:22 james Exp $";
#endif


void
mystring::create(const char *s) {
	if (s == NULL) {
		str = NULL;
		return;
	}

	ptr *p = (ptr *) xmalloc(STR_OFFSET + strlen(s) + 1);
	strcpy(p->str, s);
	p->refs = 1;

	str = (char *) p + STR_OFFSET;
}


void
mystring::create(const char *s1, const char *s2) {
	if (s1 == NULL) {
		create(s2);
		return;
	}
	if (s2 == NULL) {
		create(s1);
		return;
	}

	int len1 = strlen(s1);
	ptr *p = (ptr *) xmalloc(STR_OFFSET + len1 + strlen(s2) + 1);
	strcpy(p->str, s1);
	strcpy(p->str + len1, s2);
	p->refs = 1;

	str = (char *) p + STR_OFFSET;
}

mystring::mystring(const char *s, size_t len)
{
  ptr *p = (ptr *) xmalloc(STR_OFFSET + len + 1);
  memcpy(p->str, s, len);
  p->str[len] = '\0';
  p->refs = 1;
  str = (char *) p + STR_OFFSET;
}

const mystring& mystring::operator+=(const mystring& s)
{
  size_t newlen = strlen(str) + strlen(s.str) + 1;
  ptr *p = (ptr *) xmalloc(STR_OFFSET + newlen);
  strcpy(p->str, str);
  strcat(p->str, s.str);
  destroy();
  create(p->str);
  free(p);
  return *this;
}


mystring mystring::operator+(const mystring& s)
{
  mystring ret(str);
  ret += s;
  return ret;
}


void
mystring::destroy() {
	if (str != NULL && --STR_PTR(str)->refs == 0) {
		free(STR_PTR(str));
	}
}

mystring &
mystring::operator =(mystring const &s) {
	if (this != &s) {
		destroy();
		copy(s);
	}
	return *this;
}

mystring &
mystring::operator =(const char *s) {
	ASSERT(s == NULL || str != s);
	destroy();
	create(s);
	return *this;
}

const char&
mystring::operator[](size_t n) const
{
  return str[n];
}


#endif /* USE_STANDARD_STRING */

	
/* Local variables: */
/* mode: c++ */
/* End: */

