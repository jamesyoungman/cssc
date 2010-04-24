/*
 * test_delta.cc: Part of GNU CSSC.
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
#include "delta.h"
#include "mylist.h"
#include "sccsdate.h"
#include "sid.h"
#include <gtest/gtest.h>


TEST(DeltaTest, NullConstructor) 
{
  delta del;
  EXPECT_FALSE(del.has_includes());
  EXPECT_FALSE(del.has_excludes());
  EXPECT_FALSE(del.has_ignores());
  EXPECT_FALSE(del.removed());
}

TEST(DeltaTest, Constructor)
{
  const sid s("1.9");
  const sccs_date then("990519014208");
  const mystring user("waldo");
  const seq_no seq(2), pred(1);
  mylist<seq_no> incl, excl;
  mylist<mystring> mrlist;
  mylist<mystring> comments;

  mrlist.add(mystring("432"));
  mrlist.add(mystring("438"));

  comments.add(mystring("I'm sure I left it around here somewhere..."));
  comments.add(mystring("...ah, here it is."));

  incl.add(seq_no(1));
  excl.add(seq_no(6));
  
  delta d('R', s, then, user, seq, pred, incl, excl, mrlist, comments);
  EXPECT_EQ(d.get_type(), 'R');
  EXPECT_EQ(d.id(), s);
  EXPECT_EQ(d.date().as_string(), then.as_string());
  EXPECT_EQ(d.user(), user);
  EXPECT_EQ(d.seq(), seq);
  EXPECT_EQ(d.prev_seq(), pred);
  EXPECT_TRUE(d.mrs() == mrlist);
  EXPECT_TRUE(d.comments() == comments);

  EXPECT_EQ(1, d.get_included_seqnos().length());
  EXPECT_EQ(seq_no(1), d.get_included_seqnos()[0]);
  EXPECT_TRUE(d.has_includes());

  EXPECT_EQ(1, d.get_excluded_seqnos().length());
  EXPECT_EQ(seq_no(6), d.get_excluded_seqnos()[0]);
  EXPECT_TRUE(d.has_excludes());

  const delta e('D', s, then, user, seq, pred, mrlist, comments);
  EXPECT_EQ(e.get_type(), 'D');
  EXPECT_EQ(e.id(), s);
  EXPECT_EQ(e.date().as_string(), then.as_string());
  EXPECT_EQ(e.user(), user);
  EXPECT_EQ(e.seq(), seq);
  EXPECT_EQ(e.prev_seq(), pred);
  EXPECT_TRUE(e.mrs() == mrlist);
  EXPECT_TRUE(e.comments() == comments);

  EXPECT_EQ(0, e.get_included_seqnos().length());
  EXPECT_FALSE(e.has_includes());
  EXPECT_EQ(0, e.get_excluded_seqnos().length());
  EXPECT_FALSE(e.has_excludes());
}

TEST(DeltaTest, Assignment)
{
  mylist<mystring> mrlist;
  mylist<mystring> comments;

  mrlist.add(mystring("123"));
  comments.add(mystring("yada"));
  
  const delta e('D', sid("1.9"),
		sccs_date("990519014208"),
		mystring("fred"), seq_no(6), seq_no(3),
		mrlist, comments);
  ASSERT_EQ('D', e.get_type());
  ASSERT_EQ(1, e.comments().length());
  ASSERT_EQ(mystring("yada"), e.comments()[0]);

  delta d;
  ASSERT_EQ(0, d.comments().length());

  d = e;
  ASSERT_EQ('D', d.get_type());
  ASSERT_EQ(1, d.comments().length());
  ASSERT_EQ(mystring("yada"), d.comments()[0]);
}

TEST(DeltaTest, Removed)
{
  const mylist<mystring> mrlist;
  const mylist<mystring> comments;
  const delta e('R', sid("1.9"), sccs_date("990519014208"),
		mystring("fred"), seq_no(6), seq_no(3), mrlist, comments);
  EXPECT_TRUE(e.removed());
}

TEST(DeltaDeathTest, InvalidType)
{
  const mylist<mystring> mrlist;
  const mylist<mystring> comments;
  EXPECT_EXIT(delta e('X', sid("1.9"), sccs_date("990519014208"),
		      mystring("fred"), seq_no(6), seq_no(3), mrlist, comments),
	      ::testing::KilledBySignal(SIGABRT), 
	      "valid");
}

TEST(DeltaTest, Mutators)
{
  const sid s("1.2");
  delta d;
  const sccs_date then("140519014208");
  d.set_id(s);
  EXPECT_EQ("1.2", d.id().as_string());

  d.set_date(then);
  EXPECT_EQ("14/05/19 01:42:08", d.date().as_string());

  d.set_user("fred");
  EXPECT_EQ(d.user(), "fred");

  d.set_seq(seq_no(4));
  EXPECT_EQ(4, d.seq());
  
  d.set_prev_seq(seq_no(2));
  EXPECT_EQ(2, d.prev_seq());

  d.add_include(seq_no(1));
  ASSERT_TRUE(d.has_includes());
  ASSERT_FALSE(d.has_excludes());
  ASSERT_FALSE(d.has_ignores());
  d.add_include(seq_no(2));
  EXPECT_EQ(2, d.get_included_seqnos().length());
  EXPECT_EQ(seq_no(2), d.get_included_seqnos()[1]);

  d.add_exclude(seq_no(6));
  ASSERT_TRUE(d.has_includes());
  ASSERT_TRUE(d.has_excludes());
  ASSERT_FALSE(d.has_ignores());
  d.add_exclude(seq_no(7));
  EXPECT_EQ(2, d.get_excluded_seqnos().length());
  EXPECT_EQ(seq_no(7), d.get_excluded_seqnos()[1]);

  d.add_ignore(seq_no(3));
  ASSERT_TRUE(d.has_includes());
  ASSERT_TRUE(d.has_excludes());
  ASSERT_TRUE(d.has_ignores());
  d.add_ignore(seq_no(5));
  EXPECT_EQ(2, d.get_ignored_seqnos().length());
  EXPECT_EQ(seq_no(5), d.get_ignored_seqnos()[1]);


  ASSERT_EQ(0, d.mrs().length());
  d.add_mr("583");
  d.add_mr("2");
  EXPECT_EQ("583", d.mrs()[0]);
  EXPECT_EQ("2", d.mrs()[1]);
  EXPECT_EQ(2, d.mrs().length());
  mylist<mystring> mrs;
  mrs.add("4");
  d.set_mrs(mrs);
  EXPECT_EQ(1, d.mrs().length());
  EXPECT_EQ("4", d.mrs()[0]);

  ASSERT_EQ(0, d.comments().length());
  d.add_comment("Hello?");
  d.add_comment("Is there anybody there?\nI can hear you.");
  EXPECT_EQ("Hello?", d.comments()[0]);
  EXPECT_EQ("Is there anybody there?\nI can hear you.", d.comments()[1]);
  EXPECT_EQ(2, d.comments().length());
  
  const mystring comment("Please remember to put the cat out.");
  mylist<mystring> comments;
  comments.add(comment);
  d.set_comments(comments);
  EXPECT_EQ(1, d.comments().length());
  EXPECT_EQ(comment, d.comments()[0]);

  // Make sure we didn't confuse mrs and comments.
  EXPECT_EQ("4", d.mrs()[0]);

  ASSERT_EQ(0, d.inserted());
  ASSERT_EQ(0, d.deleted());
  ASSERT_EQ(0, d.unchanged());

  d.set_inserted(491);
  EXPECT_EQ(491, d.inserted());

  d.set_idu(5, 6, 7);
  EXPECT_EQ(5, d.inserted());
  EXPECT_EQ(6, d.deleted());
  EXPECT_EQ(7, d.unchanged());

  d.increment_inserted();
  EXPECT_EQ(6, d.inserted());
  
  d.increment_deleted();
  EXPECT_EQ(7, d.deleted());
  
  d.increment_unchanged();
  EXPECT_EQ(8, d.unchanged());
}
