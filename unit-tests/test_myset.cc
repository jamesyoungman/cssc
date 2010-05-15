/*
 * test-myset.cc: Part of GNU CSSC.
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
 * Unit tests for template myset.
 *
 */
#include "myset.h"
#include "mylist.h"
#include <gtest/gtest.h>


class NoCopyConstructor
{
  int value_;

public:
  NoCopyConstructor(int val = 1)                // default constructor
    : value_(val)
  {
  }


  NoCopyConstructor &operator =(NoCopyConstructor const &other)
  {
    value_ = other.value_;
  }

private:
  // The copy constructor is private.
  NoCopyConstructor(const NoCopyConstructor& other)
    : value_(other.value_)
  {
  }
};


TEST(MysetTest, Empty) {
  myset<int> empty;
  const myset<int>::my_size_type len = empty.count();

  EXPECT_EQ(0u, len);
}

TEST(MysetTest, Add) {
  myset<int> a;

  a.add(42);
  EXPECT_EQ(1u, a.count());
  EXPECT_TRUE(a.is_member(42));
}

TEST(MysetTest, List) {
  myset<int> a;
  mylist<int> items;

  a.add(42);
  a.add(41);
  items = a.list();

  if (items[0] == 42)
    {
      EXPECT_EQ(42, items[0]);
      EXPECT_EQ(41, items[1]);
    }
  else
    {
      EXPECT_EQ(41, items[0]);
      EXPECT_EQ(42, items[1]);
    }
  EXPECT_EQ(2u, a.count());
}
