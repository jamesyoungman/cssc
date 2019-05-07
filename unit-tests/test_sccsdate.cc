/*
 * test_sccsdate.cc: Part of GNU CSSC.
 *
 *
 * Copyright (C) 2010, 2011, 2014, 2019 Free Software Foundation, Inc.
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
 * Unit tests for sccsdate.h.
 *
 */
#include "sccsdate.h"
#include "quit.h"

#include <gtest/gtest.h>

TEST(SccsdateTest, NullConstructor)
{
  const sccs_date d;
  ASSERT_FALSE(d.valid());
}

TEST(SccsdateTest, StringConstructor)
{
  sccs_date d12("990519014208");
  ASSERT_TRUE(d12.valid());
  EXPECT_EQ("99/05/19 01:42:08", d12.as_string());

  sccs_date d14("19990519014208");
  ASSERT_TRUE(d14.valid());
  EXPECT_EQ("99/05/19 01:42:08", d14.as_string());
}

TEST(SccsdateTest, StringDateTimeConstructor)
{
  sccs_date d12("99/05/19", "01:42:08");
  ASSERT_TRUE(d12.valid());
  EXPECT_EQ("99/05/19 01:42:08", d12.as_string());
}

TEST(SccsdateTest, FourDigitYear)
{
  // This test generates a warning on stderr.
  // That's OK.
  sccs_date d14("1999/05/19", "01:42:08");
  ASSERT_TRUE(d14.valid());
  EXPECT_EQ("99/05/19 01:42:08", d14.as_string());
}

TEST(SccsdateDeathTest, BuckRogers)
{
  // As a sanity check we verify that the year is within the window
  // described by the X/Open convention for handling 2-digit years.
  //
  // We could support such years quite easily, except for the fact that
  // interoperation with other versions of SCCS would become harder.
  EXPECT_EXIT(sccs_date("2429/05/19", "01:42:08"),
	      ::testing::KilledBySignal(SIGABRT),
	      "year < 2069");
}

TEST(SccsdateTest, ColonYear)
{
  // This test generates a warning on stderr.
  // That's OK.
  //
  // Some versions of SCCS roll from 99 to :0 instead of 99 to 00.
  // Yes, that's a bug in those versions of SCCS.
  // Check that we correctly convert those dates.
  sccs_date d12(":0/05/19", "01:42:08");
  ASSERT_TRUE(d12.valid());
  EXPECT_EQ("00/05/19 01:42:08", d12.as_string());
}

TEST(SccsdateTest, Now)
{
  // Make sure the now method at least returns.
  sccs_date::now();
}

TEST(SccsdateTest, Greater)
{
  EXPECT_TRUE(sccs_date("99/01/01 00:00:00") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_FALSE(sccs_date("98/01/01 00:00:00") >
	       sccs_date("99/01/01 00:00:00"));

  EXPECT_TRUE(sccs_date("98/02/01 00:00:00") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/10/01 00:00:00") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/02 00:00:00") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/10 00:00:00") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 01:00:00") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 10:00:00") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:01:00") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:10:00") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:01") >
	      sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:10") >
	      sccs_date("98/01/01 00:00:00"));

  // Leap year.
  EXPECT_TRUE(sccs_date("00/02/29 00:00:00") >
	      sccs_date("00/02/28 00:00:00"));
  EXPECT_TRUE(sccs_date("00/03/01 00:00:00") >
	      sccs_date("00/02/29 00:00:00"));
}


TEST(SccsdateTest, Less)
{
  EXPECT_FALSE(sccs_date("99/01/01 00:00:00") < sccs_date("98/01/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("99/01/01 00:00:00"));

  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/02/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/10/01 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/01/02 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/01/10 00:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/01/01 01:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/01/01 10:00:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/01/01 00:01:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/01/01 00:10:00"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/01/01 00:00:01"));
  EXPECT_TRUE(sccs_date("98/01/01 00:00:00") < sccs_date("98/01/01 00:00:10"));

  // Leap year.
  EXPECT_TRUE(sccs_date("00/02/28 00:00:00") < sccs_date("00/02/29 00:00:00"));
  EXPECT_TRUE(sccs_date("00/02/29 00:00:00") < sccs_date("00/03/01 00:00:00"));
}

TEST(SccsdateTest, Equality)
{
  const char *datestr = "99/01/01 00:00:00";

  EXPECT_FALSE(sccs_date(datestr) < sccs_date(datestr));
  EXPECT_FALSE(sccs_date(datestr) > sccs_date(datestr));

  EXPECT_TRUE(sccs_date(datestr) <= sccs_date(datestr));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  set_prg_name("test_sccsdate");
  return RUN_ALL_TESTS();
}
