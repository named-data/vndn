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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#ifndef NDN_NET_DEVICE_FACE_H
#define NDN_NET_DEVICE_FACE_H

#include "ndn-face.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define NDN_UDP_PORT  9695

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
class NDNNetDeviceFace : public NDNFace
{
public:
    /**
     * \brief Constructor
     *
     * \param netDevice a smart pointer to NetDevice object to which
     * this face will be associate
     */
    NDNNetDeviceFace(std::string localIP, std::string hubIP);

    virtual std::ostream &Print(std::ostream &os) const;

    virtual void readHandler(EventMonitor &daemon);
    virtual bool Send(const Ptr<const Packet> &p);

private:
    NDNNetDeviceFace (const NDNNetDeviceFace &); ///< \brief Disabled copy constructor
    NDNNetDeviceFace &operator= (const NDNNetDeviceFace &); ///< \brief Disabled copy operator

    //Ptr<NetDevice> m_netDevice; ///< \brief Smart pointer to NetDevice
    struct sockaddr_in m_si;
    struct sockaddr_in m_si_other;
};

} // namespace vndn

#endif //NDN_NET_DEVICE_FACE_H
