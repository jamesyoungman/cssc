/*
 * linebuf.h: Part of GNU CSSC.
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
 * Defines the class _linebuf.
 *
 * @(#) CSSC linebuf.h 1.1 93/11/09 17:17:47
 *
 */

#ifndef CSSC__LINEBUF_H__
#define CSSC__LINEBUF_H__

#ifdef __GNUC__
//#pragma interface
#endif

/* This class is used to read lines of unlimited length from a file. */

class _linebuf {
	char *buf;
	int buflen;

public:
	_linebuf(): buf((char *) xmalloc(CONFIG_LINEBUF_CHUNK_SIZE)),
	           buflen(CONFIG_LINEBUF_CHUNK_SIZE) {}

	int read_line(FILE *f);

	operator char *() const { return buf; }
  	const char *c_str() const { return buf; }
	char &operator [](int index) const { return buf[index]; }

#ifdef __GNUC__
	char *operator +(int index) const { return buf + index; }
#endif

	~_linebuf() { free(buf); }
};

#endif /* __LINEBUF_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
