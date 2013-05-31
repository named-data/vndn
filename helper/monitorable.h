/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Author: Lucas Wang <lucas@cs.ucla.edu>
 */

#ifndef MONITORABLE_H
#define MONITORABLE_H

#include "corelib/simple-ref-count.h"

namespace vndn
{

class EventMonitor;

/** \ingroup ndn
 * \brief the Monitorable interface defines objects that
 * can be fed into EventMonitor to wait for events and these objects
 * are able to process events when they happen.
 *
 * The Monitorable interface includes (i) the callback functions to
 * process a normal event: readHandler; (ii) the callback functions to
 * process an exception event: exceptHandler; and also (iii) the
 * getMonitorFD function to get the file descriptor that will be used
 * inside EventMonitor.
 */
class Monitorable : public SimpleRefCount<Monitorable>
{
public:
    virtual void readHandler(EventMonitor &daemon) = 0;
    virtual void exceptHandler() {}
    virtual int getMonitorFd() const = 0;
    virtual ~Monitorable() {}
};

}

#endif
