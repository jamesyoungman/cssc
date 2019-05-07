/*
 * encoding.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,1998,2007, 2008, 2009, 2010, 2011, 2014 Free Software Foundation, Inc.
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

#include <config.h>

#include "string.h"
#include "cssc.h"
#include "bodyio.h"
#include "cssc-assert.h"


//
// Lines in the UUENCODEd format look like this:--

// M<F]O=#HZ,#HP.G)O;W0Z+W)O;W0Z+V)I;B]B87-H"F)I;CHJ.C$Z,3IB:6XZ

// The first character is an uppercase "M".  It's really a
// character count.  "M" represents the largest possible
// count (total line length 60 [M + 60 chars + newline]).

// This system (uuencoding) will not work on non-ascii machines,
// because it assumed that there is a block of printable characters
// following the space charcter in the execution character set.
//
// Octal is quite convenient for thinking about uuencoding since
// two octal digits make six bits.
//
#define UUDEC(c)	(((c) - 040) & 077)
#define UUENC(c)        (((c) & 077) + 040)

namespace encoding_impl {

void
encode(const char in[3], char out[4])
{
  // Notice that the bitmasks always add up to 077.
  out[0] = UUENC(((in[0] >> 2)));
  out[1] = UUENC(((in[0] << 4) & 060) | ((in[1] >> 4) & 017));
  out[2] = UUENC(((in[1] << 2) & 074) | ((in[2] >> 6) & 003));
  out[3] = UUENC(((in[2]       & 077)));
}

void
decode(const char in[4], char out[3])
{
  // Only the bottom six bits of t0,t1,t2,t3 are ever set,
  // but we use ints for their speed not their size.
  const int t0 = UUDEC(in[0]);
  const int t1 = UUDEC(in[1]);
  const int t2 = UUDEC(in[2]);
  const int t3 = UUDEC(in[3]);

  // Shift counts always add to six; number of bits
  // provided by each line is eight.
  //
  // A left shift of N provides (8-N) bits of value
  // and a right shift provides (6-N) bits of value.
  out[0] = (t0 << 2) | (t1 >> 4); // 6 + 2
  out[1] = (t1 << 4) | (t2 >> 2); // 4 + 4
  out[2] = (t2 << 6) | (t3     ); // 2 + 6
}

}  // encoding_impl

// decode a line, returning the number of characters in it.
size_t
decode_line(const char in[], char out[])
{
  const int count = UUDEC(in[0]);
  if (count <= 0)
    return 0;
  const size_t len = static_cast<size_t>(count);

  ++in;				// step over byte count.
  for (size_t n=0; n<len; n+=3)
    {
      encoding_impl::decode(in, out);
      in += 4;
      out += 3;
    }
  return len;
}

// encode a line; return the number of bytes output (not including the
// terminating NUL).
size_t
encode_line(const char in[], char out[], size_t len)
{
  ASSERT(len <= 60);
  size_t emitted = 0u;
  char length_indicator = static_cast<char>(UUENC(len));

  *out++ = length_indicator;
  ++emitted;
  while (len > 2u)
    {
	encoding_impl::encode(in, out);
      in += 3u;
      out += 4u;
      emitted += 4u;
      len -= 3u;
    }
  // deal with the tail of the buffer.
  if (len)
    {
      char tail[3];
      tail[0] = in[0];
      tail[1] = (len > 1) ? in[1] : char(0);
      tail[2] = char(0);
      encoding_impl::encode(tail, out);
      out += 4u;
      emitted += 4u;
    }

  *out++ = '\n';
  emitted++;
  *out++ = '\0';
  return emitted;
}

int
encode_stream(FILE *fin, FILE *fout)
{
  char inbuf[80], outbuf[80];
  size_t len;

  do
    {
      len = fread(inbuf, 1, 45, fin);
      const size_t bytes = encode_line(inbuf, outbuf, len);
      ASSERT(outbuf[bytes] == 0); // verify output is NUL-terminated
      if (fwrite(outbuf, 1, bytes, fout) != bytes)
	{
	  return -1;
	}
    }
  while (len);

  return ferror(fin) || ferror(fout);
}
