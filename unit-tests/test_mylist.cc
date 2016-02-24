/*
 * test-mylist.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 2010 Free Software Foundation, Inc.
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
 * Unit tests for template mylist.
 *
 */
#include <config.h>
#include "mylist.h"
#include <gtest/gtest.h>

TEST(MylistTest, Empty) {
  mylist<int> empty;
  const int len = empty.size();

  EXPECT_EQ(0, len);
}

TEST(MylistTest, PushBack) {
  mylist<int> a;

  a.push_back(42);
  EXPECT_EQ(1, a.size());
  EXPECT_EQ(42, a[0]);
}

TEST(MylistTest, Assignment) {
  mylist<int> a;
  mylist<int> b;

  a.push_back(42);
  b = a;
  EXPECT_EQ(1, b.size());
  EXPECT_EQ(42, b[0]);
  // Make sure a was not destroyed.
  EXPECT_EQ(42, a[0]);
}

TEST(MylistTest, Catenate) {
  mylist<int> a, b;

  a.push_back(42);
  b.push_back(53);
  b.push_back(54);
  EXPECT_EQ(1, a.size());
  EXPECT_EQ(2, b.size());
  a.insert(a.end(), b.cbegin(), b.cend());
  EXPECT_EQ(3, a.size());
  EXPECT_EQ(42, a[0]);
  EXPECT_EQ(53, a[1]);
  EXPECT_EQ(54, a[2]);
  EXPECT_EQ(2, b.size());
}

TEST(MylistTest, EqualityEmpty)
{
  mylist<int> a, b, c;
  EXPECT_TRUE(a.size() == b.size());
  EXPECT_TRUE(a == b);

  c.push_back(4);
  EXPECT_FALSE(a == c);
  EXPECT_FALSE(c == a);
}

TEST(MylistTest, EqualityNonEmpty)
{
  mylist<int> a, b;
  a.push_back(1);
  a.push_back(2);
  b.push_back(1);
  b.push_back(2);
  EXPECT_TRUE(a == b);
}

TEST(MylistTest, EqualityLengthDifferent)
{
  mylist<int> a, b;
  a.push_back(1);
  a.push_back(2);
  b.push_back(1);
  EXPECT_FALSE(a == b);
}

TEST(MylistTest, Different)
{
  mylist<int> a, b;
  a.push_back(1);
  a.push_back(2);
  b.push_back(2);
  b.push_back(1);
  EXPECT_FALSE(a == b);
}
