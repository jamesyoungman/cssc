/*
 * test_rel_list.cc: Part of GNU CSSC.
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
 * Unit tests for rel_list.h.
 *
 */
#include "cssc.h"
#include "rel_list.h"
#include <gtest/gtest.h>


TEST(RelListTest, NullConstructor) 
{
  release_list x;
  ASSERT_TRUE(x.empty());
  ASSERT_FALSE(x.valid());
}

TEST(RelListTest, StringConstructor)
{
  const release_list x("2,4");
  ASSERT_TRUE(x.valid());
  ASSERT_FALSE(x.empty());

  ASSERT_FALSE(x.member(release(1)));
  ASSERT_TRUE(x.member(release(2)));
  ASSERT_FALSE(x.member(release(3)));
  ASSERT_TRUE(x.member(release(4)));
  ASSERT_FALSE(x.member(release(5)));
}

TEST(RelListTest, RefConstructor)
{
  const release_list y("2,4");
  ASSERT_TRUE(y.valid());
  ASSERT_FALSE(y.empty());

  const release_list x(y);
  ASSERT_TRUE(x.valid());
  ASSERT_FALSE(x.empty());

  ASSERT_FALSE(x.member(release(1)));
  ASSERT_TRUE(x.member(release(2)));
  ASSERT_FALSE(x.member(release(3)));
  ASSERT_TRUE(x.member(release(4)));
  ASSERT_FALSE(x.member(release(5)));
}

TEST(RelListTest, Membership)
{
  const release_list y("2,4");
  ASSERT_FALSE(y.member(release(1)));
  ASSERT_TRUE(y.member(release(2)));
  ASSERT_FALSE(y.member(release(3)));
  ASSERT_TRUE(y.member(release(4)));
  ASSERT_FALSE(y.member(release(5)));
}

TEST(RelListTest, MembershipOreder)
{
  const release_list y("4,2");
  ASSERT_FALSE(y.member(release(1)));
  ASSERT_TRUE(y.member(release(2)));
  ASSERT_FALSE(y.member(release(3)));
  ASSERT_TRUE(y.member(release(4)));
  ASSERT_FALSE(y.member(release(5)));
}


// FIXME: release_list::merge and release_list::remove are
// declared but not defined; remove the declarations too.
