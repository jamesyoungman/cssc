/*
 * uu_decode: Part of GNU CSSC.
 *
 *    Copyright (C) 1997,1998,2001,2007 Free Software Foundation, Inc. 
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
 * Some of the files in the test suite are provided uuencoded.  
 * Not all systems have uudecode.  In particular, Cygwin lacks it.
 * Hence we provide our own. 
 *
 * $Id: uu_decode.c,v 1.6 2007/12/19 00:21:14 jay Exp $
 */

/*
 * This system (uuencoding) will not work on non-ascii machines,
 * because it assumed that there is a block of printable characters
 * following the space charcter in the execution character set.
 *
 * Octal is quite convenient for thinking about uuencoding since
 * two octal digits make six bits.
 *
*/
#include "config.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/stat.h>




#define UUDEC(c)        (((c) - 040) & 077)
#define UUENC(c)        (((c) & 077) + 040)

static inline void
encode(const char in[3], char out[4])
{
  /* Notice that the bitmasks always add up to 077. */
  out[0] = UUENC(((in[0] >> 2)));
  out[1] = UUENC(((in[0] << 4) & 060) | ((in[1] >> 4) & 017));
  out[2] = UUENC(((in[1] << 2) & 074) | ((in[2] >> 6) & 003));
  out[3] = UUENC(((in[2]       & 077)));
}

static inline void 
decode(const char in[4], char out[3])
{
  /* Only the bottom six bits of t0,t1,t2,t3 are ever set,
   * but we use ints for their speed not their size.
   */
  const int t0 = UUDEC(in[0]);
  const int t1 = UUDEC(in[1]);
  const int t2 = UUDEC(in[2]);
  const int t3 = UUDEC(in[3]);

  /* Shift counts always add to six; number of bits
   * provided by each line is eight.
   *
   * A left shift of N provides (8-N) bits of value
   * and a right shift provides (6-N) bits of value.
   */
  out[0] = (t0 << 2) | (t1 >> 4); /* 6 + 2 */
  out[1] = (t1 << 4) | (t2 >> 2); /* 4 + 4 */
  out[2] = (t2 << 6) | (t3     ); /* 2 + 6 */
}


/*
 * Lines in the UUENCODEd format look like this:--
 *
 * M<F]O=#HZ,#HP.G)O;W0Z+W)O;W0Z+V)I;B]B87-H"F)I;CHJ.C$Z,3IB:6XZ
 *
 * The first character is an uppercase "M".  It's really a 
 * character count.  "M" represents the largest possible 
 * count (total line length 60 [M + 60 chars + newline]).
 */

/* decode a line, returning the number of characters in it. */
int
decode_line(const char in[], char out[])
{
  int len = UUDEC(in[0]);
  int n;
  
  if (len <= 0)
    return 0;

  ++in;                         /* step over byte count. */
  
  for (n=0; n<len; n+=3)
    {
      decode(in, out);
      in += 4;
      out += 3;
    }
  return len;
}


/* encode a line, returning the number of characters in it. */
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




int
test_encode(const char *arg)
{
    /* Rather than figure out if we support stat, just lie.
     * the test suite never uses this anyway.
     */
    int rv;
    
    printf("begin 600 %s\n", arg);
    rv = encode_stream(stdin, stdout);
    printf("end\n");
    return rv;
}

int
test_decode(const char *arg)
{
  char inbuf[80], outbuf[80];
  int mode, nf, expect_end_line;
  FILE *fp_output;

  (void) arg;

  if ( 0 != fgets(inbuf, sizeof(inbuf)-1, stdin))
    {
      nf = sscanf(inbuf, "begin %o %[^\n]", &mode, outbuf);
      if (nf < 1)
        {
          fprintf(stderr, "No \"begin\" line\n");
          return 1;
        }
      if (nf != 2)
        {
          fprintf(stderr, "No filename on \"begin\" line\n");
          return 1;
        }
      else
        {
          fp_output = fopen(outbuf, "wb");
          if (NULL == fp_output)
            {
              perror(outbuf);
              return 1;
            }
          fchmod(fileno(fp_output), mode);
        }
    }

  expect_end_line = 0;
  while ( 0 != fgets(inbuf, sizeof(inbuf)-1, stdin) )
    {
        if (expect_end_line)
        {
            if (0 != strcmp(inbuf, "end\n"))
            {
                fprintf(stderr, "Expected \"end\" line\n");
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            int len = decode_line(inbuf, outbuf);
            if (0 == len)
            {
                expect_end_line = 1;
            }
            else
            {
                fwrite(outbuf, 1, len, fp_output);
            }
        }
    }
  
  if (errno)
      perror("Error reading input file");
  else
      fprintf(stderr, "Unexcpectedly reached end-of-file\n");
  return 1;
}


/* Test all possible inputs for encode(); decode its
 * outputs and check that they decode back to the correct value.
 */
int test_all(const char *arg)
{
  union lunch { long l; char ch[4]; } in, out;
  long l0, l1, l2, lo;
  long i;

  (void) arg;

  /* i only has to hold a 24-bit value. */
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
      l0 = ((unsigned char) in.ch[0]) & 0xff;
      l1 = ((unsigned char) in.ch[1]) & 0xff;
      l2 = ((unsigned char) in.ch[2]) & 0xff;
      lo = l0 | l1<<8 | l2<<16;

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
int (* const actions[])(const char *) = { test_encode, test_decode, test_all };

#define NELEM(array)   (sizeof(array)/sizeof(array[0]))


static void
usage(const char *prog)
{
  size_t i;
  fprintf(stderr, "Usage: %s [", prog ? prog : "uu_decode");
  for (i=0; i<NELEM(options); ++i)
    {
      fprintf(stderr, "%s %s", (i>0) ? " |" : "", options[i]);
    }
  fprintf(stderr, " ]\n");
}

int
main(int argc, char *argv[])
{
  size_t i;
  const char *argument;
  
  if (argc == 3)
  {
      argument = argv[2];
  }
  else if (argc == 2)
  {
      argument = NULL;
  }
  else 
  {
      usage(argv[0]);
      return 1;
  }
  
  for (i=0; i<NELEM(options); ++i)
  {
      if (0 == strcmp(options[i], argv[1]))
      {
          return (actions[i])(argument);
      }
  }
  

  fprintf(stderr, "Unknown option %s\n", argv[1]);
  usage(argv[0]);
  return 1;
}


