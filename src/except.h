/*
 * except.h: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,2007 Free Software Foundation, Inc.
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
 */
#ifndef CSSC_INC_EXCEPT_H
#define CSSC_INC_EXCEPT_H 1

#include <cstdlib>

// Exception classes
struct CsscException
{
  // abstract base.
};


struct CsscExitvalException : public CsscException
{
  int exitval;
  CsscExitvalException(int n) : exitval(abs(n)) { }
};

struct CsscQuitException : public CsscExitvalException
{
  CsscQuitException(int n) : CsscExitvalException(n) { }
};

struct CsscReallyFatalException : public CsscExitvalException
{
  CsscReallyFatalException(int n) : CsscExitvalException(n) { }
};

struct CsscContstructorFailedException : public CsscExitvalException
{
  CsscContstructorFailedException(int n) : CsscExitvalException(n) { }
};


struct CsscSfileCorruptException : public CsscExitvalException
{
  CsscSfileCorruptException() : CsscExitvalException(1) { }
};

struct CsscSfileMissingException : public CsscExitvalException
{
  CsscSfileMissingException() : CsscExitvalException(1) { }
};

struct CsscPfileCorruptException : public CsscExitvalException
{
  CsscPfileCorruptException() : CsscExitvalException(1) { }
};

struct CsscNoKeywordsException : public CsscExitvalException
{
  CsscNoKeywordsException() : CsscExitvalException(1) { }
};

struct CsscUnrecognisedFeatureException : public CsscExitvalException
{
  CsscUnrecognisedFeatureException() : CsscExitvalException(1) { }
};

#endif
