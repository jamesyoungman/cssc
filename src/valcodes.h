/*
 * $RCSfile: valcodes.h,v $: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019, 2024
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
 */

#ifndef CSSC__VALCODES_H
#define CSSC__VALCODES_H



enum ValCodes
{
  Val_MissingFile             = 1<<7,
  Val_InvalidOption           = 1<<6,
  Val_CorruptFile             = 1<<5,
  Val_CannotOpenOrWrongFormat = 1<<4,
  Val_InvalidSID              = 1<<3,
  Val_NoSuchSID               = 1<<2,
  Val_MismatchedY             = 1<<1,
  Val_MismatchedM             = 1<<0
};

#endif /* CSSC__VALCODES_H */
