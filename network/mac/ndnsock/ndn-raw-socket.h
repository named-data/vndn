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

#ifndef NDNRAWSOCKET_H_
#define NDNRAWSOCKET_H_

#include "ndn-socket.h"

#include <string>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if_arp.h>
#include <unistd.h>


namespace vndn
{

/**
 * \brief Supports broadcast communication (MAC layer)
 *
 * Implements the API for the broadcast communication at MAC layer with no use of IP protocol.
 * It uses raw socket to provide broadcast communication
 * It can be used also to unicast communication with no IP (unicast at MAC layer)
 * It is supported only by Unix-like system
 * */

class NdnRawSocket : public NdnSocket
{

public:

    /**
     * @brief Default constructor. It prepares the socket, without binding it to any device. Before you can use the socket, you need to call the setDevice to bind the socket to one (or all) device
     */
    NdnRawSocket();

    /**
     * @brief Constructor. It creates the socket and bind it to the specified device. The socket will be ready to use.
     * @param deviceName Name of the device name on which the socket has to be binded. Use the string "ALLDEVICE" to bind the socket to all the available network interfaces. Be careful, multiple sockets cannot be binded to the same device
     */
    NdnRawSocket(std::string deviceName);

    /**
     * \brief sends a packet using the raw socket
     *
     * It encapsulates the data into a MAC header and sends the packet out. The destination has to be previously set by setDestination
     * \param data address of the data that has to be sent
     * \param len size of the data
     * \return the size of the data sent out (link layer header is not considered). -1 if there was an error
     * */
    int send (void *data, int len);

    /**
     * \brief sends a packet using the raw socket
     *
     * It encapsulates the data into a MAC header and sends the packet out.
     * \param data address of the data that has to be sent
     * \param len size of the data
     * \param destMacAddress mac address used as destination
     * \return the size of the data sent out (link layer header is not considered). -1 if there was an error
     * */
    int sendTo (void *data, int len, const unsigned char destMacAddress[ETH_ALEN]);

    /**
     * \brief Read from the raw socket
     *
     * Read data from the socket
     * \param buffer address of the location where the data will be stored
     * \param maxSize max size of the data
     * \param flag if 1, ndnRawSocket has to store ndnSocketMetaData in the buffer (in the head of the data)
     * \return size of the data read from the socket (without MAC header, but with ndnSocketMetaData if flag=1)
     * */
    int read (void *buffer, int maxSize, int flag);

    /**
     * \brief Read from the raw socket
     *
     * Read data from the socket. It's like call read(buffer, maxSize, flag=0)
     * \param buffer address of the location where the data will be stored
     * \param maxSize max size of the data
     * \return size of the data read from the socket (without MAC header)
     * */
    int read (void *buffer, int maxSize);


    /**
     * \brief Read n bytes from the raw socket
     *
     * NOT IMPLEMENTED YET. maybe it will never be, there is no need of a function like that
     * */
    int readn(void *buffer, int dataSize);

    /**
     * \brief get the interface MAC address on which the raw socket is binded
     *
     * \return address of an unsigned char [6 (ETH_ALEN)] where the interface MAC address is stored
     * */
    unsigned char *getDeviceMacAddress();


    /**
     * \brief Set the destination MAC address
     *
     * Set the destination MAC address, so after that it's possible to send data out without specify again the destination address
     * \param destMacAddress destination MAC address
     * */
    void setDestination(const unsigned char destMacAddress[ETH_ALEN]);

    /**
     * \brief Delete the previously destination address
     * */
    void unsetDestination();

    /**
     * \brief Set the interface that has to be used to send/receive raw data
     *
     * Set the device in which the raw socket woulb be binded
     * \param deviceName Name of the network interface used to send/receive packets
     * */
    void setDevice (std::string deviceName);

    /**
     * \brief Get the interface name used to sent/receive packet
     * \return The name of the network interface used by raw socket to send/receive packets
     * */
    std::string getDevice ();

    /**
     * \brief Delete the previously interface information
     *
     * After this, the raw socket is no more binded to any interface. You need to call setDevice to use again the socket
     * Be aware that the ID of the socket could change, so call again getSocket
     * */
    void unsetDevice();


    /**
     * \brief Get the raw socket id used to send/receive packet
     *
     * Get the id of the socket that ndnRawSocket uses to send/receive packet. It's possible to use select with this id and then call the ndnRawSocket read/send functions to read/send the data
     * \return The id of the raw socket used by ndnRawSocket
     * */
    int getSocket();

    /**
     * \brief Close the raw socket used by the class
     *
     * Close the raw socket, delete the previously destination and interface information
     * */
    void closeSocket();

    ~NdnRawSocket();


protected:
    /**
     * \brief Set properly the sockaddr_ll sockAddr
     *
     * Set all the sockAddr that can be properly set with no knowledge about destination and network interface used
     * */
    int setSockAddr();

    /**
     * \brief Add to sockaddr_ll sockAddr the information about the destination
     *
     * \param destMacAddress destination MAC address
     * */
    void setDestSockAddr(const unsigned char destMacAddress[ETH_ALEN]);

    /**
     * \brief Add to the source MAC header the information about the destination
     *
     * \param destMacAddress destination MAC address
     * */
    void setDestEthHeader(const unsigned char destMacAddress[ETH_ALEN]);

    /**
     * \brief Set the source MAC address
     *
     * Get the MAC address of the network interface used by ndnRawSocket and add this information to the source MAC header and to sockAddr
     * \param Network interface name
     * \return 1 if everything is ok, -1 if there was an error
     * */
    int setSourceMacAddress (std::string deviceName);

    /**
     * \brief Extract source and destination information from MAC header
     *
     * \param dataWithHeader pointer to the MAC header
     * \param buffer buffer where the payload of the packet will be stored
     * \param dataLen size of the packet
     * \param metaDataSize size of metadata (ndnSocketMetaData). 0 if flag is not set
     * \param flag has to be 1 if the ndnSocketMetaData has to be stored
     * \return size of the data stored in buffer (payload + ndnSocketMetaData if required)
     * */
    int extractDataFromPacket( char *dataWithHeader, uint8_t *buffer, int dataLen, int metaDataSize, int flag);


    /**
     * \brief Raw socket file descriptor
     * */
    int socketId;

    /**
     * \brief Mac header injected in the outgoing packet
     * */
    struct ethhdr sourceMacHeader;

    /**
     * \brief Name of the device used to send/receive the packet
     * */
    std::string deviceName; //set to NULLDEVICE if there is no device name or source address

    /**
     * \brief sockaddr_ll used to specify all the physical layer options
     * */
    struct sockaddr_ll sockAddr;

    /**
     * \brief It tells if someone has already specified the destination address
     * */
    bool destinationIsSet;

    /**
     * \brief offset to store the llcOffset
     *
     * Based on the value of MACPROTO used to send packet, llcOffset could be necessary or not.
     * Using 0x88b5 as MACPROTO, llcOffset can be 0
     * */
    const static int llcOffset = 0;


    /**
     * \brief variable used to temporary store error messages
     * */
    std::string error;

};

}

#endif /* NDNRAWSOCKET_H_ */
