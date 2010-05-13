/*
 * linebuf.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,2007 Free Software Foundation, Inc. 
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Defines the class cssc_linebuf.
 *
 */

#ifndef CSSC__LINEBUF_H__
#define CSSC__LINEBUF_H__


/* This class is used to read lines of unlimited length from a file. */

class cssc_linebuf
{
  char *buf;
  size_t buflen;
  
public:
  cssc_linebuf();

  int read_line(FILE *f);

  const char *c_str() const { return buf; }
  const char *c_str() { return buf; }
  void set_char(unsigned offset, char value);
  int split(int offset, char **args, int len, char c);
  int check_id_keywords() const;
  int write(FILE*) const;
  
  char &operator [](int index) const { return buf[index]; }

#ifdef __GNUC__
  char *operator +(int index) const { return buf + index; }
#endif

  ~cssc_linebuf() { delete[] buf; }
};

#endif /* __LINEBUF_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
