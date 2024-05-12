/*
 * filelock.h: Part of GNU CSSC.
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Defines the class file_lock.
 */

#ifndef CSSC__FILELOCK_H__
#define CSSC__FILELOCK_H__

#include <memory>
#include <string>

#include "cleanup.h"
#include "failure.h"
#include "optional.h"

class file_lock : private cleanup {
        cssc::optional<cssc::Failure> lock_state_;
        std::string name_;

        // TODO: consider a more modern kind of cleanup object.
	void do_cleanup() override { this->~file_lock(); }

public:
        file_lock(const std::string& zname);
        cssc::Failure is_locked() const {
	  if (lock_state_.has_value())
	    {
	      return lock_state_.value();
	    }
	  return cssc::make_failure(cssc::errorcode::LockNotHeld);
	}
	~file_lock();
};

std::unique_ptr<file_lock> make_unique_file_lock(const std::string&);

#endif /* __FILELOCK_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
