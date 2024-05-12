/*
 * failure_macros.h: Part of GNU CSSC.
 *
 *  Copyright (C) 2019, 2024 Free Software Foundation, Inc.
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
 */
#ifndef CSSC__FAILURE_MACROS_H
#define CSSC__FAILURE_MACROS_H

#include "failure.h"
#include "ioerr.h"

#define TRY_PRINTF(printf_expression) \
  do { int result = (printf_expression); \
  if (fprintf_failed(result)) { \
    return cssc::make_failure_from_errno(errno); \
  }						 \
  } while (0)

#define TRY_PUTS(puts_expression) \
  do { int result = (puts_expression); \
  if (fputs_failed(result)) { \
    return cssc::make_failure_from_errno(errno); \
  }						 \
  } while (0)

#define TRY_PUTC(putc_expression) \
  do { int result = (putc_expression); \
  if (fputc_failed(result)) { \
    return cssc::make_failure_from_errno(errno); \
  }						 \
  } while (0)

#define TRY_OPERATION(expression)		\
  do { Failure done = (expression);		\
    if (!done.ok()) {				\
    return done;				\
  }						\
  } while (0)


#endif /* CSSC__FAILURE_MACROS_H*/

/* Local variables: */
/* mode: c++ */
/* End: */
