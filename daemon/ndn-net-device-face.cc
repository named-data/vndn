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

#include "ndn-net-device-face.h"
#include "ndn-l3-protocol.h"
#include "helper/event-monitor.h"
#include "corelib/log.h"
#include "network/packet.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

NS_LOG_COMPONENT_DEFINE ("NDNNetDeviceFace");

namespace vndn
{

NDNNetDeviceFace::NDNNetDeviceFace (std::string localIP, std::string hubIP)
    : NDNFace()
{
    /* Create the IPv4 UDP socket */
    if ((m_app_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        NS_LOG_ERROR("Failed to create UDP socket.");
        throw "failed to create UDP socket";
    }
    NS_LOG_DEBUG("UDP socket created on fd " << m_app_fd);

    /* Set to non-blocking */
    if (fcntl(m_app_fd, F_SETFL, O_NONBLOCK) < 0) {
        NS_LOG_ERROR("Could not set non-blocking flag.");
        throw "could not set non-blocking flag";
    }
    NS_LOG_DEBUG("Socket on fd " << m_app_fd << " set to non-blocking.");

    memset((char *) &m_si, 0, sizeof(m_si));
    m_si.sin_family = AF_INET;
    m_si.sin_port = htons(NDN_UDP_PORT);
    m_si.sin_addr.s_addr = inet_addr(localIP.c_str()); //htonl(INADDR_ANY);

    memset((char *) &m_si_other, 0, sizeof(m_si_other));
    m_si_other.sin_family = AF_INET;
    m_si_other.sin_port = htons(NDN_UDP_PORT);
    m_si_other.sin_addr.s_addr = inet_addr(hubIP.c_str());

    if (bind(m_app_fd, (sockaddr *)&m_si, sizeof(m_si)) == -1) {
        NS_LOG_ERROR("Failed to bind socket.");
        throw "bind error";
    }
}

bool NDNNetDeviceFace::Send(const Ptr<const Packet> &p)
{
    int sent;
    if ((sent = sendto(m_app_fd, p->GetRawBuffer(), p->GetSize(), 0, (const sockaddr *)&m_si_other, sizeof(m_si_other))) < 0) {
        NS_LOG_ERROR("sendto() failed.");
        return false;
    }

    std::string addr = std::string(inet_ntoa(m_si_other.sin_addr));
    NS_LOG_INFO("Sent a packet over IP, length = " << sent << ", dest = " << addr);
    return true;
}

void NDNNetDeviceFace::readHandler(EventMonitor &)
{
    NS_LOG_FUNCTION_NOARGS();

    // read message from app_fd and send to NDNL3Protocol
    try {
        const Ptr<Packet> newPacket = Packet::InitFromFD(m_app_fd);
        NS_LOG_INFO("Received a packet from the hub.");
        Receive(newPacket);
    } catch (PacketException) {
        NS_LOG_WARN("Packet::InitFromFD() failed.");
        //Ptr<Monitorable> pThis(this);
        //daemon.erase(pThis);
        //exit(0)
        //TODO manage error
    }
}

NDNNetDeviceFace &NDNNetDeviceFace::operator= (const NDNNetDeviceFace &)
{
    return *this;
}

std::ostream &NDNNetDeviceFace::Print(std::ostream &os) const
{
    os << "dev=net(" << getMonitorFd() << ")";
    return os;
}

} // namespace vndn
