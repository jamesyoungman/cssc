// No copyright header here because we want "make update-copyright"
// update the copyright in the string constant, not just a file header
// comment.

#include <config.h>
#include "version.h"

#include <cstdio>

static const char * copyright_explanation =
"Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, "
"2006, 2007, 2008, 2009, 2010, 2014, 2019 Free Software Foundation, Inc.\n"
"\n"
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, either version 3 of the License, or\n"
"(at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n";

void show_copyright(void)
{
  fprintf(stderr, "%s\n\n", copyright_explanation);
}
