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

#include "event-monitor.h"
#include "corelib/log.h"

#include <fcntl.h>

NS_LOG_COMPONENT_DEFINE ("EventMonitor");

namespace vndn
{

void EventMonitor::do_read(evutil_socket_t fd, short events, void *arg)
{
    event_state *e_arg = (event_state *) arg;
    assert(e_arg->em != NULL);
    e_arg->pMon->readHandler(*(e_arg->em));
}

EventMonitor::EventMonitor()
{
    m_base = event_base_new();
}

void EventMonitor::addTimer(event_callback_fn callback, void *args, const struct timeval *timeout)
{
    struct event *timerEvent = evtimer_new(m_base, callback, args);
    evtimer_add(timerEvent, timeout);
}

void EventMonitor::add(Ptr<Monitorable> pMonitorable)
{
    int fd = pMonitorable->getMonitorFd();
    event_state *state = (event_state *)calloc(1, sizeof(event_state)); // 1 memory alloc
    state->em = this;
    state->pMon = pMonitorable;

    state->ev = event_new(m_base, fd, EV_READ | EV_PERSIST, &EventMonitor::do_read, state); // 2 memory alloc

    if (state->ev != NULL) {
        m_fd_state_map[fd] = state; // 3 memory alloc
        event_add(state->ev, NULL); // 4 schedule event
    } else {
        free(state);
        NS_LOG_WARN("Cannot monitor fd(" << fd << ")");
    }
}

void EventMonitor::erase(Ptr<Monitorable> &pMon)
{
    int fd = pMon->getMonitorFd();
    NS_LOG_INFO("Erasing fd(" << fd << ")");
    fd_event_map::iterator it = m_fd_state_map.find(fd);
    assert(it != m_fd_state_map.end());
    event_state *state = it->second;
    event_del(state->ev); // 4 cancel event
    m_fd_state_map.erase(it); // 3 memory alloc
    event_free(state->ev); // 2 memory alloc
    free(state); // 1 memory alloc
}

void EventMonitor::monitor()
{
    event_base_dispatch(m_base);
    // fd_set read_fds, write_fds, except_fds;

    // while(true){
    //     clear_fds(&read_fds, &write_fds, &except_fds);

    //     int max_fd = -1;
    //     boost::unordered_set<Ptr<Monitorable> >::iterator it = monitor_set.begin();
    //     for(; it != monitor_set.end(); it++){
    //         int monitor_fd = (*it)->getMonitorFd();

    //         NS_LOG_INFO("Monitoring " << monitor_fd);

    //         FD_SET(monitor_fd, &read_fds);
    //         FD_SET(monitor_fd, &except_fds);

    //         if(monitor_fd > max_fd){
    //             max_fd = monitor_fd;
    //         }
    //     }

    //     select(max_fd + 1, &read_fds, &write_fds, &except_fds, NULL);

    //     // check which fd is ready
    //     it = monitor_set.begin();
    //     for(; it != monitor_set.end(); it++){
    //         Ptr<Monitorable> ptr = *it;
    //         int monitor_fd = ptr->getMonitorFd();


    //         if(FD_ISSET(monitor_fd, &except_fds)){
    //             ptr->exceptHandler();
    //             exit(1);
    //         }

    //         if(FD_ISSET(monitor_fd, &read_fds)){
    //             ptr->readHandler(*this);
    //             // runnig readHandler may invalidate the iterator,
    //             // so we must end the for loop at this moment, and start a
    //             // monitoring on all elements from scratch
    //             break;
    //         }
    //     }
    // }

}

}
