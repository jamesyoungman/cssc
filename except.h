/*
 * except.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 *
 */

#ifdef HAVE_EXCEPTIONS

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

#endif
