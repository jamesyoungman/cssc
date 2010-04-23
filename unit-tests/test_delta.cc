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
#include "cssc.h"
#include "mylist.h"
#include "sccsdate.h"
#include "sid.h"
#include "delta.h"
#include <gtest/gtest.h>


TEST(DeltaTest, NullConstructor) 
{
  delta del;
  EXPECT_FALSE(del.have_includes);
  EXPECT_FALSE(del.have_excludes);
  EXPECT_FALSE(del.have_ignores);
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
  EXPECT_EQ(d.id, s);
  EXPECT_EQ(d.date.as_string(), then.as_string());
  EXPECT_EQ(d.user, user);
  EXPECT_EQ(d.seq, seq);
  EXPECT_EQ(d.prev_seq, pred);
  EXPECT_TRUE(d.mrs == mrlist);
  EXPECT_TRUE(d.comments == comments);

  EXPECT_EQ(1, d.included.length());
  EXPECT_EQ(seq_no(1), d.included[0]);
  EXPECT_TRUE(d.have_includes);

  EXPECT_EQ(1, d.excluded.length());
  EXPECT_EQ(seq_no(6), d.excluded[0]);
  EXPECT_TRUE(d.have_excludes);

  EXPECT_NE(&d.id, &s);
  EXPECT_NE(&d.date, &then);
  EXPECT_NE(&d.user, &user);
  EXPECT_NE(&d.seq, &seq);
  EXPECT_NE(&d.prev_seq, &pred);
  EXPECT_NE(&d.included, &incl);
  EXPECT_NE(&d.excluded, &excl);
  EXPECT_NE(&d.mrs, &mrlist);
  EXPECT_NE(&d.comments, &comments);


  const delta e('D', s, then, user, seq, pred, mrlist, comments);
  EXPECT_EQ(e.get_type(), 'D');
  EXPECT_EQ(e.id, s);
  EXPECT_EQ(e.date.as_string(), then.as_string());
  EXPECT_EQ(e.user, user);
  EXPECT_EQ(e.seq, seq);
  EXPECT_EQ(e.prev_seq, pred);
  EXPECT_TRUE(e.mrs == mrlist);
  EXPECT_TRUE(e.comments == comments);

  EXPECT_EQ(0, e.included.length());
  EXPECT_EQ(0, e.excluded.length());
  EXPECT_FALSE(e.have_includes);
  EXPECT_FALSE(e.have_excludes);
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
  ASSERT_EQ(1, e.comments.length());
  ASSERT_EQ(mystring("yada"), e.comments[0]);

  delta d;
  ASSERT_EQ(0, d.comments.length());

  d = e;
  ASSERT_EQ('D', d.get_type());
  ASSERT_EQ(1, d.comments.length());
  ASSERT_EQ(mystring("yada"), d.comments[0]);
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

