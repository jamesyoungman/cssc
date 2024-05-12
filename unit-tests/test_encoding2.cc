/*
 * test_encoding2.cc: Part of GNU CSSC.
 *
 * Copyright (C) 1997, 1998, 2007, 2010, 2011, 2014, 2019, 2024 Free
 * Software Foundation, Inc.
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

#include <gtest/gtest.h>

TEST(EncodingTest, ShortInput)
{
  // The encoding scheme normally takes input in groups of 3 bytes.
  // Hence we had a problem if the input was only 2 bytes (specificaly
  // a branch on uninitialised data detected by valgrind).
  char *in = (char*)malloc(3);
  char out[6];

  in[0] = '%';
  in[1] = 'A';
  // in[2] is uninitialised.
  encode_line (in, out, 2);
  // You need to leave this output statement here, because
  // unless we use the result of a computation on uninitialised data,
  // valgrind will not trigger.
  fprintf (stderr, "%s\n", out);


  in[0] = in[1] = in[2] = 0;
  int chars = decode_line (out, in);
  ASSERT_EQ(2, chars);
  ASSERT_EQ('%', in[0]);
  ASSERT_EQ('A', in[1]);
}
