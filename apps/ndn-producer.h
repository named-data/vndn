/*
 * Copyright (c) 2011-2013 University of California, Los Angeles
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
 *         Giulio Grassi <giulio.grassi86@gmail.com>
 *         Davide Pesavento <davidepesa@gmail.com>
 *         Francesco Berni <kurojishi@gmail.com>
 */

#ifndef NDN_PRODUCER_H
#define NDN_PRODUCER_H

#include <list>

#include "helper/monitorable.h"
#include "helper/event-monitor.h"
#include "network/ndn-name-components.h"

namespace vndn
{

/**
 * @ingroup ndn
 * \brief NDN application for replying with content
 */
class NDNProducer : public Monitorable
{
public:
    /**
     * \brief Set up the communication channel between this NDNProducer and NDN daemon
     * \param sockfd   the file descriptor used to exchange message with the NDN daemon
     */
    NDNProducer(int sockfd);

    /// functions inherited from Monitorable
    virtual void readHandler(EventMonitor &);
    virtual int getMonitorFd() const;

    int read(NameComponents & nameComponent);

    int sendContent(NameComponents& name, std::string content);
    int sendContent(NameComponents & name, const uint8_t * content, int size);

    /**
     * \brief Application tells ndnd that it can deal with this name
     *
     * \param name nameComponent that producer wants to register to ndnd and file descriptor of the local face
     */
    void registerName(NameComponents &name, uint32_t face_fd);

    /**
     * \brief Application tells ndnd that it doesn't deal with this name anymore
     *
     * \param name nameComponent that producer wants to de-register to ndnd and file descriptor of the local face
     */
    void deregisterName(NameComponents &name, uint32_t face_fd);

protected:
    int m_sock_fd;
};

} // namespace vndn

#endif
