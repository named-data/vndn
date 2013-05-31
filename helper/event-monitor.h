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

#ifndef NDN_EVENT_MONITOR_H
#define NDN_EVENT_MONITOR_H

#include "corelib/ptr.h"
#include "monitorable.h"

#include <boost/unordered_map.hpp>

#include <event2/event.h>

namespace vndn
{

typedef struct event_state {
    EventMonitor *em;
    Ptr<Monitorable> pMon;
    struct event *ev;
} event_state;

typedef boost::unordered_map<int, struct event_state *> fd_event_map;

class EventMonitor
{
public:
    EventMonitor();

    void addTimer(event_callback_fn callback, void *args, const struct timeval *timeout); // timer events
    void add(Ptr<Monitorable> pMonitorable); // monitor file descriptor objects
    void erase(Ptr<Monitorable> &pMon);
    void monitor();

private:
    static void do_read(evutil_socket_t fd, short events, void *arg);

    fd_event_map m_fd_state_map;
    struct event_base *m_base;
};

}

#endif
