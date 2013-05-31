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

#include "ndn-raw-socket.h"
#include "corelib/assert.h"
#include "corelib/log.h"

NS_LOG_COMPONENT_DEFINE ("NDNRawSocket");

namespace vndn
{

NdnRawSocket::NdnRawSocket()
{
}

NdnRawSocket::NdnRawSocket(std::string deviceNameP)
{
    socketId = ::socket(AF_PACKET, SOCK_RAW, htons(MACPROTO)); //ETH_P_ALL instead of MACPROTO to get all the packets
    if (socketId == -1) {
        std::string err = "Creation of raw socket failed: ";
        err += strerror(errno);
        throw NdnSocketException(err);
    }

    NS_LOG_DEBUG("Raw socket created, fd=" << socketId);

    memset(&sockAddr, 0, sizeof(struct sockaddr_ll));
    memset(&sourceMacHeader, 0, sizeof(sourceMacHeader));
    if (setSockAddr() == -1) {
        std::string err = "setSockAddr() failed: ";
        err += error;
        throw NdnSocketException(err);
    }
    sourceMacHeader.h_proto = htons(MACPROTO);
    deviceName = NULLDEVICE;
    if (setSourceMacAddress(deviceNameP) == -1) {
        std::string err = "setSourceMacAddress() failed: ";
        err += error;
        throw NdnSocketException(err);
    }
    destinationIsSet = false;
}

NdnRawSocket::~NdnRawSocket()
{
    ::close(socketId);
}

int NdnRawSocket::send(void *data, int len)
{
    if (socketId == -1) {
        NS_LOG_ERROR("Invalid socket");
        throw NdnSocketException("send failed: invalid socket");
    }
    if (deviceName.compare(NULLDEVICE) == 0) {
        NS_LOG_ERROR("Invalid device");
        throw NdnSocketException("send failed: invalid device");
    }
    if (!destinationIsSet) {
        NS_LOG_ERROR("Destination is not set");
        throw NdnSocketException("send failed: no destination address is set");
    }

    char *dataWithHdr = new char[len + sizeof(struct ethhdr) + llcOffset];
    std::memcpy(dataWithHdr, &sourceMacHeader, sizeof(struct ethhdr));
    std::memcpy(dataWithHdr + sizeof(struct ethhdr) + llcOffset, data, len);

    int dataSent = 0;
    NS_LOG_DEBUG("sockaddr details: " << PRINTABLE_MAC_ADDRESS(sockAddr.sll_addr) << " "
                 << sockAddr.sll_family << " " << (int) sockAddr.sll_halen << " "
                 << sockAddr.sll_hatype << " " << (int) sockAddr.sll_ifindex);
    dataSent = sendto(socketId, dataWithHdr,
                      len + sizeof(struct ethhdr) + llcOffset, 0,
                      (struct sockaddr *) &sockAddr, sizeof(sockAddr));
    if (dataSent == -1) {
        NS_LOG_ERROR("sendto() failed: " << strerror(errno));
        delete[] dataWithHdr;
        std::string err = "send failed: ";
        err += strerror(errno);
        throw NdnSocketException(err);
    }
    delete[] dataWithHdr;
    return dataSent - sizeof(struct ethhdr) - llcOffset;
}

int NdnRawSocket::sendTo(void *data, int len, const unsigned char destMacAddress[ETH_ALEN])
{
    if (socketId == -1) {
        NS_LOG_ERROR("Invalid socket");
        throw NdnSocketException("send failed: invalid socket");
    }
    if (deviceName.compare(NULLDEVICE) == 0) {
        NS_LOG_ERROR("Invalid device");
        throw NdnSocketException("send failed: invalid device");
    }

    NS_LOG_DEBUG("destination mac address: " << PRINTABLE_MAC_ADDRESS(destMacAddress));
    struct sockaddr_ll tmpSockAddr;
    tmpSockAddr.sll_family = PF_PACKET;
    tmpSockAddr.sll_protocol = htons(MACPROTO);
    tmpSockAddr.sll_ifindex = sockAddr.sll_ifindex;
    tmpSockAddr.sll_hatype = ARPHRD_ETHER;
    tmpSockAddr.sll_halen = ETH_ALEN;
    tmpSockAddr.sll_addr[6] = 0x00;
    tmpSockAddr.sll_addr[7] = 0x00;
    for (int i = 0; i < ETH_ALEN; i++) {
        tmpSockAddr.sll_addr[i] = destMacAddress[i];

    }
    struct ethhdr tmpEthHeader;
    std::memcpy(&tmpEthHeader, &sourceMacHeader, sizeof(struct ethhdr));

    std::memcpy(&tmpEthHeader.h_dest, destMacAddress, ETH_ALEN);

    char *dataWithHdr = new char[len + sizeof(struct ethhdr) + llcOffset];
    std::memcpy(dataWithHdr, &tmpEthHeader, sizeof(struct ethhdr));
    std::memcpy(dataWithHdr + sizeof(struct ethhdr) + llcOffset, data, len);

    int dataSent = 0;
    NS_LOG_DEBUG("sockaddr details: "
                 << PRINTABLE_MAC_ADDRESS(tmpSockAddr.sll_addr) << " "
                 << tmpSockAddr.sll_family << " " << (int) tmpSockAddr.sll_halen
                 << " " << tmpSockAddr.sll_hatype << " "
                 << (int) tmpSockAddr.sll_ifindex);

    dataSent = sendto(socketId, dataWithHdr,
                      len + sizeof(struct ethhdr) + llcOffset, 0,
                      (struct sockaddr *) &tmpSockAddr, sizeof(sockAddr));
    if (dataSent == -1) {
        NS_LOG_ERROR("sendto() failed: " << strerror(errno));
        delete[] dataWithHdr;
        std::string err = "send failed: ";
        err += strerror(errno);
        throw NdnSocketException(err);
    }
    delete[] dataWithHdr;
    return dataSent - sizeof(struct ethhdr) - llcOffset;
}

int NdnRawSocket::read(void *buffer, int maxSize, int flag)
{
    int bufferSize = maxSize + sizeof(struct ethhdr) + llcOffset;
    int metaDataOffset = 0;
    if (flag == 1) { //ndnRawSocket has to store ndnSocketMetaData in the buffer (in the head of the data)
        metaDataOffset = sizeof(ndnSocketMetaData);
        bufferSize += metaDataOffset;
    }
    char *dataFromNetwork = new char[bufferSize];
    int dataLen = 0;
    if (deviceName.compare(NULLDEVICE) == 0) {
        delete[] (char *) dataFromNetwork;
        throw NdnSocketException("read error: no valid device name");
    }
    if (socketId < 0) {
        delete[] (char *) dataFromNetwork;
        throw NdnSocketException("read error: no valid socket");
    }
    dataLen = recvfrom(socketId, dataFromNetwork,
                       maxSize + sizeof(struct ethhdr) + llcOffset, 0, NULL, NULL); //if necessary, we can put the sockaddr struct to retrieve some information about the sender
    if (dataLen < 0) {
        delete[] (char *) dataFromNetwork;
        std::string err = "read failed: ";
        err.append(strerror(errno));
        throw NdnSocketException(err);
    }
    //std::cout << "ndn read: data received: " << dataLen << std::endl;
    uint8_t *buf = (uint8_t *)buffer;
    dataLen = extractDataFromPacket(dataFromNetwork, buf, dataLen, metaDataOffset, flag);
    delete[] (char *) dataFromNetwork;
    return dataLen;
}

int NdnRawSocket::read(void *buffer, int maxSize)
{
    return read(buffer, maxSize, 0);
}

int NdnRawSocket::readn(void *buffer, int maxSize)
{
    NS_ASSERT_MSG(false, "readn() not implemented");
    return -1;
}

int NdnRawSocket::getSocket()
{
    return socketId;
}

void NdnRawSocket::closeSocket()
{
    ::close(socketId);
    socketId = -1;
    destinationIsSet = false;
    deviceName = NULLDEVICE;
}

void NdnRawSocket::setDevice(std::string newDeviceName)
{
    unsetDevice();

    if (setSourceMacAddress(newDeviceName) == -1) {
        std::string err = "set new device failed: ";
        err += error;
        throw NdnSocketException(err);
    }
}

void NdnRawSocket::unsetDevice()
{
    if (deviceName.compare(NULLDEVICE) == 0) {
        //nothing to do, the socket is not bound
        return;
    }

    deviceName = NULLDEVICE;
    ::close(socketId);

    socketId = ::socket(AF_PACKET, SOCK_RAW, htons(MACPROTO));
    if (socketId == -1) {
        std::string err = "Creation of raw socket failed: ";
        err += strerror(errno);
        throw NdnSocketException(err);
    }
}

unsigned char *NdnRawSocket::getDeviceMacAddress()
{
    return sourceMacHeader.h_source;
}

std::string NdnRawSocket::getDevice()
{
    return deviceName;
}

void NdnRawSocket::setDestination(const unsigned char destMacAddress[])
{
    setDestSockAddr(destMacAddress);
    setDestEthHeader(destMacAddress);
    destinationIsSet = true;
}

void NdnRawSocket::unsetDestination()
{
    destinationIsSet = false;
}

int NdnRawSocket::setSockAddr()
{
    if (socketId <= 0) {
        error = "invalid socket ";
        error += socketId;
        return -1;
    }

    sockAddr.sll_family = PF_PACKET;
    //sockAddr.sll_protocol = htons ( ETH_P_IP ) ;
    sockAddr.sll_protocol = htons(MACPROTO);
    //sockAddr.sll_ifindex  = ifr.ifr_ifindex;
    sockAddr.sll_hatype = ARPHRD_ETHER;
    sockAddr.sll_halen = ETH_ALEN;
    //setDestSockAddr(dest_mac_adrr)
    sockAddr.sll_addr[6] = 0x00;
    sockAddr.sll_addr[7] = 0x00;

    return 1;
}

void NdnRawSocket::setDestSockAddr(const unsigned char destMacAddress[])
{
    for (int i = 0; i < ETH_ALEN; i++) {
        sockAddr.sll_addr[i] = destMacAddress[i];
    }
    NS_LOG_DEBUG("sockaddr dest " << PRINTABLE_MAC_ADDRESS(sockAddr.sll_addr));
}

void NdnRawSocket::setDestEthHeader(const unsigned char destMacAddress[])
{
    std::memcpy(sourceMacHeader.h_dest, destMacAddress, ETH_ALEN);
    //std::memcpy(sourceMacHeader.h_source,destMacAddress,ETH_ALEN);
}

int NdnRawSocket::setSourceMacAddress(std::string newDeviceName)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    if (newDeviceName.compare(NULLDEVICE) == 0) {
        error = "invalid device " + newDeviceName;
        return -1;
    }
    if (socketId <= 0) {
        error = "invalid socket ";
        error += socketId;
        return -1;
    }
    newDeviceName.copy(ifr.ifr_name, IFNAMSIZ, 0);
    if (deviceName.compare(NULLDEVICE) != 0) {
        NS_LOG_WARN("Something is wrong. This shouldn't have happened, socket already bound");
    }

    deviceName = newDeviceName;
    NS_LOG_DEBUG("device name: " << ifr.ifr_name);
    if (ioctl(socketId, SIOCGIFINDEX, &ifr) == -1) {
        error = "ioctl SIOCGIFINDEX failed: ";
        error += strerror(errno);
        return -1;
    }
    sockAddr.sll_ifindex = ifr.ifr_ifindex;
    NS_LOG_DEBUG("device index: " << sockAddr.sll_ifindex);

    if (ioctl(socketId, SIOCGIFHWADDR, &ifr) == -1) {
        error = "ioctl SIOCGIFHWADDR failed: ";
        error += strerror(errno);
        return -1;
    }
    for (int i = 0; i < ETH_ALEN; i++) {
        sourceMacHeader.h_source[i] = ifr.ifr_hwaddr.sa_data[i];
    }
    NS_LOG_DEBUG("Source mac address: " << PRINTABLE_MAC_ADDRESS(sourceMacHeader.h_source));

    if ((bind(socketId, (struct sockaddr *) &sockAddr, sizeof(sockAddr))) == -1) {
        error = "bind failed: ";
        error += strerror(errno);
        return -1;
    }

    return 1;
}

int NdnRawSocket::extractDataFromPacket(char *dataWithHeader, uint8_t *buffer,
                                        int pktLen, int metaDataSize, int flag)
{
    struct ethhdr *header = (struct ethhdr *) dataWithHeader;
    if (ntohs(header->h_proto) == MACPROTO) {
        NS_LOG_DEBUG("source address: " << PRINTABLE_MAC_ADDRESS(header->h_source));
        NS_LOG_DEBUG("dest address: " << PRINTABLE_MAC_ADDRESS(header->h_dest));
        NS_LOG_DEBUG("protocol: " << std::hex << ntohs(header->h_proto) << std::dec);

        if (flag == 1) {
            memcpy(buffer, header->h_source, metaDataSize);
        }
        memcpy(&(buffer[metaDataSize]), dataWithHeader + sizeof(struct ethhdr) + llcOffset,
               pktLen - sizeof(struct ethhdr) - llcOffset);
        return pktLen - sizeof(struct ethhdr) - llcOffset + metaDataSize;
    }

    return 0;
}

}
