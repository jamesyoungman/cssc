/*
 * mr-list.h: Part of GNU CSSC.
 *
 *  Copyright (C) 2026 Free Software Foundation, Inc.
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
 * Functions for non-destructively splitting a string into a list of
 * MRs. These declarations were previously in sccsfile.h.
 */

#ifndef CSSC__MR_LIST_H__
#define CSSC__MR_LIST_H__

#include <string>
#include <vector>

std::vector<std::string> split_mrs(const std::string& mrs);

#endif
