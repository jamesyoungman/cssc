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
 * @(#) CSSC mystring.h 1.1 93/11/09 17:24:42
 *
 */

#ifndef __MYSTRING_H__
#define __MYSTRING_H__

#ifdef __GNUC__
#pragma interface
#endif

#define STR_OFFSET (offsetof(ptr, str))
#define STR_PTR(str) ((ptr *)((str) - STR_OFFSET))

/* This is a very simple string class.  Its purpose is mainly to
   simplify the allocation of strings. */

class mystring {
	struct ptr {
		int refs;
		char str[1];
	};

	char *str;

	void create(const char *s);
	void create(const char *s1, const char *s2);
	void destroy();

	void
	copy(mystring const &s) {
		str = s.str;
		if (str != NULL) {
			STR_PTR(str)->refs++;
		}
	}

public:
	mystring(): str(NULL) {}
	mystring(const char *s) { create(s); }
	mystring(const char *s1, const char *s2) { create(s1, s2); }
        mystring(const char *s, size_t len);
        mystring(mystring &s1, mystring &s2) { create(s1.str, s2.str); }
        mystring(char *s) { create(s); }
        mystring(char *s1, char *s2) { create(s1, s2); }
	mystring(mystring const &s) {
		copy(s);
	}

	mystring &operator =(mystring const &s);
	mystring &operator =(const char *s);

  	const mystring& operator+=(const mystring& s);
  	mystring  operator+ (const mystring& s);
  
	operator const char *() const {
		return str;
	}
  	operator bool() const   {
	  return str ? true : false;
  	}
  
#ifdef __GNUC__
	operator void *() const {
		return str;
	}
  // I am informaed that this next function is required to compile
  // CSSC with the latest snapshots of GCC (as of 970811).  However,
  // I don't have access to these to really fix the ACTUAL problem
  // so the following aberration persists... XXX please fix me!
	operator char *() const { // TODO: remove me 
		return str;	  // and fix the calling code!
	}
#endif

	int
	operator ==(mystring const &s) const {
		assert(s.str != NULL);
		assert(str != NULL);
		return strcmp(str, s.str) == 0;
	}
	int operator !=(mystring const &s) const { return !operator==(s); }	

	int
	operator ==(const char *s) const {
		if (s == NULL) {
			return str == NULL;
		}
		assert(str != NULL);
		return strcmp(str, s) == 0;
	}
	int operator !=(const char *s) const { return !operator==(s); }
  
	int
	print(FILE *out) const {
		return fputs(str, out) == EOF;
	}
		
	char *
	xstrdup() const {
		return strcpy((char *) xmalloc(strlen(str) + 1), str);
	}

	~mystring() {
		destroy();
	}
};

#endif /* __MYSTRING_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
