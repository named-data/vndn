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

#include "app-connector.h"

#include "ndn-l3-protocol.h"
#include "ndn-local-face.h"
#include "corelib/log.h"
#include "corelib/singleton.h"
#include "helper/event-monitor.h"

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <boost/ref.hpp>

#define NDND_SOCKET_PATH "/var/run/ndnd.sock"

NS_LOG_COMPONENT_DEFINE("AppConnector");


namespace vndn
{

AppConnector::AppConnector()
{
    /* Create the local socket */
    if ((listen_sockfd = ::socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
        NS_LOG_ERROR("Failed to create socket for listening: " << strerror(errno));
        exit(1);
    }
    NS_LOG_DEBUG("Local socket created on fd " << listen_sockfd);

    /* Set to non-blocking */
    if (::fcntl(listen_sockfd, F_SETFL, O_NONBLOCK) < 0) {
        NS_LOG_ERROR("Could not set non-blocking flag: " << strerror(errno));
        exit(1);
    }
    NS_LOG_DEBUG("Socket on fd " << listen_sockfd << " set to non-blocking.");

    struct sockaddr_un dest_sun;
    memset(&dest_sun, 0, sizeof(dest_sun));
    dest_sun.sun_family = AF_UNIX;
    strcpy(dest_sun.sun_path, NDND_SOCKET_PATH);

    /* Cleanup previously used socket file */
    ::remove(NDND_SOCKET_PATH);

    /* Actually bind the socket */
    if (::bind(listen_sockfd, (const sockaddr *)&dest_sun, sizeof(dest_sun)) == -1) {
        NS_LOG_ERROR("Failed to bind local socket: " << strerror(errno));
        exit(1);
    }

    /* Tell the kernel to listen for new connections, queue up to 10 connections */
    ::listen(listen_sockfd, 10);

    /* initialize the new_fd */
    new_fd = -1;
}

void AppConnector::exceptHandler()
{
    NS_LOG_ERROR("Exception on local socket (fd=" << listen_sockfd << "), exiting.");
    exit(1);
}

void AppConnector::readHandler(EventMonitor &em)
{
    struct sockaddr_un dest_sun;
    socklen_t socklen = sizeof(dest_sun);

    new_fd = accept(listen_sockfd, (struct sockaddr *) &dest_sun, &socklen);
    NS_LOG_INFO("Got a new fd for local app: " << new_fd);

    Ptr<NDNLocalFace> newLocalFace = Create<NDNLocalFace>(new_fd);
    if (!!newLocalFace && send(new_fd,&new_fd,sizeof(new_fd),0) >= 0) {
        NDNL3Protocol *protocol = Singleton<NDNL3Protocol>::Get();
        protocol->AddFace(newLocalFace);
        em.add(newLocalFace);
    } else {
        NS_LOG_WARN("Failed to create NDNLocalFace for fd " << new_fd);
    }
}

int AppConnector::getMonitorFd() const
{
    return listen_sockfd;
}

int AppConnector::getNewFd()
{
    return new_fd;
}

} // end namespace vndn
