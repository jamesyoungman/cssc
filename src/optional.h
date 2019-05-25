/*
 * optional.h: Part of GNU CSSC.
 *
 *  Copyright (C) 2019 Free Software Foundation, Inc.
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
#ifndef CSSC_OPTIONAL_H
#define CSSC_OPTIONAL_H

#include "cssc-assert.h"

namespace cssc
{
  // optional implements a limited subset of C++17's std::optional.
  template <typename T> class optional
    {
    public:
      optional(const T& value)
	: hasvalue_(true), value_(value) {}

      optional()
	: hasvalue_(false), value_() {}

      void reset()
      {
	hasvalue_ = false;
	value_ = T();
      }

      constexpr bool has_value() const
      {
	return hasvalue_;
      }

      constexpr const T& value() const
      {
	ASSERT(hasvalue_);
	return value_;
      }

      optional& operator=(const T& value) {
	// This doesn't match std::optional very well.  I don't think
	// that will matter for us, our use of cssc::optional is very
	// simple.
	value_ = value;
	hasvalue_ = true;
	return *this;
      }


    private:
      bool hasvalue_;
      T value_;
    };
}  // namespace cssc

#endif /* CSSC_OPTIONAL_H */

/* Local variables: */
/* mode: c++ */
/* End: */
