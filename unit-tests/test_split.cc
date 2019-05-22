/*
 * test-split.cc: Part of GNU CSSC.

 *  Copyright (C) 2019 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Unit tests for l-split.cc.
 *
 */
#include <config.h>
#include "l-split.h"
#include <gtest/gtest.h>
#include <limits>

TEST(SplitStringTest, Empty) {
  std::vector<std::string> fields;
  const std::string input;
  auto result = split_string(input.begin(), input.end(), 'x', &fields);
  EXPECT_EQ(result, input.end());
  ASSERT_EQ(1, fields.size());
  EXPECT_EQ("", fields[0]);
}

TEST(SplitStringTest, One) {
  std::vector<std::string> fields;
  const std::string input = "hello";
  auto result = split_string(input.begin(), input.end(), ':', &fields);
  EXPECT_EQ(result, input.end());
  ASSERT_EQ(1, fields.size());
  EXPECT_EQ("hello", fields[0]);
}

TEST(SplitStringTest, TwoBasic) {
  std::vector<std::string> fields;
  const std::string input = "hello:world";
  auto result = split_string(input.begin(), input.end(), ':', &fields);
  EXPECT_EQ(input.end(), result);
  ASSERT_EQ(2, fields.size());
  EXPECT_EQ("hello", fields[0]);
  EXPECT_EQ("world", fields[1]);
}

TEST(SplitStringTest, BothEmpty) {
  std::vector<std::string> fields;
  const std::string input = ":";
  auto result = split_string(input.begin(), input.end(), ':', &fields);
  EXPECT_EQ(input.end(), result);
  EXPECT_EQ(2, fields.size());
  ASSERT_GE(fields.size(), 1);	// ensure we don't overflow fields[].
  EXPECT_EQ("", fields[0]);
  ASSERT_EQ(2, fields.size());
  EXPECT_EQ("", fields[1]);
}

TEST(SplitStringTest, SecondEmpty) {
  std::vector<std::string> fields;
  const std::string input = "yes:";
  auto result = split_string(input.begin(), input.end(), ':', &fields);
  EXPECT_EQ(input.end(), result);
  ASSERT_EQ(2, fields.size());
  EXPECT_EQ("yes", fields[0]);
  EXPECT_EQ("", fields[1]);
}

TEST(SplitStringTest, MiddleEmpty) {
  std::vector<std::string> fields;
  const std::string input = "yes::no";
  auto result = split_string(input.begin(), input.end(), ':', &fields);
  EXPECT_EQ(input.end(), result);
  ASSERT_EQ(3, fields.size());
  EXPECT_EQ("yes", fields[0]);
  EXPECT_EQ("", fields[1]);
  EXPECT_EQ("no", fields[2]);
}



TEST(SplitStringTest, BasicLimitExample) {
  std::vector<std::string> fields;
  const std::string input = "ab:cde:fghi:j";
  auto result = split_string(input.begin(), input.end(), ':', &fields, 2);
  ASSERT_EQ(2, fields.size());
  EXPECT_EQ("ab", fields[0]);
  EXPECT_EQ("cde", fields[1]);
  EXPECT_EQ(7, result - input.begin());
}


TEST(SplitStringTest, LimitZero) {
  std::vector<std::string> fields;
  const std::string input = "a:b:c";
  auto result = split_string(input.begin(), input.end(), ':', &fields, 0);
  EXPECT_EQ(input.begin(), result);
  ASSERT_EQ(0, fields.size());
}
