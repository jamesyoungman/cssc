/*
 * bodyio.h: Part of GNU CSSC.
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

 * See also bodyio.cc, sf-admin.cc and encoding.cc.
 *
 */
#ifndef CSSC_INC_BODYIO_H
#define CSSC_INC_BODYIO_H 1

#include <cstdio>

bool body_insert_text(const char iname[], const char oname[],
		      FILE *in, FILE *out,
		      unsigned long int *lines,
		      bool *idkw, bool *binary, bool *fail);

bool body_insert_binary(const char iname[], const char oname[],
			FILE *in, FILE *out,
			unsigned long int *lines,
			bool *idkw);

bool body_insert(bool *binary,
		 const char iname[], const char oname[],
		 FILE *in, FILE *out,
		 unsigned long int *lines,
		 bool *idkw);


// Encoding functions...
void encode_line(const char in[], char out[], size_t len);
size_t decode_line(const char in[], char out[]);
int encode_stream(FILE *fin, FILE *fout); //encodes whole file.


// Decoding (output) functions
class cssc_linebuf;
int output_body_line_text  (FILE *fp, const cssc_linebuf*);
int output_body_line_binary(FILE *fp, const cssc_linebuf*);


bool check_id_keywords(const char *s, size_t len);

int encode_file(const char* name_in, const char* name_out);

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



#endif
