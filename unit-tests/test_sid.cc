/*
 * test-sid.cc: Part of GNU CSSC.
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
 * Unit tests for sid.h.
 *
 */
#include "cssc.h"
#include "release.h"
#include "sid.h"
#include <gtest/gtest.h>


TEST(SidTest, NullConstructor) 
{
  const sid none;
  EXPECT_TRUE(none.is_null());
  EXPECT_FALSE(none.valid());
}

TEST(SidTest, StringConstructor) 
{
  const sid s("1.2.3.4");
  EXPECT_FALSE(s.is_null());
  EXPECT_TRUE(s.valid());
  EXPECT_EQ(4, s.components());
  EXPECT_FALSE(s.on_trunk());
}

TEST(SidTest, ConstructTrunkSid) 
{
  const sid s("1.2");
  EXPECT_FALSE(s.is_null());
  EXPECT_EQ(2, s.components());
  EXPECT_TRUE(s.valid());
  EXPECT_FALSE(s.partial_sid());
  EXPECT_TRUE(s.on_trunk());
}

TEST(SidTest, ConstructFromRelease) 
{
  const release rel(4);
  const sid s(rel);
  EXPECT_EQ(1, s.components());
  EXPECT_EQ("4.0", s.as_string());
}

TEST(SidTest, PartialSid) 
{
  const sid one("1");
  EXPECT_FALSE(one.is_null());
  EXPECT_EQ(1, one.components());
  EXPECT_TRUE(one.valid());
  EXPECT_TRUE(one.partial_sid());
  EXPECT_FALSE(one.on_trunk());

  const sid three("1.2.3");
  EXPECT_FALSE(three.is_null());
  EXPECT_EQ(3, three.components());
  EXPECT_TRUE(three.valid());
  EXPECT_TRUE(three.partial_sid());
  EXPECT_FALSE(three.on_trunk());
}

TEST(SidTest, StringConversion) 
{
  const sid one("1");
  const sid two("1.2");
  const sid three("1.2.3");
  const sid four("1.2.3.4");

  EXPECT_EQ("1.0", one.as_string());
  EXPECT_EQ("1.2", two.as_string());
  EXPECT_EQ("1.2.3.0", three.as_string());
  EXPECT_EQ("1.2.3.4", four.as_string());
}

TEST(SidTest, Assignment)
{
  const sid a("1.2.3.4");
  EXPECT_EQ("1.2.3.4", a.as_string());
  sid b("1.2.3.5");
  EXPECT_EQ("1.2.3.5", b.as_string());
  b = a;
  EXPECT_EQ("1.2.3.4", b.as_string());
  // Now make sure that all fields can be changed in an assignment.
  b = sid("4.3.2.1");
  EXPECT_EQ("4.3.2.1", b.as_string());
}

TEST(SidTest, Greater)
{
  const sid a("1.2.3.4");
  const sid b("1.2.3.5");
  EXPECT_GT(b, a);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(a > a);
  EXPECT_FALSE(b > b);
}

TEST(SidTest, GreaterEqual)
{
  const sid a("1.2.3.4");
  const sid b("1.2.3.5");
  const sid c("1.2.3.5");
  EXPECT_GE(b, a);
  EXPECT_GE(b, c);
  EXPECT_GE(c, b);
  EXPECT_FALSE(a >= b);
}

TEST(SidTest, Less)
{
  const sid a("1.2.3.4");
  const sid b("1.2.3.5");
  EXPECT_FALSE(b < a);
  EXPECT_LT(a, b);
  EXPECT_FALSE(a < a);
  EXPECT_FALSE(b < b);
}

TEST(SidTest, LessEqual)
{
  const sid a("1.2.3.4");
  const sid b("1.2.3.5");
  const sid c("1.2.3.5");
  EXPECT_FALSE(b <= a);
  EXPECT_LE(a, b);
  EXPECT_LE(b, c);
  EXPECT_LE(c, b);
}

TEST(SidTest, Equality)
{
  const sid a("1.2.3.4");
  const sid b("1.2.3.4");
  EXPECT_EQ(a, b);
  EXPECT_EQ(b, a);
  
  const sid c("1.2.3.5");
  EXPECT_NE(a, c);
  EXPECT_FALSE(a == c);
}


TEST(SidTest, Inequality)
{
  const sid a("1.2.3.4");
  const sid b("1.2.3.5");
  EXPECT_NE(a, b);
  EXPECT_NE(b, a);
  
  const sid c("1.2.3.4");
  EXPECT_EQ(a, c);
  EXPECT_FALSE(a != c);
}

TEST(SidTest, Successor)
{
  const sid a("1.2.3.4");
  ASSERT_EQ(a.successor(), sid("1.2.3.5"));

  const sid b("5.6");
  ASSERT_EQ(b.successor(), sid("5.7"));
}

TEST(SidTest, NextBranch)
{
  sid a("1.2.3.4");
  a.next_branch();
  ASSERT_EQ(a, sid("1.2.4.1"));

  sid b("5.6");
  b.next_branch();
  ASSERT_EQ(b, sid("5.6.1.1"));
}

TEST(SidTest, NextLevel)
{
  sid a("1.2.3.4");
  a.next_level();
  ASSERT_EQ(a.as_string(), "1.3");

  sid b("5.6");
  b.next_level();
  ASSERT_EQ(b.as_string(), "5.7");
}

TEST(SidTest, Increment)
{
  sid a("1.2.3.4");
  ++a;
  ASSERT_EQ(a.as_string(), "1.2.3.5");

  sid b("5.6");
  ++b;
  ASSERT_EQ(b.as_string(), "5.7");

  sid c("8");
  ++c;
  ASSERT_EQ(c.as_string(), "9.0");
}

TEST(SidTest, Decrement)
{
  sid a("1.2.3.4");
  --a;
  ASSERT_EQ(a.as_string(), "1.2.3.3");

  sid b("5.6");
  --b;
  ASSERT_EQ(b.as_string(), "5.5");

  sid c("8");
  --c;
  ASSERT_EQ(c.as_string(), "7.0");
}

TEST(SidTest, TrunkSuccessor)
{
  sid a("5.4");
  sid b("5.6");
  // b is a trunk successor of a.
  ASSERT_TRUE(a.is_trunk_successor(b));

  // c is not a trunk successor of a since c is not on the trunk.
  sid c("5.7.1.1");
  ASSERT_FALSE(a.is_trunk_successor(c));
}

TEST(SidTest, BranchGreaterThan)
{
  sid a("5.4.3.2");
  sid b("5.4.4.1");
  ASSERT_TRUE(b.branch_greater_than(a));
  ASSERT_FALSE(a.branch_greater_than(b));
  ASSERT_FALSE(a.branch_greater_than(a));
  
  sid c("5.4");
  ASSERT_TRUE(a.branch_greater_than(c));
  ASSERT_TRUE(b.branch_greater_than(c));
  ASSERT_FALSE(c.branch_greater_than(a));
  ASSERT_FALSE(c.branch_greater_than(b));
}

TEST(SidTest, PartialMatch) 
{
  // Non-comparable SIDs cannot be a partial match.
  sid a("5.4");
  sid b("1.2.3.4");
  ASSERT_FALSE(a.partial_match(b));
  ASSERT_FALSE(b.partial_match(a));

  // The null SID is not comparable with anything.
  const sid null = sid::null_sid();
  ASSERT_FALSE(null.partial_match(null));
  ASSERT_FALSE(null.partial_match(a));
  ASSERT_FALSE(b.partial_match(null));

  // Identical SIDs are also partial_matches.
  ASSERT_TRUE(a.partial_match(a));
  // FIXME: figure out if b should be a partial match for itself.
  //ASSERT_TRUE(b.partial_match(b));

  // A release mismatch will cause a partial_match to fail.
  ASSERT_FALSE(sid("1.2").partial_match(sid("5.6")));
}

TEST(SidTest, Matches) 
{
  sid a("1.2.3.4");
  sid b("1.2.3.5");

  // SIDs always match themselves.
  ASSERT_TRUE(a.matches(a, 4));
  ASSERT_TRUE(b.matches(b, 4));

  ASSERT_TRUE(a.matches(a, 3));
  ASSERT_TRUE(b.matches(b, 3));

  ASSERT_TRUE(a.matches(a, 2));
  ASSERT_TRUE(b.matches(b, 2));

  ASSERT_TRUE(a.matches(a, 1));
  ASSERT_TRUE(b.matches(b, 1));

  // Mismatched SIDs match to zero components.
  ASSERT_TRUE(a.matches(sid("5.6"), 0));
  // But we should detect a mismatch at the release.
  ASSERT_FALSE(a.matches(sid("5.6"), 1));

  // Check mismatch detection at the level.
  a = sid("1.2.3.4");
  b = sid("1.3.3.4");
  ASSERT_TRUE(a.matches(b, 1));
  ASSERT_FALSE(a.matches(b, 2));

  // Check mismatch detection at the branch.
  a = sid("1.2.3.4");
  b = sid("1.2.4.4");
  ASSERT_TRUE(a.matches(b, 1));
  ASSERT_TRUE(a.matches(b, 2));
  ASSERT_FALSE(a.matches(b, 3));
  ASSERT_FALSE(a.matches(b, 4));

  // Check mismatch detection at the sequence.
  a = sid("1.2.3.4");
  b = sid("1.2.3.5");
  ASSERT_TRUE(a.matches(b, 1));
  ASSERT_TRUE(a.matches(b, 2));
  ASSERT_TRUE(a.matches(b, 3));
  ASSERT_FALSE(a.matches(b, 4));
}

TEST(SidTest, ReleaseOnly) 
{
  const sid a("1.2.3.4");
  ASSERT_FALSE(a.release_only());
  
  const sid b("1.2");
  ASSERT_FALSE(b.release_only());

  const sid c("1.0");
  ASSERT_TRUE(c.release_only());

  const sid d("0.0");
  ASSERT_FALSE(d.release_only());
}


// trunk_match
TEST(SidTest, TrunkMatch) 
{
  ASSERT_TRUE(sid("1.2").trunk_match("1.2"));
  ASSERT_TRUE(sid("1.2").trunk_match("1.2.3.4"));
  ASSERT_FALSE(sid("1.2").trunk_match("1.3.3.4"));
  ASSERT_FALSE(sid("1.3").trunk_match("1.2"));

  // Different branches can still be trunk matches.
  ASSERT_TRUE(sid("1.2.7.8").trunk_match("1.2.3.4"));
}


#if 0
// sid::sid(relvbr) is declared but not implemented.
TEST(SidTest, ConstructFromRelvbr)
{
  const relvbr r("1.2.3");
  const sid s(r);
  EXPECT_EQ("1.2.3.0", s.as_string());
}
#endif


