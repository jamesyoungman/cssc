/*
 * encoding.cc: Part of GNU CSSC.
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
 *
 * Functions for manipulating encoded binary files.
 * The encoding is the same as that used for UUENCODE,
 * but without the "begin" and "end" lines.
 * 
 * Compile on its own, defining TEST_CODE, to compile a test program.
 */

#include "bodyio.h"


//
// Lines in the UUENCODEd format look like this:--

// M<F]O=#HZ,#HP.G)O;W0Z+W)O;W0Z+V)I;B]B87-H"F)I;CHJ.C$Z,3IB:6XZ

// The first character is an uppercase "M".  It's really a 
// character count.  "M" represents the largest possible 
// count (total line length 60 [M + 60 chars + newline]).


// decode a line, returning the number of characters in it.
int
decode_line(const char in[], char out[])
{
  int len = UUDEC(in[0]);
  if (len <= 0)
    return 0;

  ++in;				// step over byte count.
  
  int n;
  for (n=0; n<len; n+=3)
    {
      decode(in, out);
      in += 4;
      out += 3;
    }
  return len;
}


// encode a line, returning the number of characters in it.
void
encode_line(const char in[], char out[], int len)
{
  *out++ = UUENC(len);
  
  while (len > 0)
    {
      encode(in, out);
      in += 3;
      out += 4;
      len -= 3;
    }
  *out++ = '\n';
  *out++ = '\0';
}

int
encode_stream(FILE *fin, FILE *fout)
{
  char inbuf[80], outbuf[80];
  int len;
  
  do
    {
      len = fread(inbuf, 1, 45, fin);
      encode_line(inbuf, outbuf, len);
      fprintf(fout, "%s", outbuf);
    }
  while (len);
  
  return ferror(fin) || ferror(fout);
}
