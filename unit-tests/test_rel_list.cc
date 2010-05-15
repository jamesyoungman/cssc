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



TEST(RLMerge, MergeBothEmpty)
{
  release_list x, y;
  ASSERT_TRUE(x.empty());
  ASSERT_TRUE(y.empty());
  x.merge(y);
  ASSERT_TRUE(x.empty());
  ASSERT_TRUE(y.empty());
}

TEST(RLMerge, MergeOntoEmpty)
{
  release_list x;
  release_list y("2,4");

  ASSERT_TRUE(y.valid());

  ASSERT_TRUE(x.empty());
  x.merge(y);
  ASSERT_FALSE(x.empty());

  ASSERT_TRUE(x.member(release(2)));
  ASSERT_TRUE(x.member(release(4)));
  ASSERT_FALSE(x.member(release(3)));
}

TEST(RLMerge, MergeDisjoint)
{
  release_list x, y("2,6");
  x.merge(y);

  ASSERT_TRUE(x.member(release(2)));
  ASSERT_TRUE(x.member(release(6)));

  // Make sure we didn't destroy the original.
  ASSERT_TRUE(y.member(release(2)));
  ASSERT_TRUE(y.member(release(6)));
}

TEST(RLMerge, MergeInterior)
{
  release_list x("2,6"), y("4");
  x.merge(y);

  ASSERT_TRUE(x.member(release(2)));
  ASSERT_TRUE(x.member(release(4)));
  ASSERT_TRUE(x.member(release(6)));
  ASSERT_FALSE(x.member(release(3)));
}

TEST(RLMerge, MergeNoOp)
{
  release_list x("2,6"), y("6,2");
  x.merge(y);

  ASSERT_TRUE(x.member(release(2)));
  ASSERT_TRUE(x.member(release(6)));
}



TEST(RLRemove, BothEmpty)
{
  release_list x, y;
  ASSERT_TRUE(x.empty());
  ASSERT_TRUE(y.empty());
  x.remove(y);
  ASSERT_TRUE(x.empty());
  ASSERT_TRUE(y.empty());
}

TEST(RLRemove, RhsEmpty)
{
  release_list x("4,8"), y;
  ASSERT_FALSE(x.empty());
  ASSERT_TRUE(y.empty());
  x.remove(y);
  ASSERT_TRUE(y.empty());
  ASSERT_TRUE(x.member(release(4)));
  ASSERT_TRUE(x.member(release(8)));
}

TEST(RLRemove, LhsEmpty)
{
  release_list x("4,8"), y;
  ASSERT_FALSE(x.empty());
  ASSERT_TRUE(y.empty());
  y.remove(x);
  ASSERT_TRUE(y.empty());
  ASSERT_TRUE(x.member(release(4)));
  ASSERT_TRUE(x.member(release(8)));
}

TEST(RLRemove, Identical)
{
  const char *content = "4,8";
  release_list x(content), y(content);
  ASSERT_FALSE(x.empty());
  ASSERT_FALSE(y.empty());
  y.remove(x);
  ASSERT_TRUE(y.empty());
}

TEST(RLRemove, RemoveSome)
{
  release_list x("4,6,8"), y("6");
  ASSERT_FALSE(x.empty());
  ASSERT_FALSE(y.empty());
  x.remove(y);
  ASSERT_TRUE(x.member(release(4)));
  ASSERT_TRUE(x.member(release(8)));
  ASSERT_FALSE(x.member(release(6)));
  ASSERT_TRUE(y.member(release(6)));
}

TEST(RLRemove, RemoveAll)
{
  release_list x("4,6,8");
  ASSERT_FALSE(x.empty());
  ASSERT_TRUE(x.member(release(4)));
  ASSERT_TRUE(x.member(release(6)));
  ASSERT_TRUE(x.member(release(8)));
  x.remove(release_list("4"));
  ASSERT_FALSE(x.empty());
  ASSERT_TRUE(x.member(release(6)));
  ASSERT_TRUE(x.member(release(8)));
  x.remove(release_list("6"));
  ASSERT_FALSE(x.empty());
  ASSERT_TRUE(x.member(release(8)));
  x.remove(release_list("8"));
  ASSERT_TRUE(x.empty());
  ASSERT_FALSE(x.member(release(8)));
}
