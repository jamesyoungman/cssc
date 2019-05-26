/*
 * privs.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2003, 2004, 2007, 2008, 2009,
 *  2010, 2011, 2014, 2019 Free Software Foundation, Inc.
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
 */
#include "config.h"
#include "privs.h"

namespace
{

/* A flag to indicate whether or not the programme is an privileged
   (effective UID != real UID) or unprivileged (effective UID == real
   UID). */
static int unprivileged = 0;

#ifdef CONFIG_UIDS

# ifdef SAVED_IDS_OK

static uid_t old_euid;

// Set-user-id is saved.  TODO: What about setuid-root binaries?
void
give_up_privileges() {
        if (unprivileged++ == 0) {
                old_euid = geteuid();
                if (setuid(getuid()) == -1) {
                        fatal_quit(errno, "setuid(%d) failed", getuid());
                }
                ASSERT(getuid() == geteuid());
        }
}

void
restore_privileges() {
        if (--unprivileged == 0) {
                if (setuid(old_euid) == -1) {
                        fatal_quit(errno, "setuid(%d) failed", old_euid);
                }
                ASSERT(geteuid() == old_euid);
        }
        ASSERT(unprivileged >= 0);
}

# elif defined(HAVE_SETREUID)

// POSIX saved IDs not provided or not always provided; use
// setreuid() instead.

static uid_t old_ruid, old_euid;

void
give_up_privileges() {
        if (unprivileged++ == 0) {
                old_ruid = getuid();
                old_euid = geteuid();

                if (setreuid(old_euid, old_ruid) == -1) {
                        fatal_quit(errno, "setreuid(%d, %d) failed.",
                                   old_euid, old_ruid);
                }
                ASSERT(geteuid() == old_ruid);
        }
}

void
restore_privileges() {
        if (--unprivileged == 0) {
                if (setreuid(old_ruid, old_euid) == -1) {
                        fatal_quit(errno, "setreuid(%d, %d) failed.",
                                   old_ruid, old_euid);
                }
                ASSERT(geteuid() == old_euid);
        }
        ASSERT(unprivileged >= 0);
}


# else /* defined(HAVE_SETREUID) */

// Processes do not have a saved set-user-id,
// and setreuid() is not available.
void
give_up_privileges() {
        ++unprivileged;
        if (geteuid() != getuid()) {
                fatal_quit(-1, "Set UID not supported.");
        }
}

void
restore_privileges() {
        --unprivileged;
        ASSERT(unprivileged >= 0);
}

# endif /* defined(HAVE_SETREUID) */


#else /* CONFIG_UIDS */

void
give_up_privileges()
{
  // Dummy implementation for systems without Unix-like UIDs.
  unprivileged++;
}

void
restore_privileges()
{
  // Dummy implementation for systems without Unix-like UIDs.
  --unprivileged;
}

#endif /* CONFIG_UIDS */

}  // unnamed namespace

TempPrivDrop::TempPrivDrop(bool real)
  : real_(real)
{
  if (real_)
    {
      give_up_privileges();
    }
}

TempPrivDrop::~TempPrivDrop()
{
  if (real_)
    {
      restore_privileges();
    }
}
