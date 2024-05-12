/*
 * linebuf.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 2007, 2009, 2010, 2011, 2014, 2019, 2024
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Members of the class cssc_linebuf.
 *
 */
#include "config.h"

#include <cstdio>
#include <cstring>
#include <climits>
#include <system_error>

#include "cssc.h"
#include "bodyio.h"
#include "failure.h"
#include "linebuf.h"
#include "cssc-assert.h"
#include "ioerr.h"


// Use a small chunk size for testing...
#define CONFIG_LINEBUF_CHUNK_SIZE (1024u)

cssc_linebuf::cssc_linebuf()
  : buf_(new char[CONFIG_LINEBUF_CHUNK_SIZE]),
    buflen_(CONFIG_LINEBUF_CHUNK_SIZE)
{
}


cssc::Failure
cssc_linebuf::read_line(FILE *f)
{
  ASSERT(CONFIG_LINEBUF_CHUNK_SIZE > 2u);
  buf_[buflen_ - 2u] = '\0';

  ASSERT(buflen_ < INT_MAX);
  char *s = fgets(buf_, static_cast<int>(buflen_), f);
  while (s != NULL)
    {
      char c = buf_[buflen_ - 2u];
      if (c == '\0' || c == '\n')
	return cssc::Failure::Ok();

//
// Add another chunk
//

      char *temp_buf = new char[CONFIG_LINEBUF_CHUNK_SIZE + buflen_];
      memcpy( temp_buf, buf_, buflen_);
      delete [] buf_;
      buf_ = temp_buf;

      s = buf_ + buflen_ - 1u;
      buflen_ += CONFIG_LINEBUF_CHUNK_SIZE;
      buf_[buflen_ - 2u] = '\0';

      s = fgets(s, CONFIG_LINEBUF_CHUNK_SIZE + 1u, f); // fill the new chunk
    }
  if (ferror(f))
    {
      return cssc::make_failure_from_errno(errno);
    }
  return cssc::make_failure(cssc::errorcode::UnexpectedEOF);
}


cssc::Failure cssc_linebuf::write(FILE *f) const
{
  size_t len = strlen(buf_);
  return fwrite_failed(fwrite(buf_, sizeof(char), len, f), len);
}

int
cssc_linebuf::split(int offset, char **args, int len, char c)
{
  char *start = buf_ + offset;
  char *end = strchr(start, c);
  int i;

  for (i = 0; i < len; i++)
    {
      args[i] = start;
      if (nullptr == end)
	{
	  if (start[0] != '\0')
	    i++;
	  return i;		// no more delimiters.
	}
      *end++ = '\0';
      start = end;
      end = strchr(start, c);
    }

  return i;
}

void cssc_linebuf::
set_char(unsigned offset, char value)
{
  ASSERT(offset < buflen_);
  buf_[offset] = value;
}

bool cssc_linebuf::check_id_keywords() const
{
  return ::check_id_keywords(buf_, strlen(buf_));	// TODO: make NUL-safe!
}

std::unique_ptr<cssc_linebuf> make_unique_linebuf()
{
#if __cplusplus >= 201402L
  return std::make_unique<cssc_linebuf>();
#else
  return std::unique_ptr<cssc_linebuf>(new cssc_linebuf());
#endif
}


/* Local variables: */
/* mode: c++ */
/* End: */
