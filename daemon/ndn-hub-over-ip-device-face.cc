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

#include "ndn-hub-over-ip-device-face.h"
#include "corelib/log.h"

#include <cstring>
#include <errno.h>

#define DEFAULT_CLIENT_IP "192.168.0.8"

NS_LOG_COMPONENT_DEFINE("NDNHubOverIPDeviceFace");

namespace vndn
{

NDNHubOverIPDeviceFace::NDNHubOverIPDeviceFace(std::string localIP)
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
    m_si.sin_addr.s_addr = inet_addr(localIP.c_str());

    if (bind(m_app_fd, (sockaddr *)&m_si, sizeof(m_si)) == -1) {
        NS_LOG_ERROR("Failed to bind socket.");
        throw "bind error";
    }
}

bool NDNHubOverIPDeviceFace::Send(const Ptr<const Packet> &p)
{
    return Send(p, NULL);
}

bool NDNHubOverIPDeviceFace::Send(const Ptr<const Packet> &p, RequestSourceInfo *metadata)
{
    int sent;
    struct sockaddr_in source;
    RequestSourceIPInfo *info = (RequestSourceIPInfo *)metadata;
    if (metadata != NULL) {
        std::list<std::pair<in_addr_t, unsigned short> >::iterator it;
        for (it = info->getAllSource().begin(); it != info->getAllSource().end(); it++) {
            memset((char *) &source, 0, sizeof(source));
            source.sin_family = AF_INET;
            memcpy(&(source.sin_port), &(it->second), sizeof(unsigned short));
            memcpy(&(source.sin_addr.s_addr), &(it->first), sizeof(in_addr_t));
            if ((sent = sendto(m_app_fd, p->GetRawBuffer(), p->GetSize(), 0, (const sockaddr *)&source, sizeof(source))) < 0) {
                NS_LOG_ERROR("sendto() failed.");
                return false;
            }
            std::string addr = std::string(inet_ntoa(source.sin_addr));
            NS_LOG_INFO("Sent a packet over IP, length = " << sent << ", dest = " << addr<<":"<<ntohs(source.sin_port));
        }
        //free(metadata);
        //metadata=NULL;
    } else {
        /**
         * Send it to default ip.
         * This could happens when we are sending a packet that has not been requested from this face,
         * for example we are forwarding an interest.
         * */
        memset((char *) &source, 0, sizeof(source));
        source.sin_family = AF_INET;
        source.sin_port = htons(NDN_UDP_PORT);
        source.sin_addr.s_addr = inet_addr(DEFAULT_CLIENT_IP);
        if ((sent = sendto(m_app_fd, p->GetRawBuffer(), p->GetSize(), 0, (const sockaddr *)&source, sizeof(source))) < 0) {
            NS_LOG_ERROR("sendto() failed: " << strerror(errno));
            return false;
        }
        std::string addr = std::string(inet_ntoa(source.sin_addr));
        NS_LOG_INFO("Sent a packet over IP, length = " << sent << ", dest = " << addr);
    }

    return true;
}

void NDNHubOverIPDeviceFace::readHandler(EventMonitor &daemon)
{
    NS_LOG_FUNCTION_NOARGS();

    struct sockaddr_in source;
    //it will store the ip address of the node that sent the packet
    memset((char *) &source, 0, sizeof(source));
    uint8_t buffer[BUFLEN];
    socklen_t slen = sizeof(source);
    int bytes_read;
    if ((bytes_read = recvfrom(m_app_fd, buffer, BUFLEN, 0, (sockaddr *)&source, &slen)) <= 0) {
        NS_LOG_ERROR("recvfrom() failed: " << strerror(errno));
        //throw PacketException();
        //TODO manage error
        return;
    }    
    std::string addr = std::string(inet_ntoa(source.sin_addr));
    unsigned short sinPort = source.sin_port;
    NS_LOG_INFO("Received a packet over IP from " << addr <<":"<<ntohs(sinPort)<< ", length = " << bytes_read);
    Ptr<Packet> newPacket = Packet::InitFromBuffer(buffer, bytes_read);

    LLMetadataOverIP *metadata = new LLMetadataOverIP(source.sin_addr.s_addr, source.sin_port);

    //newPacket->getBuffer().SetSize(bytes_read);
    newPacket->llmetadataptr = metadata;//(NDNIncomingPktSharedWithDeviceAdapter[incomingPktSharedMemoryIndex].metadata);
    newPacket->llmetadata = &(newPacket->llmetadataptr);

    Receive(newPacket);
}

NDNHubOverIPDeviceFace &NDNHubOverIPDeviceFace::operator=(const NDNHubOverIPDeviceFace &)
{
    return *this;
}

std::ostream &NDNHubOverIPDeviceFace::Print(std::ostream &os) const
{
    os << "dev=hub(" << getMonitorFd() << ")";
    return os;
}

} // namespace vndn
