/*
 * test-relvbr.cc: Part of GNU CSSC.
 *
 *
 * Copyright (C) 2010, 2011, 2014, 2019 Free Software Foundation, Inc.
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
 * Unit tests for relvbr.h.
 *
 */
#include "release.h"
#include "sid.h"
#include <gtest/gtest.h>


TEST(TestRelease, Constructor)
{
  const release r;
  ASSERT_FALSE(r.valid());

  const release r2(2);
  ASSERT_TRUE(r2.valid());

  ASSERT_EQ(static_cast<short>(r2), 2);
  ASSERT_EQ(static_cast<unsigned long>(r2), 2uL);

  const release r3("3");
  ASSERT_TRUE(r3.valid());
  ASSERT_EQ(static_cast<short>(r3), 3);

  const sid s("4.5.6.7");
  const release r4(s);
  ASSERT_EQ(static_cast<short>(r4), 4);
}

TEST(TestRelease, Casts)
{
  const release r2(2);
  ASSERT_EQ(static_cast<short>(r2), 2);
  ASSERT_EQ(static_cast<unsigned long>(r2), 2uL);
}

TEST(TestRelease, Comparison)
{
  ASSERT_TRUE(release(1) < release(2));
  ASSERT_TRUE(release(1) <= release(2));
  ASSERT_TRUE(release(1) <= release(1));

  ASSERT_FALSE(release(2) < release(1));
  ASSERT_FALSE(release(2) <= release(1));
  ASSERT_TRUE(release(1) <= release(1));

  ASSERT_TRUE(release(2) > release(1));
  ASSERT_TRUE(release(2)>= release(1));
  ASSERT_TRUE(release(2) >= release(2));

  ASSERT_FALSE(release(1) > release(2));
  ASSERT_TRUE(release(1) <= release(2));
  ASSERT_TRUE(release(2) <= release(2));
}

TEST(TestRelease, Equality)
{
  ASSERT_TRUE(release(1) == release(1));
  ASSERT_FALSE(release(1) == release(2));
}

TEST(TestRelease, Inequality)
{
  ASSERT_FALSE(release(1) != release(1));
  ASSERT_TRUE(release(1) != release(2));
}
