/*
 * test_linebuf.cc: Part of GNU CSSC.
 *
 * Copyright (C) 2011, 2014, 2019 Free Software Foundation, Inc.
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
 *
 * Unit tests for cssc_linebuf.
 *
 */
#include <config.h>
#include <stdio.h>
#include "linebuf.h"
#include <gtest/gtest.h>

class LineBufTest : public ::testing::Test {
public:
  cssc_linebuf three_colon;
  cssc_linebuf two_slash_with_null;

  static FILE * MakeFile(const char *data, size_t len)
  {
    FILE *fp;

    fp = tmpfile();
    fwrite (data, 1, len, fp);
    rewind (fp);
    return fp;
  }
  
  virtual void SetUp() 
  {
    FILE *fp;

    fp = MakeFile("hello:there:world", 17);
    ASSERT_EQ(0, three_colon.read_line(fp));
    fclose (fp);

    fp = MakeFile ("one\0two/three\n", 14);
    ASSERT_EQ(0, two_slash_with_null.read_line(fp));
    fclose (fp);
  }
};

TEST_F(LineBufTest, InitialState) {
  cssc_linebuf empty;
  const char *p = empty.c_str();
  EXPECT_NE((const char*)0, p);
  /* content is not guaranteed at all. */
}

TEST_F(LineBufTest, ArrayAccess) {
  EXPECT_EQ('h', three_colon[0]);
  EXPECT_EQ('e', three_colon[1]);
  EXPECT_EQ('\0', three_colon[17]);
}

TEST_F(LineBufTest, C_Str) {
  const char *p = three_colon.c_str();
  EXPECT_EQ('h', p[0]);
  EXPECT_EQ('e', p[1]);
  EXPECT_EQ('\0', p[17]);

  const cssc_linebuf& ref(three_colon);
  const char *cp = ref.c_str();
  EXPECT_EQ('h', cp[0]);
  EXPECT_EQ('e', cp[1]);
  EXPECT_EQ('\0', cp[17]);
}

TEST_F(LineBufTest, SetChar) {
  EXPECT_EQ('h', three_colon[0]);
  EXPECT_EQ('e', three_colon[1]);
  three_colon.set_char(0, 'y');
  EXPECT_EQ('y', three_colon[0]);
  EXPECT_EQ('e', three_colon[1]);
}

TEST_F(LineBufTest, SplitBasic) {
  int num_fields;
  char *fields[5];
  char dummy;
  fields[4] = 0;
  fields[3] = &dummy;
  num_fields = three_colon.split(0, fields, 4, ':');
  ASSERT_EQ(3, num_fields);
  EXPECT_EQ(0, strcmp("hello", fields[0])) << "fields[0] is" << fields[0];
  EXPECT_EQ(0, strcmp("there", fields[1])) << "fields[1] is" << fields[1];
  EXPECT_EQ(0, strcmp("world", fields[2])) << "fields[2] is" << fields[2];
  /* Check that fields[] is not assigned to beyond the specified limit. */
  EXPECT_EQ(&dummy, fields[3]);
}

TEST_F(LineBufTest, SplitMissing) {
  int num_fields;
  char *fields[2];
  /* Same again, using a delimiter not in the string. */
  num_fields = three_colon.split(0, fields, 2, '/');
  ASSERT_EQ(1, num_fields);
  EXPECT_EQ(0, strcmp("hello:there:world", fields[0]));
}

TEST_F(LineBufTest, SplitOffset) {
  int num_fields;
  char *fields[3];
  /* Same again, using a delimiter not in the string. */
  num_fields = three_colon.split(3, fields, 3, ':');
  ASSERT_EQ(3, num_fields);
  EXPECT_EQ(0, strcmp("lo",    fields[0])) << "fields[0] is" << fields[0];
  EXPECT_EQ(0, strcmp("there", fields[1])) << "fields[1] is" << fields[1];
  EXPECT_EQ(0, strcmp("world", fields[2])) << "fields[2] is" << fields[2];
}

TEST_F(LineBufTest, SplitLimit) {
  int num_fields;
  char *fields[2];
  /* Same again, using a delimiter not in the string. */
  num_fields = three_colon.split(0, fields, 1, ':');
  ASSERT_EQ(1, num_fields);
  EXPECT_EQ(0, strcmp("hello", fields[0])) << "fields[0] is" << fields[0];
}

TEST_F(LineBufTest, Keywords) {
  EXPECT_EQ(0, three_colon.check_id_keywords());
  EXPECT_EQ(0, two_slash_with_null.check_id_keywords());
  cssc_linebuf kwbuf;
  FILE *fp = MakeFile ("hello %M% world\n", 16);
  ASSERT_EQ(0, kwbuf.read_line(fp));
  fclose (fp);
  EXPECT_EQ(1, kwbuf.check_id_keywords());
}

TEST_F(LineBufTest, Write) {
  FILE *fp = tmpfile();
  three_colon.write(fp);
  three_colon.set_char(0, 't');
  rewind (fp);
  three_colon.write(fp);
  rewind (fp);
  /* Now read back the modified data. */
  cssc_linebuf buf;
  ASSERT_EQ(0, buf.read_line(fp));
  EXPECT_EQ('t', buf[0]);
  EXPECT_EQ('e', buf[1]);
  fclose (fp);
}

TEST_F(LineBufTest, ReadLineSequence) {
  FILE *fp = tmpfile();
  fprintf (fp, "one\ntwo\nthree\n");
  rewind (fp);
  cssc_linebuf b;
  
  ASSERT_EQ(0, b.read_line(fp));
  EXPECT_EQ(0, strcmp(b.c_str(), "one\n"))
    << "actual value was '" << b.c_str() << "'";
  
  ASSERT_EQ(0, b.read_line(fp));
  EXPECT_EQ(0, strcmp(b.c_str(), "two\n"))
    << "actual value was '" << b.c_str() << "'";

  ASSERT_EQ(0, b.read_line(fp));
  EXPECT_EQ(0, strcmp(b.c_str(), "three\n"))
    << "actual value was '" << b.c_str() << "'";

  /* Make sure we also detect EOF. */
  ASSERT_EQ(1, b.read_line(fp));
  fclose (fp);
}
