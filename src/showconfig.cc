/*
 * showconfig.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 2001, 2007, 2008, 2009, 2010, 2011, 2014, 2019 Free
 *  Software Foundation, Inc.
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
 * Code for showing the configuration of CSSC.
 *
 * This is called from version() in version.h.  See the README and
 * INSTALL files for details of the specific configuration options
 * which are possible.
 *
 */
#include <config.h>

#include <cstdio>
#include <unistd.h>
#include <limits.h>
#include <limits>

#include "cssc.h"
#include "version.h"

static void show_system_line_max(void)
{
  long m = 0L;
#ifdef HAVE_SYSCONF
#ifdef _SC_LINE_MAX
  if (0L == m)
    m = sysconf(_SC_LINE_MAX);
  if (m < 0L)
    m = 0L;		   // there was an error, use LINE_MAX instead
#endif
#endif
#ifdef LINE_MAX
  /* LINE_MAX is a conservative value - _SC_LINE_MAX is likely
   * to be larger.
   */
  if (0L == m)
    m = LINE_MAX;
#endif

  if (m)
    {
      fprintf(stderr,
	      "\n"
	      "The system diff utility should be able to handle lines of up\n"
	      "to %ld characters, and perhaps more.  You are using %s as your\n"
	      "diff utility, but this program is not clever enough to determine\n"
	      "if that is the system utility (to which the _SC_LINE_MAX limit\n"
	      "shown above applies) or not.  If %s is GNU diff, then there\n"
	      "will be no upper limit.\n",
	      m,
	      (CONFIG_DIFF_COMMAND), (CONFIG_DIFF_COMMAND));

#ifdef HAVE_GNU_DIFF
      fprintf(stderr,
	      "\nWhen this version of CSSC was compiled, %s was GNU diff, "
	      "which has no upper limit on line lengths.\n",
	      (CONFIG_DIFF_COMMAND));
#endif

    }
}


void show_config_info(void)
{
  static const char * const enabled  = "enabled";
  static const char * const disabled = "disabled";
  bool binary_ok = binary_file_creation_allowed();
  long int line_max = max_sfile_line_len();

  fprintf(stderr,"CURRENT CONFIGURATION:\n");
  fprintf(stderr,
	  "Binary file support (compiled-in default): %s\n",
#ifdef CONFIG_DISABLE_BINARY_SUPPORT
	  disabled
#else
	  enabled
#endif
	  );
  fprintf(stderr,
	  "Binary file support (as overridden by $CSSC_BINARY_SUPPORT): %s\n",
	  binary_ok ? enabled : disabled );

  // If CONFIG_MAX_BODY_LINE_LENGTH is too large to fit into the cast to long int
  // below, prompt the programmer to use a different type.
  static_assert(CONFIG_MAX_BODY_LINE_LENGTH
		<= std::numeric_limits<long int>::max(),
		"Please change the format specifier and cast below to suit your "
		"choice of CONFIG_MAX_BODY_LINE_LENGTH.");
  fprintf(stderr,
	  "Maximum body line length (compiled-in default): %ld\n",
	  static_cast<long int>(CONFIG_MAX_BODY_LINE_LENGTH));
  fprintf(stderr,
	  "Maximum body line length (as overridden by "
	  "$CSSC_MAX_LINE_LENGTH): %ld\n",
	  line_max);
  fprintf(stderr,"\n");

  fprintf(stderr, "Commentary:\n");

  if (binary_ok)
    {
      fprintf(stderr, "%s",
	      "Binary file support is enabled; this means that CSSC\n"
	      "will create an encoded SCCS file if you pass the \"-b\"\n"
	      "option to \"admin\", or if you create an SCCS file from\n"
	      "an input file which the SCCS file format cannot represent in\n"
	      "text format.\n"
	      );
    }
  else
    {
      fprintf(stderr, "%s",
	      "Binary file support is disabled; this means that CSSC\n"
	      "will not create encoded SCCS files, but will handle them\n"
	      "both for reading and writing, if it finds already encoded\n"
	      "files.\n"
	      );
    }
  fprintf(stderr,
	  "Set the environment variable CSSC_BINARY_SUPPORT "
	  "to change this.\n\n");


  if (line_max)
    {
      fprintf(stderr,
	      "Lines in the main body of the SCCS files that CSSC produces\n"
	      "are limited to %ld characters; input lines longer than this\n"
	      "will cause a fatal error.  This means that CSSC can fail to\n"
	      "interoperate with SCCS implementations which limit the length\n"
	      "of a body line to less than %ld characters, or fail to\n"
	      "correctly modify SCCS files produced by SCCS implementations\n"
	      "which has a limit which is greater than %ld characters.\n",
	      line_max,
	      line_max,
	      line_max
	      );
    }
  else
    {
      fprintf(stderr,
	      "Lines in the main body of the SCCS files that CSSC produces\n"
	      "are not limited in length.  This means that CSSC can fail to\n"
	      "interoperate with SCCS implementations which limit the length\n"
	      "of a body line to some fixed number.\n");
    }
  fprintf(stderr,
	  "Set the environment variable CSSC_MAX_LINE_LENGTH "
	  "to change this.\n");

  show_system_line_max();
}
