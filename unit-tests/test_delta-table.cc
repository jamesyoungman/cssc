/*
 * test_delta-table.cc: Part of GNU CSSC.
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
 * Unit tests for delta-table.h.
 *
 */
#include "delta-table.h"
#include "delta.h"
#include <gtest/gtest.h>


TEST(DeltaTable, Constructor)
{
  cssc_delta_table t;
  ASSERT_EQ(0, t.length());
  delta del;
  t.add(del);
  
}

// add
// length
TEST(DeltaTable, Add)
{
  cssc_delta_table t;
  ASSERT_EQ(0, t.length());
  delta del;
  t.add(del);
  ASSERT_EQ(1, t.length());
}

// prepend
// select
TEST(DeltaTable, Prepend)
{
  cssc_delta_table t;
  ASSERT_EQ(0, t.length());
  delta a, b;
  a.set_seq(seq_no(1));
  b.set_seq(seq_no(2));
  t.add(a);
  t.prepend(b);
  ASSERT_EQ(2, t.length());
  EXPECT_EQ(2, t.select(0).seq());
  EXPECT_EQ(1, t.select(1).seq());
}

// select const
TEST(DeltaTable, SelectConst)
{
  cssc_delta_table t;
  delta a;
  a.set_seq(seq_no(3));
  t.add(a);
  const cssc_delta_table& tc(t);
  EXPECT_EQ(3, tc.select(0).seq());
}

TEST(DeltaTableDeathTest, SeqNoRangeChecks)
{
  cssc_delta_table t;
  const mylist<mystring> no_comments, no_mrs;
  const delta a('D', sid("1.1"), sccs_date("990519014208"), "aldo", 
		seq_no(1), seq_no(0), no_comments, no_mrs);
  t.add(a);
  ASSERT_TRUE(t.delta_at_seq_exists(seq_no(1)));
  EXPECT_EXIT(t.delta_at_seq_exists(seq_no(0)),
	      ::testing::KilledBySignal(SIGABRT), "seq");
  EXPECT_EXIT(t.delta_at_seq_exists(seq_no(2)),
	      ::testing::KilledBySignal(SIGABRT), "seq");
}


// delta_at_seq_exists
TEST(DeltaTable, DeltaAtSeqExists)
{
  cssc_delta_table t;
  const mylist<mystring> no_comments, no_mrs;
  
  const delta a('D', sid("1.1"), sccs_date("990519014208"), "aldo", 
		seq_no(1), seq_no(0), no_comments, no_mrs);
  const delta br('R', sid("1.2"), sccs_date("990619014208"), "waldo", 
		 seq_no(2), seq_no(1), no_comments, no_mrs);
  const delta b('D', sid("1.1.1.1"), sccs_date("990620014208"), "wiggy", 
		seq_no(2), seq_no(1), no_comments, no_mrs);
  t.add(a);
  t.add(br);
  t.add(b);

  ASSERT_TRUE(t.delta_at_seq_exists(seq_no(1)));
  ASSERT_TRUE(t.delta_at_seq_exists(seq_no(2)));
}

// delta_at_seq_exists
// delta_at_seq
TEST(DeltaTable, DeltaAtSeqWithGap)
{
  cssc_delta_table t;
  const mylist<mystring> no_comments, no_mrs;
  
  const delta a('D', sid("1.1"), sccs_date("990519014208"), "aldo", 
		seq_no(1), seq_no(0), no_comments, no_mrs);
  const delta b('D', sid("1.2"), sccs_date("990619014208"), "waldo", 
		 seq_no(2), seq_no(1), no_comments, no_mrs);
  const delta c('D', sid("1.3"), sccs_date("990620014208"), "wiggy", 
		seq_no(4), seq_no(2), no_comments, no_mrs);
  t.add(a);
  t.add(b);
  t.add(c);

  ASSERT_TRUE(t.delta_at_seq_exists(seq_no(1)));
  ASSERT_TRUE(t.delta_at_seq_exists(seq_no(2)));
  ASSERT_FALSE(t.delta_at_seq_exists(seq_no(3)));
  ASSERT_TRUE(t.delta_at_seq_exists(seq_no(4)));

  ASSERT_TRUE(t.delta_at_seq(seq_no(1)).id() == a.id());
  ASSERT_TRUE(t.delta_at_seq(seq_no(2)).id() == b.id());
}

// delta_at_seq_exists
// delta_at_seq
TEST(DeltaTable, RemovedDelta)
{
  cssc_delta_table t;
  const mylist<mystring> no_comments, no_mrs;
  
  const delta a('D', sid("1.1"), sccs_date("990519014208"), "aldo", 
		seq_no(1), seq_no(0), no_comments, no_mrs);
  const delta r('R', sid("1.2"), sccs_date("990619014208"), "waldo", 
		seq_no(2), seq_no(1), no_comments, no_mrs);
  t.add(a);
  t.add(r);
  
  ASSERT_TRUE(t.delta_at_seq_exists(seq_no(1)));
  ASSERT_TRUE(t.delta_at_seq_exists(seq_no(2)));

  ASSERT_TRUE(t.delta_at_seq(seq_no(2)).id() == r.id());
}

// find_any
// find const
TEST(DeltaTable, FindAny)
{
  cssc_delta_table t;
  const delta* p;
  const mylist<mystring> no_comments, no_mrs;
  
  const delta a('D', sid("1.1"), sccs_date("990519014208"), "aldo", 
		seq_no(1), seq_no(0), no_comments, no_mrs);
  const delta r('R', sid("1.2"), sccs_date("990619014208"), "waldo", 
		seq_no(2), seq_no(1), no_comments, no_mrs);
  t.add(a);
  t.add(r);
  const cssc_delta_table& ct(t);

  p = ct.find(sid("1.1"));
  ASSERT_TRUE(p->id() == sid("1.1"));
  ASSERT_FALSE(p->removed());

  p = ct.find(sid("1.2"));
  ASSERT_TRUE(p == NULL);
  p = ct.find_any(sid("1.2"));
  ASSERT_TRUE(p->removed());
  ASSERT_TRUE(p->id() == sid("1.2"));
}


// highest_seqno
// next_seqno
// highest_release
TEST(DeltaTable, HighestSeqno)
{
  cssc_delta_table t;
  const delta* p;
  const mylist<mystring> no_comments, no_mrs;
  
  const delta a('D', sid("1.1"), sccs_date("990519014208"), "aldo", 
		seq_no(1), seq_no(0), no_comments, no_mrs);
  const delta r('R', sid("1.2"), sccs_date("990619014208"), "waldo", 
		seq_no(2), seq_no(1), no_comments, no_mrs);
  t.add(a);
  EXPECT_EQ(1, t.highest_seqno());
  EXPECT_EQ(2, t.next_seqno());
  EXPECT_EQ(1, t.highest_release());

  t.add(r);
  EXPECT_EQ(2, t.highest_seqno());
  EXPECT_EQ(3, t.next_seqno());
  EXPECT_EQ(1, t.highest_release());

  const delta b('D', sid("2.1"), sccs_date("990819014208"), "dumbo", 
		seq_no(8), seq_no(1), no_comments, no_mrs);
  t.add(b);
  EXPECT_EQ(8, t.highest_seqno());
  EXPECT_EQ(2, t.highest_release());

  // Find the next seqno and make sure it does not already exist.
  const seq_no next(t.next_seqno());
  ASSERT_GT(next, b.seq());
  ASSERT_GT(next, t.highest_seqno());
}
