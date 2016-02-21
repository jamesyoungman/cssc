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

TEST(MylistTest, Add) {
  mylist<int> a;

  a.add(42);
  EXPECT_EQ(1, a.size());
  EXPECT_EQ(42, a[0]);
}

TEST(MylistTest, Assignment) {
  mylist<int> a;
  mylist<int> b;

  a.add(42);
  b = a;
  EXPECT_EQ(1, b.size());
  EXPECT_EQ(42, b[0]);
  // Make sure a was not destroyed.
  EXPECT_EQ(42, a[0]);
}

TEST(MylistTest, Catenate) {
  mylist<int> a, b;

  a.add(42);
  b.add(53);
  b.add(54);
  a += b;
  EXPECT_EQ(3, a.size());
  EXPECT_EQ(42, a[0]);
  EXPECT_EQ(53, a[1]);
  EXPECT_EQ(54, a[2]);
  EXPECT_EQ(2, b.size());
}

TEST(MylistTest, MinusEmpty) {
  mylist<int> a, empty;

  a.add(42);
  a -= empty;
  EXPECT_EQ(1, a.size());
  EXPECT_EQ(42, a[0]);

  empty -=a ;
  EXPECT_EQ(0, empty.size());
  EXPECT_EQ(1, a.size());

}

TEST(MylistTest, Minus) {
  mylist<int> a, b;

  a.add(42);
  b.add(53);
  b.add(54);
  a -= b;
  EXPECT_EQ(1, a.size());
  b.add(42);
  a.add(96);
  a -= b;
  EXPECT_EQ(1, a.size());
  EXPECT_EQ(96, a[0]);
}

TEST(MylistTest, EqualityEmpty)
{
  mylist<int> a, b, c;
  EXPECT_TRUE(a.size() == b.size());
  EXPECT_TRUE(a == b);

  c.add(4);
  EXPECT_FALSE(a == c);
  EXPECT_FALSE(c == a);
}

TEST(MylistTest, EqualityNonEmpty)
{
  mylist<int> a, b;
  a.add(1);
  a.add(2);
  b.add(1);
  b.add(2);
  EXPECT_TRUE(a == b);
}

TEST(MylistTest, EqualityLengthDifferent)
{
  mylist<int> a, b;
  a.add(1);
  a.add(2);
  b.add(1);
  EXPECT_FALSE(a == b);
}

TEST(MylistTest, Different)
{
  mylist<int> a, b;
  a.add(1);
  a.add(2);
  b.add(2);
  b.add(1);
  EXPECT_FALSE(a == b);
}

TEST(MylistDeathTest, IndexTooLow)
{
  mylist<int> a;
  a.add(2);
  EXPECT_EXIT(a[-1], ::testing::KilledBySignal(SIGABRT), "index");
}

TEST(MylistDeathTest, IndexTooHigh)
{
  mylist<int> a;
  a.add(2);
  EXPECT_EXIT(a[1], ::testing::KilledBySignal(SIGABRT), "index");
}

TEST(MylistDeathTest, IndexOnEmpty)
{
  mylist<int> a;
  EXPECT_EXIT(a[0], ::testing::KilledBySignal(SIGABRT), "index");
}
