/*
 * Copyright (c) 2012-2013 Giulio Grassi <giulio.grassi86@gmail.com>
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
 */

#ifndef NDN_HUB_OVER_IP_DEVICE_FACE_H
#define NDN_HUB_OVER_IP_DEVICE_FACE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include "ndn-net-device-face.h"
#include "ndn-l3-protocol.h"
#include "helper/event-monitor.h"
#include "network/packet.h"
#include "network/mac/ll-metadata.h"
#include "network/mac/ll-metadata-over-ip.h"
#include "network/request-source-ip-info.h"


namespace vndn
{

class EventMonitor;


/**
 * \ingroup ndn-face
 * \brief Implementation of layer-2 (Ethernet) NDN face
 *
 * This class defines basic functionality of NDN face. Face is core
 * component responsible for actual delivery of data packet to and
 * from NDN stack
 *
 * NDNNetDevice face is permanently associated with one NetDevice
 * object and this object cannot be changed for the lifetime of the
 * face
 *
 * \see NDNLocalFace, NDNNetDeviceFace, NDNIpv4Face, NDNUdpFace
 */
class NDNHubOverIPDeviceFace : public NDNFace
{
public:
    /**
     * \brief Constructor
     *
     * \param netDevice a smart pointer to NetDevice object to which
     * this face will be associate
     */
    NDNHubOverIPDeviceFace (std::string localIP);

    virtual std::ostream &Print(std::ostream &os) const;

    virtual void readHandler(EventMonitor &daemon);
    virtual bool Send(const Ptr<const Packet> &p);

    /**
    * \brief Send packet on a Face, plus some metadata
    *
    * \param p smart pointer to a packet to send
    * \param metadata metadata for the face. It contains the list of destination IP address
    *
    * @return false if either limit is reached
    * */
    virtual bool Send(const Ptr<const Packet> &p, RequestSourceInfo *metadata);

private:
    NDNHubOverIPDeviceFace (const NDNHubOverIPDeviceFace &); ///< \brief Disabled copy constructor
    NDNHubOverIPDeviceFace &operator= (const NDNHubOverIPDeviceFace &); ///< \brief Disabled copy operator

    struct sockaddr_in m_si;
    struct sockaddr_in m_si_other;
};

} // namespace vndn

#endif // NDN_HUB_OVER_IP_DEVICE_FACE_H
