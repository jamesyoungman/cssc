/*
 * test-relvbr.cc: Part of GNU CSSC.
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
 * Unit tests for relvbr.h.
 *
 */
#include "relvbr.h"
#include <gtest/gtest.h>


TEST(TestRelvbr, Constructor) 
{
  relvbr a;
  ASSERT_FALSE(a.valid());
  
  relvbr b(1, 2, 3);
  ASSERT_TRUE(b.valid());
  
  ASSERT_FALSE(relvbr(1, 0, 0).valid());
  ASSERT_FALSE(relvbr(1, 2, 0).valid());
  ASSERT_TRUE( relvbr(1, 2, 3).valid());

  ASSERT_TRUE(relvbr(1,2,3) == relvbr("1.2.3"));
}

TEST(TestRelvbr, Less)
{
  ASSERT_TRUE(relvbr(1, 1, 1) < relvbr(2, 3, 4));
  ASSERT_TRUE(relvbr(1, 1, 1) < relvbr(1, 2, 3));
  ASSERT_TRUE(relvbr(1, 1, 1) < relvbr(1, 1, 3));

  ASSERT_FALSE(relvbr(1, 1, 1) < relvbr(1, 1, 1));

  ASSERT_FALSE(relvbr(2, 3, 4) < relvbr(1, 1, 1));
  ASSERT_FALSE(relvbr(1, 2, 3) < relvbr(1, 1, 1));
  ASSERT_FALSE(relvbr(1, 1, 3) < relvbr(1, 1, 1));
}

TEST(TestRelvbr, Greater)
{
  ASSERT_FALSE(relvbr(1, 1, 1) > relvbr(2, 3, 4));
  ASSERT_FALSE(relvbr(1, 1, 1) > relvbr(1, 2, 3));
  ASSERT_FALSE(relvbr(1, 1, 1) > relvbr(1, 1, 3));

  ASSERT_FALSE(relvbr(1, 1, 1) > relvbr(1, 1, 1));

  ASSERT_TRUE(relvbr(2, 3, 4) > relvbr(1, 1, 1));
  ASSERT_TRUE(relvbr(1, 2, 3) > relvbr(1, 1, 1));
  ASSERT_TRUE(relvbr(1, 1, 3) > relvbr(1, 1, 1));
}

TEST(TestRelvbr, LessEqual)
{
  ASSERT_TRUE(relvbr(1, 1, 1) <= relvbr(2, 3, 4));
  ASSERT_TRUE(relvbr(1, 1, 1) <= relvbr(1, 2, 3));
  ASSERT_TRUE(relvbr(1, 1, 1) <= relvbr(1, 1, 3));

  ASSERT_TRUE(relvbr(1, 1, 1) <= relvbr(1, 1, 1));
  ASSERT_TRUE(relvbr(1, 2, 3) <= relvbr(1, 2, 3));

  ASSERT_FALSE(relvbr(2, 3, 4) <= relvbr(1, 1, 1));
  ASSERT_FALSE(relvbr(1, 2, 3) <= relvbr(1, 1, 1));
  ASSERT_FALSE(relvbr(1, 1, 3) <= relvbr(1, 1, 1));
}

TEST(TestRelvbr, GreaterEqual)
{
  ASSERT_FALSE(relvbr(1, 1, 1) >= relvbr(2, 3, 4));
  ASSERT_FALSE(relvbr(1, 1, 1) >= relvbr(1, 2, 3));
  ASSERT_FALSE(relvbr(1, 1, 1) >= relvbr(1, 1, 3));

  ASSERT_TRUE(relvbr(1, 1, 1) >= relvbr(1, 1, 1));
  ASSERT_TRUE(relvbr(1, 2, 3) >= relvbr(1, 2, 3));

  ASSERT_TRUE(relvbr(2, 3, 4) >= relvbr(1, 1, 1));
  ASSERT_TRUE(relvbr(1, 2, 3) >= relvbr(1, 1, 1));
  ASSERT_TRUE(relvbr(1, 1, 3) >= relvbr(1, 1, 1));
}

TEST(TestRelvbr, Equal)
{
  ASSERT_FALSE(relvbr(1, 1, 1) == relvbr(2, 1, 1));
  ASSERT_FALSE(relvbr(1, 1, 1) == relvbr(1, 2, 1));
  ASSERT_FALSE(relvbr(1, 1, 1) == relvbr(1, 1, 2));

  ASSERT_TRUE(relvbr(1, 1, 1) == relvbr(1, 1, 1));
  ASSERT_TRUE(relvbr(1, 2, 3) == relvbr(1, 2, 3));
}

TEST(TestRelvbr, NotEqual)
{
  ASSERT_TRUE(relvbr(1, 1, 1) != relvbr(2, 1, 1));
  ASSERT_TRUE(relvbr(1, 1, 1) != relvbr(1, 2, 1));
  ASSERT_TRUE(relvbr(1, 1, 1) != relvbr(1, 1, 2));

  ASSERT_FALSE(relvbr(1, 1, 1) != relvbr(1, 1, 1));
  ASSERT_FALSE(relvbr(1, 2, 3) != relvbr(1, 2, 3));
}
