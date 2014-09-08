/*
 * environment.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 2001,2007 Free Software Foundation, Inc.
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
 *
 * Code for handling environment variables which affect CSSC.  See the
 * sections "Environment" and "Interoperability" in the CSSC manual.
 *
 */
#include "config.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits.h>

#include "cssc.h"

bool binary_file_creation_allowed (void)
{
  static const char * const bin_var = "CSSC_BINARY_SUPPORT";
  static const char * const enabled = "enabled";
  static const char * const disabled = "disabled";

  const char *p = getenv(bin_var);

  if (p)
    {
      if (0 == strcmp(p, enabled))
	{
	  return true;
	}
      else if (0 == strcmp(p, disabled))
	{
	  return false;
	}
      else
	{
	  /* This function should be called at program start-up,
	   * so there should be few cleanup implications of a direct
	   * call to exit() here.
	   */
	  fprintf(stderr,
		  "Error: The %s environment variable, if set, must be set "
		  "to either '%s' or '%s'.\n",
		  bin_var,
		  enabled,
		  disabled);
	  exit(1);
	}
    }
  else
    {
#ifdef CONFIG_DISABLE_BINARY_SUPPORT
      return false;
#else
      return true;
#endif
    }
}


long max_sfile_line_len(void)
{
  static const char * const max_var = "CSSC_MAX_LINE_LENGTH";
  const char *p;
  long len;


  p = getenv(max_var);
  if (p)
    {
      char *endptr;
      errno = 0;
      len = strtol(p, &endptr, 10);
      if ( (endptr == p)
	   || ( (LONG_MIN == len) || (LONG_MAX == len) ) && (0 != errno)
	   || len < 0)
	{
	  fprintf(stderr,
		  "Error: Environment variable '%s' is set to '%s', but "
		  "should be either a positive decimal integer or unset.\n",
		  max_var,
		  p);
	  exit(1);
	}
      else
	{
	  return len;
	}
    }
  else
    {
      return CONFIG_MAX_BODY_LINE_LENGTH;
    }
}


void check_env_vars(void)
{
  (void) binary_file_creation_allowed();
  (void) max_sfile_line_len();
}
