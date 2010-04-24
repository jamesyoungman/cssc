/*
 * test_stack.cc: Part of GNU CSSC.
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
 * Unit tests for stack.h.
 *
 */
#include "stack.h"
#include <gtest/gtest.h>


TEST(StackTest, Constructor) 
{
  stack<int> si(4);
  ASSERT_EQ(1, si.empty());
}

TEST(StackDeathTest, Capacity) 
{
  stack<int> si(2);
  ASSERT_EQ(1, si.empty());
  si.push(1);
  si.push(1);
  EXPECT_EXIT(si.push(1), ::testing::KilledBySignal(SIGABRT), "top < len");
}

TEST(StackDeathTest, Underflow) 
{
  stack<int> si(2);
  EXPECT_EXIT(si.pop(), ::testing::KilledBySignal(SIGABRT), "top > 0");
}

TEST(StackTest, PushPop) 
{
  stack<int> si(4);
  si.push(1);
  si.push(2);
  si.push(3);
  si.push(4);
  EXPECT_EQ(4, si.pop());
  EXPECT_EQ(3, si.pop());
  EXPECT_EQ(2, si.pop());
  EXPECT_EQ(1, si.pop());
  ASSERT_EQ(1, si.empty());
}

TEST(StackTest, Assignment) 
{
  stack<int> si1(4), si2(4);
  si1.push(1);
  si1.push(2);
  si1.push(3);
  si1.push(4);
  si2 = si1;
  EXPECT_EQ(4, si1.pop());
  EXPECT_EQ(3, si1.pop());
  EXPECT_EQ(2, si1.pop());
  EXPECT_EQ(1, si1.pop());
  ASSERT_EQ(1, si1.empty());

  ASSERT_EQ(0, si2.empty());
  EXPECT_EQ(4, si2.pop());
  EXPECT_EQ(3, si2.pop());
  EXPECT_EQ(2, si2.pop());
  EXPECT_EQ(1, si2.pop());
  ASSERT_EQ(1, si2.empty());
}

TEST(StackDeathTest, AssignmentChangesCapacity) 
{
  stack<int> si2(2);
  stack<int> si3(3);
  si2.push(1);
  si2.push(2);
  // si2 is now full.
  EXPECT_EXIT(si2.push(3), ::testing::KilledBySignal(SIGABRT), "top < len");
  si2 = si3;
  // si2 is now empty, but with a larger capacity.
  // We should be able to push one more item now.
  si2.push(1);
  si2.push(2);
  si2.push(3);
  // si2 is now full, again.
  EXPECT_EXIT(si2.push(4), ::testing::KilledBySignal(SIGABRT), "top < len");
}

struct HasNonTrivialCopyConstructor
{
  int val_;
  bool is_original_;

  HasNonTrivialCopyConstructor(int value)
    : val_(value),
      is_original_(true)
  {
  }
  
  HasNonTrivialCopyConstructor()
    : val_(0),
      is_original_(true)
  {
  }
  
  HasNonTrivialCopyConstructor(const HasNonTrivialCopyConstructor& other)
  {
    if (this != &other)
      {
	val_ = other.val_;
	is_original_ = false;
      }
  }
};

  
TEST(StackTest, AssignNotCopy)
{
  // Verify that things are put into the stack by assignment, not memcpy.
  stack<HasNonTrivialCopyConstructor> s(1);
  const HasNonTrivialCopyConstructor a(3);
  s.push(a);
  const HasNonTrivialCopyConstructor result(s.pop());
  ASSERT_EQ(false, result.is_original_);
  ASSERT_EQ(3, result.val_);
}
