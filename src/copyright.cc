/*
 * copyright.cc: Part of GNU CSSC.
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
#include <config.h>
#include "version.h"

#include <cstdio>
#include "copyright_data.inc"

void show_copyright(void)
{
  fprintf(stderr, "%s\n\n", copyright_explanation);
}
