/*
 * ioerr.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 2007, 2008, 2009, 2010, 2011, 2014, 2019
 *  Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This file centralises knowledge of the return values
 * of the various <cstdio> functions.
 *
 */

#ifndef CSSC__IOERR_H__
#define CSSC__IOERR_H__

#include "failure.h"

#define  fclose_failed(n) (EOF == (n))
#define   fputc_failed(n) (EOF == (n))
#define    putc_failed(n) (EOF == (n))
#define   fputs_failed(n) (EOF == (n))
#define  fflush_failed(n) (EOF == (n))

#define  printf_failed(n) ((n) < 0)
#define fprintf_failed(n) ((n) < 0)

inline cssc::Failure fwrite_failed(int written, int desired) {
  if (written < desired)
      return cssc::make_failure_from_errno(errno);
  return cssc::Failure::Ok();
}

inline cssc::Failure fputc_failure(int ch, FILE *f) {
  if (fputc(ch, f) == EOF)
    return cssc::make_failure_from_errno(errno);
  return cssc::Failure::Ok();
}

inline cssc::Failure fprintf_failure(int rv)
{
  if (rv < 0)
    return cssc::make_failure_from_errno(errno);
  return cssc::Failure::Ok();
}

inline cssc::Failure fclose_failure(FILE *fp)
{
  if (fclose(fp) != 0)
    return cssc::make_failure_from_errno(errno);
  return cssc::Failure::Ok();
}

inline cssc::Failure fflush_failure(FILE *fp)
{
  if (fflush_failed(fflush(fp)))
    return cssc::make_failure_from_errno(errno);
  return cssc::Failure::Ok();
}


#endif
