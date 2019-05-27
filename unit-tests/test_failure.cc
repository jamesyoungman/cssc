/*
 * test_failure.cc: Part of GNU CSSC.
 *
 * Copyright (C) 2019 Free Software Foundation, Inc.
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
 * Unit tests for failure.h.
 *
 */
#include "failure.h"

#include <gtest/gtest.h>

using cssc::make_error_condition;
using cssc::make_error_code;
using cssc::errorcode;
using cssc::condition;

TEST(ConditionTest, BodyIsBinary)
{
  EXPECT_EQ(make_error_code(errorcode::ControlCharacterAtStartOfLine),
	    make_error_condition(condition::BodyIsBinary));
  EXPECT_EQ(make_error_condition(condition::BodyIsBinary),
	    make_error_code(errorcode::ControlCharacterAtStartOfLine));

  EXPECT_EQ(make_error_code(errorcode::BodyLineTooLong),
	    make_error_condition(condition::BodyIsBinary));
  EXPECT_EQ(make_error_condition(condition::BodyIsBinary),
	    make_error_code(errorcode::BodyLineTooLong));
  
  EXPECT_EQ(make_error_code(errorcode::FileDoesNotEndWithNewline),
	    make_error_condition(condition::BodyIsBinary));
  EXPECT_EQ(make_error_condition(condition::BodyIsBinary),
	    make_error_code(errorcode::FileDoesNotEndWithNewline));

  EXPECT_NE(make_error_code(errorcode::NotAnSccsHistoryFile),
	    make_error_condition(condition::BodyIsBinary));
  EXPECT_NE(make_error_condition(condition::BodyIsBinary),
	    make_error_code(errorcode::NotAnSccsHistoryFile));
}

TEST(CategoryTest, Name)
{
  const std::string cssc = "cssc";
  auto code = make_error_code(errorcode::NotAnSccsHistoryFile);
  EXPECT_EQ(std::string(code.category().name()), cssc);
  auto cond = make_error_condition(condition::BodyIsBinary);
  EXPECT_EQ(std::string(cond.category().name()), cssc);
}

TEST(FailureTest, FactoryFunctions)
{
  auto f = make_failure(errorcode::FileDoesNotEndWithNewline);
  ASSERT_FALSE(f.ok());
  ASSERT_EQ(f.code(), make_error_code(errorcode::FileDoesNotEndWithNewline));
  ASSERT_NE(f.code(), make_error_code(errorcode::DeclineToOverwriteOutputFile));
  ASSERT_NE(f.to_string().find("newline"), std::string::npos)
    << "expected to see 'newline' in '" << f.to_string() << "'";
  ASSERT_TRUE(f.detail().empty())
    << "expected detail string to be empty but got: " << f.detail().empty();

  auto f_withdetail = make_failure(errorcode::FileDoesNotEndWithNewline,
				   "it ends with a song-and-dance routine");
  ASSERT_FALSE(f_withdetail.ok());
  ASSERT_NE(f_withdetail.to_string().find("song-and-dance"), std::string::npos)
    << "expected to_string() string to contain 'song-and-dance' but got '"
    << f_withdetail.to_string() << "'";
  ASSERT_NE(f_withdetail.detail().find("song-and-dance"), std::string::npos)
    << "expected detail() string to contain 'song-and-dance' but got '"
    << f_withdetail.detail() << "'";
}

TEST(FailureTest, Ok)
{
  auto ok = cssc::Failure::Ok();
  ASSERT_TRUE(ok.ok());
  ASSERT_NE(ok.code(), make_error_code(errorcode::FileDoesNotEndWithNewline));
  ASSERT_FALSE(ok.code());
}

TEST(FailureTest, Update)
{
  // The important property that Update should provide is that an Ok
  // value is overwritten by a non-Ok value, but a non-Ok value is
  // never overwritten by anything.
  auto ok = cssc::Failure::Ok();
  ASSERT_TRUE(ok.ok());
  auto f = make_failure(errorcode::NotAnSccsHistoryFile, "it's a horse");
  ASSERT_FALSE(f.ok());

  auto f2 = cssc::Update(ok, f);
  ASSERT_FALSE(f2.ok());
  ASSERT_TRUE(ok.ok());
  ASSERT_EQ(f2.code(), f.code());
  ASSERT_EQ(f2.code(), make_error_code(errorcode::NotAnSccsHistoryFile));
  ASSERT_NE(f2.to_string().find("horse"), std::string::npos)
    << "'" << f2.to_string() << "' should contain 'horse'";

  auto f3 = make_failure(errorcode::FileHasHardLinks, "random");
  auto f4 = cssc::Update(f2, f3); // should leave f4 like f2, not f3.
  ASSERT_EQ(f4.code(), f2.code());
  ASSERT_NE(f4.code(), f3.code());
}

TEST(FailureTest, Errno)
{
  auto f = cssc::make_failure_from_errno(ENOENT, "marshmallow-castle");
  ASSERT_EQ(f.code(), std::errc::no_such_file_or_directory);
  ASSERT_NE(f.to_string().find("marshmallow"), std::string::npos)
    << "'" << f.to_string() << "' should contain 'marshmallow'";
}

TEST(FailureBuilderTest, Basic)
{
  const auto fb = cssc::make_failure_builder(errorcode::NotAnSccsHistoryFile)
    << "donkey";
  const cssc::Failure f = fb;
  ASSERT_EQ(f.code(), make_error_code(errorcode::NotAnSccsHistoryFile));
  ASSERT_NE(f.to_string().find("donkey"), std::string::npos)
    << "'" << f.to_string() << "' should contain 'donkey'";
}

TEST(FailureBuilderTest, Insertion)
{
  cssc::Failure f =
    cssc::make_failure_builder(errorcode::NotAnSccsHistoryFile)
    << "ice-cream";
  ASSERT_EQ(f.code(), make_error_code(errorcode::NotAnSccsHistoryFile));
  ASSERT_NE(f.to_string().find("ice-cream"), std::string::npos)
    << "'" << f.to_string() << "' should contain 'ice-cream'";
}

TEST(FailureBuilderTest, AutoConversion)
{
  // This just needs to compile.
  cssc::Failure f = cssc::make_failure_builder(errorcode::NotAnSccsHistoryFile);

  cssc::Failure f2 = cssc::make_failure_builder(errorcode::NotAnSccsHistoryFile)
    << "also insert a string";
}
