/*
 * showconfig.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 2001 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 *
 * Code for showing the configuration of CSSC.
 * 
 * This is called from version() in version.h.  See the README and
 * INSTALL files for details of the specific configuration options
 * which are possible.
 *
 */
#include "cssc.h"

void show_config_info(void)
{
  static const char * const enabled  = "enabled";
  static const char * const disabled = "disabled";
  bool binary_ok = binary_file_creation_allowed();
  long int line_max = max_sfile_line_len();
  
  fprintf(stderr,"\n");
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

  fprintf(stderr, 
	  "Maximum body line length (compiled-in default): %ld\n",
	  (long int) CONFIG_MAX_BODY_LINE_LENGTH);
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
}
