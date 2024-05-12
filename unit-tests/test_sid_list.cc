/*
 * test_sid_list.cc: Part of GNU CSSC.
 *
 *
 * Copyright (C) 2010, 2011, 2014, 2019, 2024 Free Software Foundation,
 * Inc.
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
 * Unit tests for sid_list.h.
 *
 */
#include "sid_list.h"
#include "sl-merge.h"

#include <gtest/gtest.h>


TEST(SidListTest, NullConstructor)
{
  sid_list x;
  ASSERT_TRUE(x.valid());
  ASSERT_TRUE(x.empty());
}

TEST(SidListTest, OtherConstructors)
{
  sid_list a;
  ASSERT_TRUE(a.valid());
  ASSERT_TRUE(a.empty());
  sid_list b(a);
  ASSERT_TRUE(b.empty());
  ASSERT_TRUE(b.valid());
  sid_list c = a;
  ASSERT_TRUE(c.empty());
  ASSERT_TRUE(c.valid());
}

TEST(SidListTest, Invalidate)
{
  sid_list x;
  ASSERT_TRUE(x.valid());
  x.invalidate();
  ASSERT_FALSE(x.valid());
}

TEST(SidListTest, OpenRange)
{
  /* SourceForge bug number #438857:
   * ranges like "1.1.1.2," cause an assertion
   * failure while SCCS just ignores the empty list item.
   */
  const sid_list x("1.1.1.2,");
  ASSERT_TRUE(x.valid());

  const sid s("1.1.1.2");
  ASSERT_TRUE(x.member(s));
}

TEST(SidListTest, SingleItem)
{
  const sid_list x("1.7");
  ASSERT_TRUE(x.valid());
  ASSERT_FALSE(x.empty());
}

TEST(SidListTest, CommaSeparatedItems)
{
  const sid_list x("1.1.1.2,2.2.2.3");
  ASSERT_TRUE(x.valid());

  const sid s("1.1.1.2");
  const sid t("2.2.2.3");
  ASSERT_TRUE(x.member(s));
  ASSERT_TRUE(x.member(t));
}

TEST(SidListTest, OneRangeTrunk)
{
  const sid_list x("1.1-1.8");
  ASSERT_TRUE(x.valid());

  ASSERT_TRUE(x.member(sid("1.2")));
  ASSERT_TRUE(x.member(sid("1.3")));
  ASSERT_TRUE(x.member(sid("1.8")));
  ASSERT_FALSE(x.member(sid("1.9")));
  ASSERT_FALSE(x.member(sid("1.2.1.1")));
}

TEST(SidListTest, OneRangeBranch)
{
  const sid_list x("1.2.1.1-1.2.1.4");
  ASSERT_TRUE(x.valid());

  ASSERT_FALSE(x.member(sid("1.2")));
  ASSERT_TRUE(x.member(sid("1.2.1.1")));
  ASSERT_TRUE(x.member(sid("1.2.1.2")));
  ASSERT_TRUE(x.member(sid("1.2.1.3")));
  ASSERT_TRUE(x.member(sid("1.2.1.4")));
  ASSERT_FALSE(x.member(sid("1.2.1.5")));
  ASSERT_FALSE(x.member(sid("1.2")));
}

TEST(SidListTest, BackwardRange)
{
  const sid_list x("1.2.1.4-1.2.1.1");
  ASSERT_FALSE(x.valid());
}

TEST(SidListTest, Overlap)
{
  const sid_list x("1.2.1.1-1.2.1.9,1.2.1.7-1.2.1.14");
  ASSERT_TRUE(x.valid());
  ASSERT_TRUE(x.member(sid("1.2.1.1")));
  ASSERT_TRUE(x.member(sid("1.2.1.7")));
  ASSERT_TRUE(x.member(sid("1.2.1.9")));
  ASSERT_TRUE(x.member(sid("1.2.1.10")));
  ASSERT_TRUE(x.member(sid("1.2.1.14")));
  ASSERT_FALSE(x.member(sid("1.2.1.15")));
}

TEST(SidListTest, OverlapOtherOrder)
{
  const sid_list x("1.2.1.7-1.2.1.14,1.2.1.1-1.2.1.9");
  ASSERT_TRUE(x.valid());
  ASSERT_TRUE(x.member(sid("1.2.1.1")));
  ASSERT_TRUE(x.member(sid("1.2.1.7")));
  ASSERT_TRUE(x.member(sid("1.2.1.9")));
  ASSERT_TRUE(x.member(sid("1.2.1.10")));
  ASSERT_TRUE(x.member(sid("1.2.1.14")));
  ASSERT_FALSE(x.member(sid("1.2.1.15")));
}

TEST(SidListTest, Island)
{
  const sid_list x("1.2.1.7-1.2.1.14,1.2.1.9");
  ASSERT_TRUE(x.valid());
  ASSERT_TRUE(x.member(sid("1.2.1.7")));
  ASSERT_TRUE(x.member(sid("1.2.1.9")));
  ASSERT_TRUE(x.member(sid("1.2.1.14")));
  ASSERT_FALSE(x.member(sid("1.2.1.15")));
}

TEST(SidListTest, Outlier)
{
  const sid_list x("1.2.1.7-1.2.1.14,1.2.1.19");
  ASSERT_TRUE(x.valid());
  ASSERT_FALSE(x.empty());
  ASSERT_TRUE(x.member(sid("1.2.1.7")));
  ASSERT_TRUE(x.member(sid("1.2.1.9")));
  ASSERT_TRUE(x.member(sid("1.2.1.14")));
  ASSERT_FALSE(x.member(sid("1.2.1.15")));
  ASSERT_TRUE(x.member(sid("1.2.1.19")));
}

TEST(SidListTest, Assignment)
{
  const sid_list y("1.2.1.7-1.2.1.14,1.2.1.19");
  const sid_list x(y);
  ASSERT_TRUE(x.valid());
  ASSERT_TRUE(x.member(sid("1.2.1.7")));
  ASSERT_TRUE(x.member(sid("1.2.1.9")));
  ASSERT_TRUE(x.member(sid("1.2.1.14")));
  ASSERT_FALSE(x.member(sid("1.2.1.15")));
  ASSERT_TRUE(x.member(sid("1.2.1.19")));
}

TEST(SidListTest, Merge)
{
  sid_list x("1.2.1.7-1.2.1.14");
  const sid_list y("1.2.1.10-1.2.1.20");
  const sid_list z("1.8-1.20");

  x.merge(y);
  x.merge(z);

  // Verify that the pre-merge conditions still hold.
  ASSERT_TRUE(x.member(sid("1.2.1.7")));
  ASSERT_TRUE(y.member(sid("1.2.1.10")));
  ASSERT_TRUE(z.member(sid("1.10")));

  // Now verify that the y and z were merged into x.
  ASSERT_TRUE(x.member(sid("1.2.1.7")));
  ASSERT_TRUE(x.member(sid("1.2.1.10")));
  ASSERT_TRUE(x.member(sid("1.10")));

  // Verify that we don't just answer all questions with yes.
  ASSERT_FALSE(x.member(sid("1.2.1.51")));
}

TEST(SidListTest, Remove)
{
  sid_list x("1.2.1.7-1.2.1.14");
  ASSERT_TRUE(x.member(sid("1.2.1.10")));
  x.remove("1.2.1.10");
  ASSERT_FALSE(x.member(sid("1.2.1.10")));
}
