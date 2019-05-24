/*
 * test_encoding.cc: Part of GNU CSSC.
 *
 * Copyright (C) 1997, 1998, 2007, 2010, 2011, 2013, 2019 Free Software
 * Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <cstdlib>

#include "bodyio.h"

using encoding_impl::decode;
using encoding_impl::encode;

int
test_encode(void)
{
  auto result = encode_stream(stdin, stdout);
  if (!result.first.ok()) 
    {
      return 1;
    }
  if (!result.second.ok()) 
    {
      return 1;
    }
  return 0;
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
int (* const actions[])(void) = { test_encode, test_decode, test_all };

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
    {
      for (size_t i=0; i<NELEM(options); ++i)
	if (0 == strcmp(options[i], argv[1]))
	  return (actions[i])();
    }
  else if (1 == argc)
    {
      // No options specified; the default is to perform the sanity test.
      return test_all();
    }
  else
    {
      usage(argv[0] ? argv[0] : "encoding-test");
    }
  return 1;
}
