/*
 * encoding.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * 
 *
 * Functions for manipulating encoded binary files.
 * The encoding is the same as that used for UUENCODE,
 * but without the "begin" and "end" lines.
 * 
 * Compile on its own, defining TEST_CODE, to compile a test program.
 */

#ifndef TEST_CODE
// Various #include statements not required by the test code.
#include "cssc.h"

#endif


// This system (uuencoding) will not work on non-ascii machines,
// because it assumed that there is a block of printable characters
// following the space charcter in the execution character set.
//
// Octal is quite convenient for thinking about uuencoding since
// two octal digits make six bits.
//
#define UUDEC(c)	(((c) - 040) & 077)
#define UUENC(c)        (((c) & 077) + 040)

static inline void
encode(const char in[3], char out[4])
{
  // Notice that the bitmasks always add up to 077.
  out[0] = UUENC(((in[0] >> 2)));
  out[1] = UUENC(((in[0] << 4) & 060) | ((in[1] >> 4) & 017));
  out[2] = UUENC(((in[1] << 2) & 074) | ((in[2] >> 6) & 003));
  out[3] = UUENC(((in[2]       & 077)));
}

static inline void 
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


#ifdef TEST_CODE

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

int
test_encode(void)
{
  return encode_stream(stdin, stdout);
}

int
test_decode(void)
{
  char inbuf[80], outbuf[80];
  while ( 0 != fgets(inbuf, sizeof(inbuf)-1, stdin) )
    {
      int len = decode_line(inbuf, outbuf);
      if (0 == len)
	return 0;
      fwrite(outbuf, 1, len, stdout);
    }
  return 1;
}


// Test all possible inputs for encode(); decode its
// outputs and check that they decode back to the correct value.
int test_all(void)
{
  union lunch { long l; char ch[4]; } in, out;
  long i;

  // i only has to hlod a 24-bit value.
  const long maxval = 0xff | (0xff<<8) | (0xff<<16);
  const double dmaxval = maxval;
  for (i=0; i<=maxval; i++)
    {
      if ( 0x7FFFF == (i & 0x7FFFF) )
	{
	  double completed = (100.0 * i) / dmaxval;
	  printf("%06lx %3.0f%%...\n", i, completed);
	}
      
      
      in.ch[0] = (i & 0x0000ff) >>  0;
      in.ch[1] = (i & 0x00ff00) >>  8;
      in.ch[2] = (i & 0xff0000) >> 16;
      in.ch[3] = '\0';
      
      encode(in.ch, out.ch);
      decode(out.ch, in.ch);
      const long l0 = ((unsigned char) in.ch[0]) & 0xff;
      const long l1 = ((unsigned char) in.ch[1]) & 0xff;
      const long l2 = ((unsigned char) in.ch[2]) & 0xff;
      const long lo = l0 | l1<<8 | l2<<16;

      if (lo != i)
	{
	  fprintf(stderr,
		  "Asymmetry!\n"
		  "Input was %06lx, output was %05lx\n",
		  i, lo);
	  return 1;
	}
    }
  printf("Success!\n");
  return 0;
}

const char *options[] = { "--encode", "--decode", "--all" };
const int (*actions[])(void) = { test_encode, test_decode, test_all };

#define NELEM(array)   (sizeof(array)/sizeof(array[0]))


static void
usage(const char *prog)
{
  fprintf(stderr, "Usage: %s [", prog);
  for (size_t i=0; i<NELEM(options); ++i)
    {
      fprintf(stderr, "%s %s", (i>0) ? " |" : "", options[i]);
    }
  fprintf(stderr, " ]\n");
}

int
main(int argc, char *argv[])
{
  if (2 == argc)
    for (size_t i=0; i<NELEM(options); ++i)
      if (0 == strcmp(options[i], argv[1]))
	return (actions[i])();
  
  usage(argv[0] ? argv[0] : "encoding-test");
  return 1;
}

#endif
