/*
 * run.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2007, 2009, 2010, 2011, 2014, 2019
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
 */

#ifndef CSSC__RUN_H__
#define CSSC__RUN_H__

#include <string>
#include <vector>

/* fork(); problem.
 *
 * On AmigaOS, ixemul.library provides fork(), but only as a stub.
 * This means that we can't use it even though configure finds it.
 * We can't run it to test it because that would mean that we would
 * lose all support for cross-compiling.   Blech.  We hack configure.in
 * to also check for the __amigaos__ proprocessor macro...
 */
#ifdef HAVE_FORK
#ifdef __amigaos__
#error Unless I'm mistaken we can't use fork() on AmigaOS.
Stop now!  Compilers should obey #error!  Stop I say, stop!  Run for it Harold!
#endif
#endif

int run(const char *prg, std::vector<const char *> const &args);
int run_mr_checker(const char *prg, const char *arg1,
		   const std::vector<std::string>& mrs);

#endif /* CSSC__RUN_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
