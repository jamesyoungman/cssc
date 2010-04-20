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
#include "cssc.h"
#include "mylist.h"
#include <gtest/gtest.h>


class NoCopyConstructor
{
  int value_;
  
public:
  NoCopyConstructor(int val = 1) 		// default constructor
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


TEST(MylistTest, Empty) {
  mylist<int> empty;
  const int len = empty.length();
  
  EXPECT_EQ(0, len);
}

TEST(MylistTest, Add) {
  mylist<int> a;

  a.add(42);
  EXPECT_EQ(1, a.length());
  EXPECT_EQ(42, a[0]);
}

TEST(MylistTest, Assignment) {
  mylist<int> a;
  mylist<int> b;

  a.add(42);
  b = a;
  EXPECT_EQ(1, b.length());
  EXPECT_EQ(42, b[0]);
  // Make sure a was not destroyed.
  EXPECT_EQ(42, a[0]);
}

TEST(MylistTest, Select) {
  mylist<int> a;

  a.add(42);
  EXPECT_EQ(1, a.length());
  EXPECT_EQ(42, a[0]);
  a.select(0) = 52;
  EXPECT_EQ(52, a[0]);
}

TEST(MylistTest, Catenate) {
  mylist<int> a, b;

  a.add(42);
  b.add(53);
  b.add(54);
  a += b;
  EXPECT_EQ(3, a.length());
  EXPECT_EQ(42, a[0]);
  EXPECT_EQ(53, a[1]);
  EXPECT_EQ(54, a[2]);
  EXPECT_EQ(2, b.length());
}

TEST(MylistTest, MinusEmpty) {
  mylist<int> a, empty;

  a.add(42);
  a -= empty;
  EXPECT_EQ(1, a.length());
  EXPECT_EQ(42, a[0]);

  empty -=a ;
  EXPECT_EQ(0, empty.length());
  EXPECT_EQ(1, a.length());
  
}

TEST(MylistTest, Minus) {
  mylist<int> a, b;

  a.add(42);
  b.add(53);
  b.add(54);
  a -= b;
  EXPECT_EQ(1, a.length());
  b.add(42);
  a.add(96);
  a -= b;
  EXPECT_EQ(1, a.length());
  EXPECT_EQ(96, a[0]);
}

TEST(MylistTest, AssignmentDoesNotNeedElementCopyConstructor) {
  mylist<NoCopyConstructor> a;
  mylist<NoCopyConstructor> b;
  NoCopyConstructor val(12);

  // Make sure we can perform an assignment of a mylist<T> when 
  // T has a private copy constructor.
  a.add(val);
  b = a;
}

