/*
 * cleanup.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1999, 2001, 2007, 2008, 2009, 2010, 2011, 2014,
 *  2019 Free Software Foundation, Inc.
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
#ifndef CSSC__CLEANUP_H__
#define CSSC__CLEANUP_H__

#include <functional>

// TODO: consider switching to using a template cleanup object with a
// lambda for custom cleanup actions.

// Cleaner is a class that you should have in scope in main;
// it ensures that all required cleanups are run even if you don't
// call quit().
class Cleaner
{
public:
  Cleaner();
  ~Cleaner();
};

/*
 * The class "cleanup" is one which you can inherit from in order to
 * ensure that resources are cleaned up when a function is exited.
 * This is most notably used by the "file_lock" class.
 */
class cleanup {
        static class cleanup *head;
        static int running;
        static int all_disabled;
#if HAVE_FORK
        static int in_child_flag;
#endif

        class cleanup *next;

	// This class has pointer members so we should override the
	// copy constructor and assignment operator if we want
	// instances to be vopyable.  So that we don't have to do
	// that, just prevent copying.
	cleanup(const cleanup&) = delete;
	cleanup& operator=(const cleanup&) = delete;

protected:
        cleanup();
        virtual void do_cleanup() = 0;

        virtual ~cleanup();

public:

        static void run_cleanups();
        static int active() { return running; }
        static void disable_all() { all_disabled++; }
#ifdef HAVE_FORK
        static void set_in_child() { in_child_flag = 1; disable_all(); }
        static int in_child() { return in_child_flag; }
#endif
};

// clumsy name, but we will need to rename "class cleanup" to give
// this its name.
class ResourceCleanup
{
 public:
  explicit ResourceCleanup(std::function<void()> action)
    : doit_(action) {}

  static inline void noop() { }

  void disarm()
  {
    doit_ = noop;
  }

  ~ResourceCleanup()
    {
      doit_();
    }

 private:
  std::function<void()> doit_;
};


#endif
