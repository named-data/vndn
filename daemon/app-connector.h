/*
 * Copyright (c) 2012-2013 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Lucas Wang <lucas@cs.ucla.edu>
 *         Giulio Grassi <giulio.grassi86@gmail.com>
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#ifndef APP_CONNECTOR_H
#define APP_CONNECTOR_H

#include <sys/select.h>
#include "helper/monitorable.h"

namespace vndn
{

class EventMonitor;

/**
 * \brief Listen for connections from local applications and creates NDNLocalFaces
 */
class AppConnector : public Monitorable
{
public:
    AppConnector();
    int getNewFd();

    ////////////////////////////////////////////////
    /// overrided functions defined in base class Monitorable
    /**
     * \brief Create a new NDNLocalFace, add it to EventMonitor and
     * the NDNL3Protocol, also configure the face with name prefix "/"
     * in fib so that it will be forwarded with any interest
     */
    void readHandler(EventMonitor &em);

    /**
     * \brief Print error message and exit the daemon
     */
    void exceptHandler();
    /**
     * \brief see Monitorable::getMonitorFD
     */
    int getMonitorFd() const;

private:
    int listen_sockfd; ///< \brief the socket used to listen for new connections from local apps
    int new_fd; ///< \brief its value gets set to the new connection fd after call to read_handler
};

}

#endif
